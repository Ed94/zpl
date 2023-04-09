// file: header/parsers/json.h


ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

typedef enum json_error
{
	ZPL_JSON_ERROR_NONE,
	ZPL_JSON_ERROR_INTERNAL,
	ZPL_JSON_ERROR_INVALID_NAME,
	ZPL_JSON_ERROR_INVALID_VALUE,
	ZPL_JSON_ERROR_INVALID_ASSIGNMENT,
	ZPL_JSON_ERROR_UNKNOWN_KEYWORD,
	ZPL_JSON_ERROR_ARRAY_LEFT_OPEN,
	ZPL_JSON_ERROR_OBJECT_END_PAIR_MISMATCHED,
	ZPL_JSON_ERROR_OUT_OF_MEMORY,
} json_error;

typedef adt_node json_object;

ZPL_DEF u8     json_parse( json_object* root, char* text, AllocatorInfo allocator );
ZPL_DEF void   json_free( json_object* obj );
ZPL_DEF b8     json_write( FileInfo* file, json_object* obj, sw indent );
ZPL_DEF String json_write_string( AllocatorInfo a, json_object* obj, sw indent );

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
