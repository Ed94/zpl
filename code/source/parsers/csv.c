// file: source/parsers/csv.c

#ifdef ZPL_CSV_DEBUG
#define ZPL_CSV_ASSERT( msg ) ZPL_PANIC( msg )
#else
#define ZPL_CSV_ASSERT( msg )
#endif

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

u8 csv_parse_delimiter( csv_object* root, char* text, zpl_allocator allocator, b32 has_header, char delim )
{
	csv_error err = ZPL_CSV_ERROR_NONE;
	ZPL_ASSERT_NOT_NULL( root );
	ZPL_ASSERT_NOT_NULL( text );
	zero_item( root );
	adt_make_branch( root, allocator, NULL, has_header ? false : true );
	char *p = text, *b = p, *e = p;
	sw    colc = 0, total_colc = 0;

	do
	{
		char d = 0;
		p      = zpl_cast( char* ) str_trim( p, false );
		if ( *p == 0 )
			break;
		adt_node row_item = { 0 };
		row_item.type     = ZPL_ADT_TYPE_STRING;
#ifndef ZPL_PARSER_DISABLE_ANALYSIS
		row_item.name_style = ZPL_ADT_NAME_STYLE_NO_QUOTES;
#endif

		/* handle string literals */
		if ( *p == '"' )
		{
			p = b = e       = p + 1;
			row_item.string = b;
#ifndef ZPL_PARSER_DISABLE_ANALYSIS
			row_item.name_style = ZPL_ADT_NAME_STYLE_DOUBLE_QUOTE;
#endif
			do
			{
				e = zpl_cast( char* ) str_skip( e, '"' );
				if ( *e && *( e + 1 ) == '"' )
				{
					e += 2;
				}
				else
					break;
			} while ( *e );
			if ( *e == 0 )
			{
				ZPL_CSV_ASSERT( "unmatched quoted string" );
				err = ZPL_CSV_ERROR_UNEXPECTED_END_OF_INPUT;
				return err;
			}
			*e = 0;
			p  = zpl_cast( char* ) str_trim( e + 1, true );
			d  = *p;

			/* unescape escaped quotes (so that unescaped text escapes :) */
			{
				char* ep = b;
				do
				{
					if ( *ep == '"' && *( ep + 1 ) == '"' )
					{
						zpl_memmove( ep, ep + 1, zpl_strlen( ep ) );
					}
					ep++;
				} while ( *ep );
			}
		}
		else if ( *p == delim )
		{
			d               = *p;
			row_item.string = "";
		}
		else if ( *p )
		{
			/* regular data */
			b = e           = p;
			row_item.string = b;
			do
			{
				e++;
			} while ( *e && *e != delim && *e != '\n' );
			if ( *e )
			{
				p = zpl_cast( char* ) str_trim( e, true );
				while ( char_is_space( *( e - 1 ) ) )
				{
					e--;
				}
				d  = *p;
				*e = 0;
			}
			else
			{
				d = 0;
				p = e;
			}

			/* check if number and process if so */
			b32   skip_number = false;
			char* num_p       = b;
			do
			{
				if ( ! char_is_hex_digit( *num_p ) && ( ! strchr( "+-.eExX", *num_p ) ) )
				{
					skip_number = true;
					break;
				}
			} while ( *num_p++ );

			if ( ! skip_number )
			{
				adt_str_to_number( &row_item );
			}
		}

		if ( colc >= array_count( root->nodes ) )
		{
			adt_append_arr( root, NULL );
		}

		array_append( root->nodes[ colc ].nodes, row_item );

		if ( d == delim )
		{
			colc++;
			p++;
		}
		else if ( d == '\n' || d == 0 )
		{
			/* check if number of rows is not mismatched */
			if ( total_colc < colc )
				total_colc = colc;
			else if ( total_colc != colc )
			{
				ZPL_CSV_ASSERT( "mismatched rows" );
				err = ZPL_CSV_ERROR_MISMATCHED_ROWS;
				return err;
			}
			colc = 0;
			if ( d != 0 )
				p++;
		}
	} while ( *p );

	if ( array_count( root->nodes ) == 0 )
	{
		ZPL_CSV_ASSERT( "unexpected end of input. stream is empty." );
		err = ZPL_CSV_ERROR_UNEXPECTED_END_OF_INPUT;
		return err;
	}

	/* consider first row as a header. */
	if ( has_header )
	{
		for ( sw i = 0; i < array_count( root->nodes ); i++ )
		{
			csv_object* col = root->nodes + i;
			csv_object* hdr = col->nodes;
			col->name       = hdr->string;
			array_remove_at( col->nodes, 0 );
		}
	}

	return err;
}
void csv_free( csv_object* obj )
{
	adt_destroy_branch( obj );
}

void zpl__csv_write_record( zpl_file* file, csv_object* node )
{
	switch ( node->type )
	{
		case ZPL_ADT_TYPE_STRING :
			{
#ifndef ZPL_PARSER_DISABLE_ANALYSIS
				switch ( node->name_style )
				{
					case ZPL_ADT_NAME_STYLE_DOUBLE_QUOTE :
						{
							zpl_fprintf( file, "\"" );
							adt_print_string( file, node, "\"", "\"" );
							zpl_fprintf( file, "\"" );
						}
						break;

					case ZPL_ADT_NAME_STYLE_NO_QUOTES :
						{
#endif
							zpl_fprintf( file, "%s", node->string );
#ifndef ZPL_PARSER_DISABLE_ANALYSIS
						}
						break;
				}
#endif
			}
			break;

		case ZPL_ADT_TYPE_REAL :
		case ZPL_ADT_TYPE_INTEGER :
			{
				adt_print_number( file, node );
			}
			break;
	}
}

void zpl__csv_write_header( zpl_file* file, csv_object* header )
{
	csv_object temp = *header;
	temp.string     = temp.name;
	temp.type       = ZPL_ADT_TYPE_STRING;
	zpl__csv_write_record( file, &temp );
}

void csv_write_delimiter( zpl_file* file, csv_object* obj, char delimiter )
{
	ZPL_ASSERT_NOT_NULL( file );
	ZPL_ASSERT_NOT_NULL( obj );
	ZPL_ASSERT( obj->nodes );
	sw cols = array_count( obj->nodes );
	if ( cols == 0 )
		return;

	sw rows = array_count( obj->nodes[ 0 ].nodes );
	if ( rows == 0 )
		return;

	b32 has_headers = obj->nodes[ 0 ].name != NULL;

	if ( has_headers )
	{
		for ( sw i = 0; i < cols; i++ )
		{
			zpl__csv_write_header( file, &obj->nodes[ i ] );
			if ( i + 1 != cols )
			{
				zpl_fprintf( file, "%c", delimiter );
			}
		}
		zpl_fprintf( file, "\n" );
	}

	for ( sw r = 0; r < rows; r++ )
	{
		for ( sw i = 0; i < cols; i++ )
		{
			zpl__csv_write_record( file, &obj->nodes[ i ].nodes[ r ] );
			if ( i + 1 != cols )
			{
				zpl_fprintf( file, "%c", delimiter );
			}
		}
		zpl_fprintf( file, "\n" );
	}
}

string csv_write_string_delimiter( zpl_allocator a, csv_object* obj, char delimiter )
{
	zpl_file tmp;
	file_stream_new( &tmp, a );
	csv_write_delimiter( &tmp, obj, delimiter );
	sw     fsize;
	u8*    buf    = file_stream_buf( &tmp, &fsize );
	string output = string_make_length( a, (char*)buf, fsize );
	file_close( &tmp );
	return output;
}

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
