// file: source/opts.c

////////////////////////////////////////////////////////////////
//
// CLI Options
//
//

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

void opts_init( zpl_opts* opts, zpl_allocator a, char const * app )
{
	zpl_opts opts_  = { 0 };
	*opts           = opts_;
	opts->allocator = a;
	opts->appname   = app;

	array_init( opts->entries, a );
	array_init( opts->positioned, a );
	array_init( opts->errors, a );
}

void opts_free( zpl_opts* opts )
{
	for ( s32 i = 0; i < array_count( opts->entries ); ++i )
	{
		opts_entry* e = opts->entries + i;
		if ( e->type == ZPL_OPTS_STRING )
		{
			string_free( e->text );
		}
	}

	array_free( opts->entries );
	array_free( opts->positioned );
	array_free( opts->errors );
}

void opts_add( zpl_opts* opts, char const * name, char const * lname, const char* desc, u8 type )
{
	opts_entry e = { 0 };

	e.name  = name;
	e.lname = lname;
	e.desc  = desc;
	e.type  = type;
	e.met   = false;
	e.pos   = false;

	array_append( opts->entries, e );
}

opts_entry* zpl__opts_find( zpl_opts* opts, char const * name, uw len, b32 longname )
{
	opts_entry* e = 0;

	for ( int i = 0; i < array_count( opts->entries ); ++i )
	{
		e              = opts->entries + i;
		char const * n = ( longname ? e->lname : e->name );
		if ( ! n )
			continue;

		if ( zpl_strnlen( name, len ) == zpl_strlen( n ) && ! str_compare( n, name, len ) )
		{
			return e;
		}
	}

	return NULL;
}

void opts_positional_add( zpl_opts* opts, char const * name )
{
	opts_entry* e = zpl__opts_find( opts, name, zpl_strlen( name ), true );

	if ( e )
	{
		e->pos = true;
		array_append_at( opts->positioned, e, 0 );
	}
}

b32 opts_positionals_filled( zpl_opts* opts )
{
	return array_count( opts->positioned ) == 0;
}

string opts_string( zpl_opts* opts, char const * name, char const * fallback )
{
	opts_entry* e = zpl__opts_find( opts, name, zpl_strlen( name ), true );

	return (char*)( ( e && e->met ) ? e->text : fallback );
}

f64 opts_real( zpl_opts* opts, char const * name, f64 fallback )
{
	opts_entry* e = zpl__opts_find( opts, name, zpl_strlen( name ), true );

	return ( e && e->met ) ? e->real : fallback;
}

s64 opts_integer( zpl_opts* opts, char const * name, s64 fallback )
{
	opts_entry* e = zpl__opts_find( opts, name, zpl_strlen( name ), true );

	return ( e && e->met ) ? e->integer : fallback;
}

void zpl__opts_set_value( zpl_opts* opts, opts_entry* t, char* b )
{
	t->met = true;

	switch ( t->type )
	{
		case ZPL_OPTS_STRING :
			{
				t->text = string_make( opts->allocator, b );
			}
			break;

		case ZPL_OPTS_FLOAT :
			{
				t->real = str_to_f64( b, NULL );
			}
			break;

		case ZPL_OPTS_INT :
			{
				t->integer = str_to_i64( b, NULL, 10 );
			}
			break;
	}

	for ( sw i = 0; i < array_count( opts->positioned ); i++ )
	{
		if ( ! str_compare( opts->positioned[ i ]->lname, t->lname ) )
		{
			array_remove_at( opts->positioned, i );
			break;
		}
	}
}

b32 opts_has_arg( zpl_opts* opts, char const * name )
{
	opts_entry* e = zpl__opts_find( opts, name, zpl_strlen( name ), true );

	if ( e )
	{
		return e->met;
	}

	return false;
}

void opts_print_help( zpl_opts* opts )
{
	zpl_printf( "USAGE: %s", opts->appname );

	for ( sw i = array_count( opts->entries ); i >= 0; --i )
	{
		opts_entry* e = opts->entries + i;

		if ( e->pos == ( b32 ) true )
		{
			zpl_printf( " [%s]", e->lname );
		}
	}

	zpl_printf( "\nOPTIONS:\n" );

	for ( sw i = 0; i < array_count( opts->entries ); ++i )
	{
		opts_entry* e = opts->entries + i;

		if ( e->name )
		{
			if ( e->lname )
			{
				zpl_printf( "\t-%s, --%s: %s\n", e->name, e->lname, e->desc );
			}
			else
			{
				zpl_printf( "\t-%s: %s\n", e->name, e->desc );
			}
		}
		else
		{
			zpl_printf( "\t--%s: %s\n", e->lname, e->desc );
		}
	}
}

void opts_print_errors( zpl_opts* opts )
{
	for ( int i = 0; i < array_count( opts->errors ); ++i )
	{
		opts_err* err = ( opts->errors + i );

		zpl_printf( "ERROR: " );

		switch ( err->type )
		{
			case ZPL_OPTS_ERR_OPTION :
				zpl_printf( "Invalid option \"%s\"", err->val );
				break;

			case ZPL_OPTS_ERR_VALUE :
				zpl_printf( "Invalid value \"%s\"", err->val );
				break;

			case ZPL_OPTS_ERR_MISSING_VALUE :
				zpl_printf( "Missing value for option \"%s\"", err->val );
				break;

			case ZPL_OPTS_ERR_EXTRA_VALUE :
				zpl_printf( "Extra value for option \"%s\"", err->val );
				break;
		}

		zpl_printf( "\n" );
	}
}

void zpl__opts_push_error( zpl_opts* opts, char* b, u8 errtype )
{
	opts_err err = { 0 };
	err.val      = b;
	err.type     = errtype;
	array_append( opts->errors, err );
}

b32 opts_compile( zpl_opts* opts, int argc, char** argv )
{
	b32 had_errors = false;
	for ( int i = 1; i < argc; ++i )
	{
		char* p = argv[ i ];

		if ( *p )
		{
			p = zpl_cast( char* ) str_trim( p, false );
			if ( *p == '-' )
			{
				opts_entry* t       = 0;
				b32         checkln = false;
				if ( *( p + 1 ) == '-' )
				{
					checkln = true;
					++p;
				}

				char *b = p + 1, *e = b;

				while ( char_is_alphanumeric( *e ) || *e == '-' || *e == '_' )
				{
					++e;
				}

				t = zpl__opts_find( opts, b, ( e - b ), checkln );

				if ( t )
				{
					char* ob = b;
					b        = e;

					/**/ if ( *e == '=' )
					{
						if ( t->type == ZPL_OPTS_FLAG )
						{
							*e = '\0';
							zpl__opts_push_error( opts, ob, ZPL_OPTS_ERR_EXTRA_VALUE );
							had_errors = true;
							continue;
						}

						b = e = e + 1;
					}
					else if ( *e == '\0' )
					{
						char* sp = argv[ i + 1 ];

						if ( sp && *sp != '-' && ( array_count( opts->positioned ) < 1 || t->type != ZPL_OPTS_FLAG ) )
						{
							if ( t->type == ZPL_OPTS_FLAG )
							{
								zpl__opts_push_error( opts, b, ZPL_OPTS_ERR_EXTRA_VALUE );
								had_errors = true;
								continue;
							}

							p = sp;
							b = e = sp;
							++i;
						}
						else
						{
							if ( t->type != ZPL_OPTS_FLAG )
							{
								zpl__opts_push_error( opts, ob, ZPL_OPTS_ERR_MISSING_VALUE );
								had_errors = true;
								continue;
							}
							t->met = true;
							continue;
						}
					}

					e = zpl_cast( char* ) str_control_skip( e, '\0' );
					zpl__opts_set_value( opts, t, b );
				}
				else
				{
					zpl__opts_push_error( opts, b, ZPL_OPTS_ERR_OPTION );
					had_errors = true;
				}
			}
			else if ( array_count( opts->positioned ) )
			{
				opts_entry* l = array_back( opts->positioned );
				array_pop( opts->positioned );
				zpl__opts_set_value( opts, l, p );
			}
			else
			{
				zpl__opts_push_error( opts, p, ZPL_OPTS_ERR_VALUE );
				had_errors = true;
			}
		}
	}
	return ! had_errors;
}

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
