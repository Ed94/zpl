// file: header/parsers/csv.h

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

typedef enum csv_error
{
	ZPL_CSV_ERROR_NONE,
	ZPL_CSV_ERROR_INTERNAL,
	ZPL_CSV_ERROR_UNEXPECTED_END_OF_INPUT,
	ZPL_CSV_ERROR_MISMATCHED_ROWS,
} csv_error;

typedef adt_node csv_object;

ZPL_DEF_INLINE u8 csv_parse( csv_object* root, char* text, zpl_allocator allocator, b32 has_header );
ZPL_DEF u8        csv_parse_delimiter( csv_object* root, char* text, zpl_allocator allocator, b32 has_header, char delim );
ZPL_DEF void      csv_free( csv_object* obj );

ZPL_DEF_INLINE void   csv_write( zpl_file* file, csv_object* obj );
ZPL_DEF_INLINE string csv_write_string( zpl_allocator a, csv_object* obj );
ZPL_DEF void          csv_write_delimiter( zpl_file* file, csv_object* obj, char delim );
ZPL_DEF string        csv_write_string_delimiter( zpl_allocator a, csv_object* obj, char delim );

/* inline */

ZPL_IMPL_INLINE u8 csv_parse( csv_object* root, char* text, zpl_allocator allocator, b32 has_header )
{
	return csv_parse_delimiter( root, text, allocator, has_header, ',' );
}

ZPL_IMPL_INLINE void csv_write( zpl_file* file, csv_object* obj )
{
	csv_write_delimiter( file, obj, ',' );
}

ZPL_IMPL_INLINE string csv_write_string( zpl_allocator a, csv_object* obj )
{
	return csv_write_string_delimiter( a, obj, ',' );
}

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
