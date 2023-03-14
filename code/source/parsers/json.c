// file: source/parsers/json.c

////////////////////////////////////////////////////////////////
//
// JSON5 Parser
//
//

#ifdef ZPL_JSON_DEBUG
#define ZPL_JSON_ASSERT( msg ) ZPL_PANIC( msg )
#else
#define ZPL_JSON_ASSERT( msg )
#endif

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

char* zpl__json_parse_object( adt_node* obj, char* base, zpl_allocator a, u8* err_code );
char* zpl__json_parse_array( adt_node* obj, char* base, zpl_allocator a, u8* err_code );
char* zpl__json_parse_value( adt_node* obj, char* base, zpl_allocator a, u8* err_code );
char* zpl__json_parse_name( adt_node* obj, char* base, u8* err_code );
char* zpl__json_trim( char* base, b32 catch_newline );
b8    zpl__json_write_value( zpl_file* f, adt_node* o, adt_node* t, sw indent, b32 is_inline, b32 is_last );

#define zpl__json_fprintf( s_, fmt_, ... )                                                                                                                                         \
	do                                                                                                                                                                             \
	{                                                                                                                                                                              \
		if ( zpl_fprintf( s_, fmt_, ##__VA_ARGS__ ) < 0 )                                                                                                                          \
			return false;                                                                                                                                                          \
	} while ( 0 )

#define zpl___ind( x )                                                                                                                                                             \
	if ( x > 0 )                                                                                                                                                                   \
		zpl__json_fprintf( f, "%*r", x, ' ' );

u8 json_parse( adt_node* root, char* text, zpl_allocator a )
{
	u8 err_code = ZPL_JSON_ERROR_NONE;
	ZPL_ASSERT( root );
	ZPL_ASSERT( text );
	zero_item( root );
	text = zpl__json_trim( text, true );

#ifndef ZPL_PARSER_DISABLE_ANALYSIS
	if ( ! strchr( "{[", *text ) )
	{
		root->cfg_mode = true;
	}
#endif

	zpl__json_parse_object( root, text, a, &err_code );
	return err_code;
}

void json_free( adt_node* obj )
{
	adt_destroy_branch( obj );
}

string json_write_string( zpl_allocator a, adt_node* obj, sw indent )
{
	zpl_file tmp;
	if ( ! file_stream_new( &tmp, a ) )
		return NULL;
	if ( ! json_write( &tmp, obj, indent ) )
		return NULL;
	sw     fsize;
	u8*    buf    = file_stream_buf( &tmp, &fsize );
	string output = string_make_length( a, (char*)buf, fsize );
	file_close( &tmp );
	return output;
}

/* private */

#define zpl__json_append_node( x, item )                                                                                                                                           \
	do                                                                                                                                                                             \
	{                                                                                                                                                                              \
		if ( ! array_append( x, item ) )                                                                                                                                           \
		{                                                                                                                                                                          \
			*err_code = ZPL_JSON_ERROR_OUT_OF_MEMORY;                                                                                                                              \
			return NULL;                                                                                                                                                           \
		}                                                                                                                                                                          \
		if ( item.type == ZPL_ADT_TYPE_OBJECT || item.type == ZPL_ADT_TYPE_ARRAY )                                                                                                 \
		{                                                                                                                                                                          \
			for ( sw i = 0; i < array_count( item.nodes ); i++ )                                                                                                                   \
				item.nodes[ i ].parent = array_end( x );                                                                                                                           \
		}                                                                                                                                                                          \
	} while ( 0 );

static ZPL_ALWAYS_INLINE b32 zpl__json_is_assign_char( char c )
{
	return ! ! strchr( ":=|", c );
}
static ZPL_ALWAYS_INLINE b32 zpl__json_is_delim_char( char c )
{
	return ! ! strchr( ",|\n", c );
}
ZPL_DEF_INLINE b32 zpl__json_validate_name( char const * str, char* err );

#define jx( x ) ! char_is_hex_digit( str[ x ] )
ZPL_IMPL_INLINE b32 zpl__json_validate_name( char const * str, char* err )
{
	while ( *str )
	{
		/* todo: refactor name validation. */
		if ( ( str[ 0 ] == '\\' && ! char_is_control( str[ 1 ] ) ) && ( str[ 0 ] == '\\' && jx( 1 ) && jx( 2 ) && jx( 3 ) && jx( 4 ) ) )
		{
			if ( err )
				*err = *str;
			return false;
		}

		++str;
	}

	return true;
}
#undef jx

char* zpl__json_parse_array( adt_node* obj, char* base, zpl_allocator a, u8* err_code )
{
	ZPL_ASSERT( obj && base );
	char* p = base;

	obj->type = ZPL_ADT_TYPE_ARRAY;
	if ( ! array_init( obj->nodes, a ) )
	{
		*err_code = ZPL_JSON_ERROR_OUT_OF_MEMORY;
		return NULL;
	}

	while ( *p )
	{
		p = zpl__json_trim( p, false );

		if ( *p == ']' )
		{
			return p;
		}

		adt_node elem = { 0 };
		p             = zpl__json_parse_value( &elem, p, a, err_code );

		if ( *err_code != ZPL_JSON_ERROR_NONE )
		{
			return NULL;
		}

		zpl__json_append_node( obj->nodes, elem );

		p = zpl__json_trim( p, false );

		if ( *p == ',' )
		{
			++p;
			continue;
		}
		else
		{
			if ( *p != ']' )
			{
				ZPL_JSON_ASSERT( "end of array unfulfilled" );
				*err_code = ZPL_JSON_ERROR_ARRAY_LEFT_OPEN;
				return NULL;
			}
			return p;
		}
	}

	*err_code = ZPL_JSON_ERROR_INTERNAL;
	return NULL;
}

char* zpl__json_parse_value( adt_node* obj, char* base, zpl_allocator a, u8* err_code )
{
	ZPL_ASSERT( obj && base );
	char *p = base, *b = p, *e = p;

	/* handle quoted strings */
	if ( ! ! strchr( "`\"'", *p ) )
	{
		char c    = *p;
		obj->type = ( c == '`' ) ? ZPL_ADT_TYPE_MULTISTRING : ZPL_ADT_TYPE_STRING;
		b = e       = p + 1;
		obj->string = b;
		e           = zpl_cast( char* ) str_skip_literal( e, c );
		*e = '\0', p = e + 1;
	}
	else if ( char_is_alpha( *p ) || ( *p == '-' && ! char_is_digit( *( p + 1 ) ) ) )
	{
		/* handle constants */
		if ( str_has_prefix( p, "true" ) )
		{
			obj->type  = ZPL_ADT_TYPE_REAL;
			obj->props = ZPL_ADT_PROPS_TRUE;
			obj->real  = 1;
			p += 4;
		}
		else if ( str_has_prefix( p, "false" ) )
		{
			obj->type  = ZPL_ADT_TYPE_REAL;
			obj->props = ZPL_ADT_PROPS_FALSE;
			obj->real  = 0;
			p += 5;
		}
		else if ( str_has_prefix( p, "null" ) )
		{
			obj->type  = ZPL_ADT_TYPE_REAL;
			obj->props = ZPL_ADT_PROPS_NULL;
			obj->real  = 0;
			p += 4;
		}
		else if ( str_has_prefix( p, "Infinity" ) )
		{
			obj->type  = ZPL_ADT_TYPE_REAL;
			obj->real  = ZPL_INFINITY;
			obj->props = ZPL_ADT_PROPS_INFINITY;
			p += 8;
		}
		else if ( str_has_prefix( p, "-Infinity" ) )
		{
			obj->type  = ZPL_ADT_TYPE_REAL;
			obj->real  = -ZPL_INFINITY;
			obj->props = ZPL_ADT_PROPS_INFINITY_NEG;
			p += 9;
		}
		else if ( str_has_prefix( p, "NaN" ) )
		{
			obj->type  = ZPL_ADT_TYPE_REAL;
			obj->real  = ZPL_NAN;
			obj->props = ZPL_ADT_PROPS_NAN;
			p += 3;
		}
		else if ( str_has_prefix( p, "-NaN" ) )
		{
			obj->type  = ZPL_ADT_TYPE_REAL;
			obj->real  = -ZPL_NAN;
			obj->props = ZPL_ADT_PROPS_NAN_NEG;
			p += 4;
		}
		else
		{
			ZPL_JSON_ASSERT( "unknown keyword" );
			*err_code = ZPL_JSON_ERROR_UNKNOWN_KEYWORD;
			return NULL;
		}
	}
	else if ( char_is_digit( *p ) || *p == '+' || *p == '-' || *p == '.' )
	{
		/* handle numbers */
		/* defer operation to our helper method. */
		p = adt_parse_number( obj, p );
	}
	else if ( ! ! strchr( "[{", *p ) )
	{
		/* handle compound objects */
		p = zpl__json_parse_object( obj, p, a, err_code );
		++p;
	}

	return p;
}

char* zpl__json_parse_object( adt_node* obj, char* base, zpl_allocator a, u8* err_code )
{
	ZPL_ASSERT( obj && base );
	char* p = base;

	p = zpl__json_trim( p, false );
	/**/ if ( *p == '{' )
	{
		++p;
	}
	else if ( *p == '[' )
	{ /* special case for when we call this func on an array. */
		++p;
		obj->type = ZPL_ADT_TYPE_ARRAY;
		return zpl__json_parse_array( obj, p, a, err_code );
	}

	if ( ! array_init( obj->nodes, a ) )
	{
		*err_code = ZPL_JSON_ERROR_OUT_OF_MEMORY;
		return NULL;
	}
	obj->type = ZPL_ADT_TYPE_OBJECT;

	do
	{
		adt_node node = { 0 };
		p             = zpl__json_trim( p, false );
		if ( *p == '}' && obj->type == ZPL_ADT_TYPE_OBJECT )
			return p;
		else if ( *p == ']' && obj->type == ZPL_ADT_TYPE_ARRAY )
			return p;
		else if ( ! ! strchr( "}]", *p ) )
		{
			ZPL_JSON_ASSERT( "mismatched end pair" );
			*err_code = ZPL_JSON_ERROR_OBJECT_END_PAIR_MISMATCHED;
			return NULL;
		}

		/* First, we parse the key, then we proceed to the value itself. */
		p = zpl__json_parse_name( &node, p, err_code );
		if ( err_code && *err_code != ZPL_JSON_ERROR_NONE )
		{
			return NULL;
		}
		p = zpl__json_trim( p + 1, false );
		p = zpl__json_parse_value( &node, p, a, err_code );
		if ( err_code && *err_code != ZPL_JSON_ERROR_NONE )
		{
			return NULL;
		}

		zpl__json_append_node( obj->nodes, node );

		char* end_p = p;
		unused( end_p );
		p = zpl__json_trim( p, true );

		/* this code analyses the keyvalue pair delimiter used in the packet. */
		if ( zpl__json_is_delim_char( *p ) )
		{
#ifndef ZPL_PARSER_DISABLE_ANALYSIS
			adt_node* n    = array_end( obj->nodes );
			n->delim_style = ZPL_ADT_DELIM_STYLE_COMMA;

			if ( *p == '\n' )
				n->delim_style = ZPL_ADT_DELIM_STYLE_NEWLINE;
			else if ( *p == '|' )
			{
				n->delim_style      = ZPL_ADT_DELIM_STYLE_LINE;
				n->delim_line_width = zpl_cast( u8 )( p - end_p );
			}
#endif
			++p;
		}
		p = zpl__json_trim( p, false );
	} while ( *p );
	return p;
}

char* zpl__json_parse_name( adt_node* node, char* base, u8* err_code )
{
	char *p = base, *b = p, *e = p;
	u8    name_style = 0;

	if ( *p == '"' || *p == '\'' || char_is_alpha( *p ) || *p == '_' || *p == '$' )
	{
		if ( *p == '"' || *p == '\'' )
		{
#ifndef ZPL_PARSER_DISABLE_ANALYSIS
			if ( *p == '"' )
			{
				node->name_style = ZPL_ADT_NAME_STYLE_DOUBLE_QUOTE;
			}
			else if ( *p == '\'' )
			{
				node->name_style = ZPL_ADT_NAME_STYLE_SINGLE_QUOTE;
			}
#endif
			char c     = *p;
			b          = ++p;
			e          = zpl_cast( char* ) str_control_skip( b, c );
			node->name = b;

			/* we can safely null-terminate here, since "e" points to the quote pair end. */
			*e++ = '\0';
		}
		else
		{
			b = e = p;
			str_advance_while( e, *e && ( char_is_alphanumeric( *e ) || *e == '_' ) && ! char_is_space( *e ) && ! zpl__json_is_assign_char( *e ) );
			node->name = b;
			name_style = ZPL_ADT_NAME_STYLE_NO_QUOTES;
			/* we defer null-termination as it can potentially wipe our assign char as well. */
		}

		char* assign_p = e;
		unused( assign_p );
		p = zpl__json_trim( e, false );
#ifndef ZPL_PARSER_DISABLE_ANALYSIS
		node->assign_line_width = zpl_cast( u8 )( p - assign_p );
#endif

		if ( *p && ! zpl__json_is_assign_char( *p ) )
		{
			ZPL_JSON_ASSERT( "invalid assignment" );
			*err_code = ZPL_JSON_ERROR_INVALID_ASSIGNMENT;
			return NULL;
		}
		else
		{
#ifndef ZPL_PARSER_DISABLE_ANALYSIS
			if ( *p == '=' )
				node->assign_style = ZPL_ADT_ASSIGN_STYLE_EQUALS;
			else if ( *p == '|' )
				node->assign_style = ZPL_ADT_ASSIGN_STYLE_LINE;
			else
				node->assign_style = ZPL_ADT_ASSIGN_STYLE_COLON;
#endif
		}

		/* since we already know the assign style, we can cut it here for unquoted names */
		if ( name_style == ZPL_ADT_NAME_STYLE_NO_QUOTES && *e )
		{
			*e = '\0';
#ifndef ZPL_PARSER_DISABLE_ANALYSIS
			node->name_style = name_style;
#endif
		}
	}

	if ( node->name && ! zpl__json_validate_name( node->name, NULL ) )
	{
		ZPL_JSON_ASSERT( "invalid name" );
		*err_code = ZPL_JSON_ERROR_INVALID_NAME;
		return NULL;
	}

	return p;
}

char* zpl__json_trim( char* base, b32 catch_newline )
{
	ZPL_ASSERT_NOT_NULL( base );
	char* p = base;
	do
	{
		if ( str_has_prefix( p, "//" ) )
		{
			const char* e = str_skip( p, '\n' );
			p += ( e - p );
		}
		else if ( str_has_prefix( p, "/*" ) )
		{
			const char* e = str_skip( p + 2, '*' );
			if ( *e && *( e + 1 ) == '/' )
			{
				e += 2; /* advance past end comment block */
				p += ( e - p );
			}
		}
		else if ( *p == '\n' && catch_newline )
		{
			return p;
		}
		else if ( ! char_is_space( *p ) )
		{
			return p;
		}
	} while ( *p++ );
	return NULL;
}

b8 json_write( zpl_file* f, adt_node* o, sw indent )
{
	if ( ! o )
		return true;

	ZPL_ASSERT( o->type == ZPL_ADT_TYPE_OBJECT || o->type == ZPL_ADT_TYPE_ARRAY );

	zpl___ind( indent - 4 );
#ifndef ZPL_PARSER_DISABLE_ANALYSIS
	if ( ! o->cfg_mode )
#else
	if ( 1 )
#endif
		zpl__json_fprintf( f, "%c\n", o->type == ZPL_ADT_TYPE_OBJECT ? '{' : '[' );
	else
	{
		indent -= 4;
	}

	if ( o->nodes )
	{
		sw cnt = array_count( o->nodes );

		for ( int i = 0; i < cnt; ++i )
		{
			if ( ! zpl__json_write_value( f, o->nodes + i, o, indent, false, ! ( i < cnt - 1 ) ) )
				return false;
		}
	}

	zpl___ind( indent );

	if ( indent > 0 )
	{
		zpl__json_fprintf( f, "%c", o->type == ZPL_ADT_TYPE_OBJECT ? '}' : ']' );
	}
	else
	{
#ifndef ZPL_PARSER_DISABLE_ANALYSIS
		if ( ! o->cfg_mode )
#endif
			zpl__json_fprintf( f, "%c\n", o->type == ZPL_ADT_TYPE_OBJECT ? '}' : ']' );
	}

	return true;
}

b8 zpl__json_write_value( zpl_file* f, adt_node* o, adt_node* t, sw indent, b32 is_inline, b32 is_last )
{
	adt_node* node = o;
	indent += 4;

	if ( ! is_inline )
	{
		zpl___ind( indent );

		if ( t->type != ZPL_ADT_TYPE_ARRAY )
		{
#ifndef ZPL_PARSER_DISABLE_ANALYSIS
			switch ( node->name_style )
			{
				case ZPL_ADT_NAME_STYLE_DOUBLE_QUOTE :
					{
						zpl__json_fprintf( f, "\"%s\"", node->name );
					}
					break;

				case ZPL_ADT_NAME_STYLE_SINGLE_QUOTE :
					{
						zpl__json_fprintf( f, "\'%s\'", node->name );
					}
					break;

				case ZPL_ADT_NAME_STYLE_NO_QUOTES :
					{
						zpl__json_fprintf( f, "%s", node->name );
					}
					break;
			}

			if ( o->assign_style == ZPL_ADT_ASSIGN_STYLE_COLON )
				zpl__json_fprintf( f, ": " );
			else
			{
				zpl___ind( max( o->assign_line_width, 1 ) );

				if ( o->assign_style == ZPL_ADT_ASSIGN_STYLE_EQUALS )
					zpl__json_fprintf( f, "= " );
				else if ( o->assign_style == ZPL_ADT_ASSIGN_STYLE_LINE )
				{
					zpl__json_fprintf( f, "| " );
				}
			}
#else
			zpl__json_fprintf( f, "\"%s\": ", node->name );
#endif
		}
	}

	switch ( node->type )
	{
		case ZPL_ADT_TYPE_STRING :
			{
				zpl__json_fprintf( f, "\"" );
				if ( adt_print_string( f, node, "\"", "\\" ) )
					return false;
				zpl__json_fprintf( f, "\"" );
			}
			break;

		case ZPL_ADT_TYPE_MULTISTRING :
			{
				zpl__json_fprintf( f, "`" );
				if ( adt_print_string( f, node, "`", "\\" ) )
					return false;
				zpl__json_fprintf( f, "`" );
			}
			break;

		case ZPL_ADT_TYPE_ARRAY :
			{
				zpl__json_fprintf( f, "[" );
				sw elemn = array_count( node->nodes );
				for ( int j = 0; j < elemn; ++j )
				{
					sw ind = ( ( node->nodes + j )->type == ZPL_ADT_TYPE_OBJECT || ( node->nodes + j )->type == ZPL_ADT_TYPE_ARRAY ) ? 0 : -4;
					if ( ! zpl__json_write_value( f, node->nodes + j, o, ind, true, true ) )
						return false;

					if ( j < elemn - 1 )
					{
						zpl__json_fprintf( f, ", " );
					}
				}
				zpl__json_fprintf( f, "]" );
			}
			break;

		case ZPL_ADT_TYPE_REAL :
		case ZPL_ADT_TYPE_INTEGER :
			{
				if ( adt_print_number( f, node ) )
					return false;
			}
			break;

		case ZPL_ADT_TYPE_OBJECT :
			{
				if ( ! json_write( f, node, indent ) )
					return false;
			}
			break;
	}

	if ( ! is_inline )
	{
#ifndef ZPL_PARSER_DISABLE_ANALYSIS
		if ( o->delim_style != ZPL_ADT_DELIM_STYLE_COMMA )
		{
			if ( o->delim_style == ZPL_ADT_DELIM_STYLE_NEWLINE )
				zpl__json_fprintf( f, "\n" );
			else if ( o->delim_style == ZPL_ADT_DELIM_STYLE_LINE )
			{
				zpl___ind( o->delim_line_width );
				zpl__json_fprintf( f, "|\n" );
			}
		}
		else
		{
			if ( ! is_last )
			{
				zpl__json_fprintf( f, ",\n" );
			}
			else
			{
				zpl__json_fprintf( f, "\n" );
			}
		}
#else
		if ( ! is_last )
		{
			zpl__json_fprintf( f, ",\n" );
		}
		else
		{
			zpl__json_fprintf( f, "\n" );
		}
#endif
	}

	return true;
}

#undef zpl__json_fprintf
#undef zpl___ind
#undef zpl__json_append_node

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
