// file: source/adt.c

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

#define zpl__adt_fprintf( s_, fmt_, ... )                                                                                                                                          \
	do                                                                                                                                                                             \
	{                                                                                                                                                                              \
		if ( zpl_fprintf( s_, fmt_, ##__VA_ARGS__ ) < 0 )                                                                                                                          \
			return ZPL_ADT_ERROR_OUT_OF_MEMORY;                                                                                                                                    \
	} while ( 0 )

u8 adt_make_branch( adt_node* node, zpl_allocator backing, char const * name, b32 is_array )
{
	u8 type = ZPL_ADT_TYPE_OBJECT;
	if ( is_array )
	{
		type = ZPL_ADT_TYPE_ARRAY;
	}
	adt_node* parent = node->parent;
	zero_item( node );
	node->type   = type;
	node->name   = name;
	node->parent = parent;
	if ( ! array_init( node->nodes, backing ) )
		return ZPL_ADT_ERROR_OUT_OF_MEMORY;
	return 0;
}

u8 adt_destroy_branch( adt_node* node )
{
	ZPL_ASSERT_NOT_NULL( node );
	if ( ( node->type == ZPL_ADT_TYPE_OBJECT || node->type == ZPL_ADT_TYPE_ARRAY ) && node->nodes )
	{
		for ( sw i = 0; i < array_count( node->nodes ); ++i )
		{
			adt_destroy_branch( node->nodes + i );
		}

		array_free( node->nodes );
	}
	return 0;
}

u8 adt_make_leaf( adt_node* node, char const * name, u8 type )
{
	ZPL_ASSERT( type != ZPL_ADT_TYPE_OBJECT && type != ZPL_ADT_TYPE_ARRAY );
	adt_node* parent = node->parent;
	zero_item( node );
	node->type   = type;
	node->name   = name;
	node->parent = parent;
	return 0;
}

adt_node* adt_find( adt_node* node, char const * name, b32 deep_search )
{
	if ( node->type != ZPL_ADT_TYPE_OBJECT )
	{
		return NULL;
	}

	for ( sw i = 0; i < array_count( node->nodes ); i++ )
	{
		if ( ! str_compare( node->nodes[ i ].name, name ) )
		{
			return ( node->nodes + i );
		}
	}

	if ( deep_search )
	{
		for ( sw i = 0; i < array_count( node->nodes ); i++ )
		{
			adt_node* res = adt_find( node->nodes + i, name, deep_search );

			if ( res != NULL )
				return res;
		}
	}

	return NULL;
}

internal adt_node* zpl__adt_get_value( adt_node* node, char const * value )
{
	switch ( node->type )
	{
		case ZPL_ADT_TYPE_MULTISTRING :
		case ZPL_ADT_TYPE_STRING :
			{
				if ( node->string && ! str_compare( node->string, value ) )
				{
					return node;
				}
			}
			break;
		case ZPL_ADT_TYPE_INTEGER :
		case ZPL_ADT_TYPE_REAL :
			{
				char     back[ 4096 ] = { 0 };
				zpl_file tmp;

				/* allocate a file descriptor for a memory-mapped number to string conversion, input source buffer is not cloned, however. */
				file_stream_open( &tmp, heap(), (u8*)back, size_of( back ), ZPL_FILE_STREAM_WRITABLE );
				adt_print_number( &tmp, node );

				sw  fsize = 0;
				u8* buf   = file_stream_buf( &tmp, &fsize );

				if ( ! str_compare( (char const *)buf, value ) )
				{
					file_close( &tmp );
					return node;
				}

				file_close( &tmp );
			}
			break;
		default :
			break; /* node doesn't support value based lookup */
	}

	return NULL;
}

internal adt_node* zpl__adt_get_field( adt_node* node, char* name, char* value )
{
	for ( sw i = 0; i < array_count( node->nodes ); i++ )
	{
		if ( ! str_compare( node->nodes[ i ].name, name ) )
		{
			adt_node* child = &node->nodes[ i ];
			if ( zpl__adt_get_value( child, value ) )
			{
				return node; /* this object does contain a field of a specified value! */
			}
		}
	}

	return NULL;
}

adt_node* adt_query( adt_node* node, char const * uri )
{
	ZPL_ASSERT_NOT_NULL( uri );

	if ( *uri == '/' )
	{
		uri++;
	}

	if ( *uri == 0 )
	{
		return node;
	}

	if ( ! node || ( node->type != ZPL_ADT_TYPE_OBJECT && node->type != ZPL_ADT_TYPE_ARRAY ) )
	{
		return NULL;
	}

#if defined ZPL_ADT_URI_DEBUG || 0
	zpl_printf( "uri: %s\n", uri );
#endif

	char *    p = (char*)uri, *b = p, *e = p;
	adt_node* found_node = NULL;

	b = p;
	p = e     = (char*)str_skip( p, '/' );
	char* buf = bprintf( "%.*s", (int)( e - b ), b );

	/* handle field value lookup */
	if ( *b == '[' )
	{
		char *l_p = buf + 1, *l_b = l_p, *l_e = l_p, *l_b2 = l_p, *l_e2 = l_p;
		l_e  = (char*)str_skip( l_p, '=' );
		l_e2 = (char*)str_skip( l_p, ']' );

		if ( ( ! *l_e && node->type != ZPL_ADT_TYPE_ARRAY ) || ! *l_e2 )
		{
			ZPL_ASSERT_MSG( 0, "Invalid field value lookup" );
			return NULL;
		}

		*l_e2 = 0;

		/* [field=value] */
		if ( *l_e )
		{
			*l_e = 0;
			l_b2 = l_e + 1;

			/* run a value comparison against our own fields */
			if ( node->type == ZPL_ADT_TYPE_OBJECT )
			{
				found_node = zpl__adt_get_field( node, l_b, l_b2 );
			}

			/* run a value comparison against any child that is an object node */
			else if ( node->type == ZPL_ADT_TYPE_ARRAY )
			{
				for ( sw i = 0; i < array_count( node->nodes ); i++ )
				{
					adt_node* child = &node->nodes[ i ];
					if ( child->type != ZPL_ADT_TYPE_OBJECT )
					{
						continue;
					}

					found_node = zpl__adt_get_field( child, l_b, l_b2 );

					if ( found_node )
						break;
				}
			}
		}
		/* [value] */
		else
		{
			for ( sw i = 0; i < array_count( node->nodes ); i++ )
			{
				adt_node* child = &node->nodes[ i ];
				if ( zpl__adt_get_value( child, l_b2 ) )
				{
					found_node = child;
					break; /* we found a matching value in array, ignore the rest of it */
				}
			}
		}

		/* go deeper if uri continues */
		if ( *e )
		{
			return adt_query( found_node, e + 1 );
		}
	}
	/* handle field name lookup */
	else if ( node->type == ZPL_ADT_TYPE_OBJECT )
	{
		found_node = adt_find( node, buf, false );

		/* go deeper if uri continues */
		if ( *e )
		{
			return adt_query( found_node, e + 1 );
		}
	}
	/* handle array index lookup */
	else
	{
		sw idx = (sw)str_to_i64( buf, NULL, 10 );
		if ( idx >= 0 && idx < array_count( node->nodes ) )
		{
			found_node = &node->nodes[ idx ];

			/* go deeper if uri continues */
			if ( *e )
			{
				return adt_query( found_node, e + 1 );
			}
		}
	}

	return found_node;
}

adt_node* adt_alloc_at( adt_node* parent, sw index )
{
	if ( ! parent || ( parent->type != ZPL_ADT_TYPE_OBJECT && parent->type != ZPL_ADT_TYPE_ARRAY ) )
	{
		return NULL;
	}

	if ( ! parent->nodes )
		return NULL;

	if ( index < 0 || index > array_count( parent->nodes ) )
		return NULL;

	adt_node o = { 0 };
	o.parent   = parent;
	if ( ! array_append_at( parent->nodes, o, index ) )
		return NULL;

	return parent->nodes + index;
}

adt_node* adt_alloc( adt_node* parent )
{
	if ( ! parent || ( parent->type != ZPL_ADT_TYPE_OBJECT && parent->type != ZPL_ADT_TYPE_ARRAY ) )
	{
		return NULL;
	}

	if ( ! parent->nodes )
		return NULL;

	return adt_alloc_at( parent, array_count( parent->nodes ) );
}

b8 adt_set_obj( adt_node* obj, char const * name, zpl_allocator backing )
{
	return adt_make_branch( obj, backing, name, 0 );
}
b8 adt_set_arr( adt_node* obj, char const * name, zpl_allocator backing )
{
	return adt_make_branch( obj, backing, name, 1 );
}
b8 adt_set_str( adt_node* obj, char const * name, char const * value )
{
	adt_make_leaf( obj, name, ZPL_ADT_TYPE_STRING );
	obj->string = value;
	return true;
}
b8 adt_set_flt( adt_node* obj, char const * name, f64 value )
{
	adt_make_leaf( obj, name, ZPL_ADT_TYPE_REAL );
	obj->real = value;
	return true;
}
b8 adt_set_int( adt_node* obj, char const * name, s64 value )
{
	adt_make_leaf( obj, name, ZPL_ADT_TYPE_INTEGER );
	obj->integer = value;
	return true;
}

adt_node* adt_move_node_at( adt_node* node, adt_node* new_parent, sw index )
{
	ZPL_ASSERT_NOT_NULL( node );
	ZPL_ASSERT_NOT_NULL( new_parent );
	adt_node* old_parent = node->parent;
	adt_node* new_node   = adt_alloc_at( new_parent, index );
	*new_node            = *node;
	new_node->parent     = new_parent;
	if ( old_parent )
	{
		adt_remove_node( node );
	}
	return new_node;
}

adt_node* adt_move_node( adt_node* node, adt_node* new_parent )
{
	ZPL_ASSERT_NOT_NULL( node );
	ZPL_ASSERT_NOT_NULL( new_parent );
	ZPL_ASSERT( new_parent->type == ZPL_ADT_TYPE_ARRAY || new_parent->type == ZPL_ADT_TYPE_OBJECT );
	return adt_move_node_at( node, new_parent, array_count( new_parent->nodes ) );
}

void adt_swap_nodes( adt_node* node, adt_node* other_node )
{
	ZPL_ASSERT_NOT_NULL( node );
	ZPL_ASSERT_NOT_NULL( other_node );
	adt_node* parent                     = node->parent;
	adt_node* other_parent               = other_node->parent;
	sw        index                      = ( pointer_diff( parent->nodes, node ) / size_of( adt_node ) );
	sw        index2                     = ( pointer_diff( other_parent->nodes, other_node ) / size_of( adt_node ) );
	adt_node  temp                       = parent->nodes[ index ];
	temp.parent                          = other_parent;
	other_parent->nodes[ index2 ].parent = parent;
	parent->nodes[ index ]               = other_parent->nodes[ index2 ];
	other_parent->nodes[ index2 ]        = temp;
}

void adt_remove_node( adt_node* node )
{
	ZPL_ASSERT_NOT_NULL( node );
	ZPL_ASSERT_NOT_NULL( node->parent );
	adt_node* parent = node->parent;
	sw        index  = ( pointer_diff( parent->nodes, node ) / size_of( adt_node ) );
	array_remove_at( parent->nodes, index );
}

adt_node* adt_append_obj( adt_node* parent, char const * name )
{
	adt_node* o = adt_alloc( parent );
	if ( ! o )
		return NULL;
	if ( adt_set_obj( o, name, ZPL_ARRAY_HEADER( parent->nodes )->allocator ) )
	{
		adt_remove_node( o );
		return NULL;
	}
	return o;
}
adt_node* adt_append_arr( adt_node* parent, char const * name )
{
	adt_node* o = adt_alloc( parent );
	if ( ! o )
		return NULL;
	if ( adt_set_arr( o, name, ZPL_ARRAY_HEADER( parent->nodes )->allocator ) )
	{
		adt_remove_node( o );
		return NULL;
	}
	return o;
}
adt_node* adt_append_str( adt_node* parent, char const * name, char const * value )
{
	adt_node* o = adt_alloc( parent );
	if ( ! o )
		return NULL;
	adt_set_str( o, name, value );
	return o;
}
adt_node* adt_append_flt( adt_node* parent, char const * name, f64 value )
{
	adt_node* o = adt_alloc( parent );
	if ( ! o )
		return NULL;
	adt_set_flt( o, name, value );
	return o;
}
adt_node* adt_append_int( adt_node* parent, char const * name, s64 value )
{
	adt_node* o = adt_alloc( parent );
	if ( ! o )
		return NULL;
	adt_set_int( o, name, value );
	return o;
}

/* parser helpers */

char* adt_parse_number( adt_node* node, char* base_str )
{
	ZPL_ASSERT_NOT_NULL( node );
	ZPL_ASSERT_NOT_NULL( base_str );
	char *p = base_str, *e = p;

	s32 base         = 0;
	s32 base2        = 0;
	u8  base2_offset = 0;
	s8  exp = 0, orig_exp = 0;
	u8  neg_zero   = 0;
	u8  lead_digit = 0;
	u8  node_type  = 0;
	u8  node_props = 0;

	/* skip false positives and special cases */
	if ( ! ! strchr( "eE", *p ) || ( ! ! strchr( ".+-", *p ) && ! char_is_hex_digit( *( p + 1 ) ) && *( p + 1 ) != '.' ) )
	{
		return ++base_str;
	}

	node_type = ZPL_ADT_TYPE_INTEGER;
	neg_zero  = false;

	sw   ib        = 0;
	char buf[ 48 ] = { 0 };

	if ( *e == '+' )
		++e;
	else if ( *e == '-' )
	{
		buf[ ib++ ] = *e++;
	}

	if ( *e == '.' )
	{
		node_type   = ZPL_ADT_TYPE_REAL;
		node_props  = ZPL_ADT_PROPS_IS_PARSED_REAL;
		lead_digit  = false;
		buf[ ib++ ] = '0';
		do
		{
			buf[ ib++ ] = *e;
		} while ( char_is_digit( *++e ) );
	}
	else
	{
		if ( ! str_compare( e, "0x", 2 ) || ! str_compare( e, "0X", 2 ) )
		{
			node_props = ZPL_ADT_PROPS_IS_HEX;
		}
		while ( char_is_hex_digit( *e ) || char_to_lower( *e ) == 'x' )
		{
			buf[ ib++ ] = *e++;
		}

		if ( *e == '.' )
		{
			node_type  = ZPL_ADT_TYPE_REAL;
			lead_digit = true;
			u32 step   = 0;

			do
			{
				buf[ ib++ ] = *e;
				++step;
			} while ( char_is_digit( *++e ) );

			if ( step < 2 )
			{
				buf[ ib++ ] = '0';
			}
		}
	}

	/* check if we have a dot here, this is a false positive (IP address, ...) */
	if ( *e == '.' )
	{
		return ++base_str;
	}

	f32  eb          = 10;
	char expbuf[ 6 ] = { 0 };
	sw   expi        = 0;

	if ( *e && ! ! strchr( "eE", *e ) )
	{
		++e;
		if ( *e == '+' || *e == '-' || char_is_digit( *e ) )
		{
			if ( *e == '-' )
			{
				eb = 0.1f;
			}
			if ( ! char_is_digit( *e ) )
			{
				++e;
			}
			while ( char_is_digit( *e ) )
			{
				expbuf[ expi++ ] = *e++;
			}
		}

		orig_exp = exp = (u8)str_to_i64( expbuf, NULL, 10 );
	}

	if ( node_type == ZPL_ADT_TYPE_INTEGER )
	{
		node->integer = str_to_i64( buf, 0, 0 );
#ifndef ZPL_PARSER_DISABLE_ANALYSIS
		/* special case: negative zero */
		if ( node->integer == 0 && buf[ 0 ] == '-' )
		{
			neg_zero = true;
		}
#endif
		while ( orig_exp-- > 0 )
		{
			node->integer *= (s64)eb;
		}
	}
	else
	{
		node->real = str_to_f64( buf, 0 );

#ifndef ZPL_PARSER_DISABLE_ANALYSIS
		char *q = buf, *base_string = q, *base_string2 = q;
		base_string           = zpl_cast( char* ) str_skip( base_string, '.' );
		*base_string          = '\0';
		base_string2          = base_string + 1;
		char* base_string_off = base_string2;
		while ( *base_string_off++ == '0' )
			base2_offset++;

		base  = (s32)str_to_i64( q, 0, 0 );
		base2 = (s32)str_to_i64( base_string2, 0, 0 );
		if ( exp )
		{
			exp        = exp * ( ! ( eb == 10.0f ) ? -1 : 1 );
			node_props = ZPL_ADT_PROPS_IS_EXP;
		}

		/* special case: negative zero */
		if ( base == 0 && buf[ 0 ] == '-' )
		{
			neg_zero = true;
		}
#endif
		while ( orig_exp-- > 0 )
		{
			node->real *= eb;
		}
	}

	node->type  = node_type;
	node->props = node_props;

#ifndef ZPL_PARSER_DISABLE_ANALYSIS
	node->base         = base;
	node->base2        = base2;
	node->base2_offset = base2_offset;
	node->exp          = exp;
	node->neg_zero     = neg_zero;
	node->lead_digit   = lead_digit;
#else
	unused( base );
	unused( base2 );
	unused( base2_offset );
	unused( exp );
	unused( neg_zero );
	unused( lead_digit );
#endif
	return e;
}

adt_error adt_print_number( zpl_file* file, adt_node* node )
{
	ZPL_ASSERT_NOT_NULL( file );
	ZPL_ASSERT_NOT_NULL( node );
	if ( node->type != ZPL_ADT_TYPE_INTEGER && node->type != ZPL_ADT_TYPE_REAL )
	{
		return ZPL_ADT_ERROR_INVALID_TYPE;
	}

#ifndef ZPL_PARSER_DISABLE_ANALYSIS
	if ( node->neg_zero )
	{
		zpl__adt_fprintf( file, "-" );
	}
#endif

	switch ( node->type )
	{
		case ZPL_ADT_TYPE_INTEGER :
			{
				if ( node->props == ZPL_ADT_PROPS_IS_HEX )
				{
					zpl__adt_fprintf( file, "0x%llx", (long long)node->integer );
				}
				else
				{
					zpl__adt_fprintf( file, "%lld", (long long)node->integer );
				}
			}
			break;

		case ZPL_ADT_TYPE_REAL :
			{
				if ( node->props == ZPL_ADT_PROPS_NAN )
				{
					zpl__adt_fprintf( file, "NaN" );
				}
				else if ( node->props == ZPL_ADT_PROPS_NAN_NEG )
				{
					zpl__adt_fprintf( file, "-NaN" );
				}
				else if ( node->props == ZPL_ADT_PROPS_INFINITY )
				{
					zpl__adt_fprintf( file, "Infinity" );
				}
				else if ( node->props == ZPL_ADT_PROPS_INFINITY_NEG )
				{
					zpl__adt_fprintf( file, "-Infinity" );
				}
				else if ( node->props == ZPL_ADT_PROPS_TRUE )
				{
					zpl__adt_fprintf( file, "true" );
				}
				else if ( node->props == ZPL_ADT_PROPS_FALSE )
				{
					zpl__adt_fprintf( file, "false" );
				}
				else if ( node->props == ZPL_ADT_PROPS_NULL )
				{
					zpl__adt_fprintf( file, "null" );
#ifndef ZPL_PARSER_DISABLE_ANALYSIS
				}
				else if ( node->props == ZPL_ADT_PROPS_IS_EXP )
				{
					zpl__adt_fprintf( file, "%lld.%0*d%llde%lld", (long long)node->base, node->base2_offset, 0, (long long)node->base2, (long long)node->exp );
				}
				else if ( node->props == ZPL_ADT_PROPS_IS_PARSED_REAL )
				{
					if ( ! node->lead_digit )
						zpl__adt_fprintf( file, ".%0*d%lld", node->base2_offset, 0, (long long)node->base2 );
					else
						zpl__adt_fprintf( file, "%lld.%0*d%lld", (long long int)node->base2_offset, 0, (int)node->base, (long long)node->base2 );
#endif
				}
				else
				{
					zpl__adt_fprintf( file, "%f", node->real );
				}
			}
			break;
	}

	return ZPL_ADT_ERROR_NONE;
}

adt_error adt_print_string( zpl_file* file, adt_node* node, char const * escaped_chars, char const * escape_symbol )
{
	ZPL_ASSERT_NOT_NULL( file );
	ZPL_ASSERT_NOT_NULL( node );
	ZPL_ASSERT_NOT_NULL( escaped_chars );
	if ( node->type != ZPL_ADT_TYPE_STRING && node->type != ZPL_ADT_TYPE_MULTISTRING )
	{
		return ZPL_ADT_ERROR_INVALID_TYPE;
	}

	/* escape string */
	char const *p = node->string, *b = p;
	do
	{
		p = str_skip_any( p, escaped_chars );
		zpl__adt_fprintf( file, "%.*s", ptr_diff( b, p ), b );
		if ( *p && ! ! strchr( escaped_chars, *p ) )
		{
			zpl__adt_fprintf( file, "%s%c", escape_symbol, *p );
			p++;
		}
		b = p;
	} while ( *p );

	return ZPL_ADT_ERROR_NONE;
}

adt_error adt_str_to_number( adt_node* node )
{
	ZPL_ASSERT( node );

	if ( node->type == ZPL_ADT_TYPE_REAL || node->type == ZPL_ADT_TYPE_INTEGER )
		return ZPL_ADT_ERROR_ALREADY_CONVERTED; /* this is already converted/parsed */
	if ( node->type != ZPL_ADT_TYPE_STRING && node->type != ZPL_ADT_TYPE_MULTISTRING )
	{
		return ZPL_ADT_ERROR_INVALID_TYPE;
	}

	adt_parse_number( node, (char*)node->string );

	return ZPL_ADT_ERROR_NONE;
}

#undef zpl__adt_fprintf

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
