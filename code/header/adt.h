// a_file: header/adt.h

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

typedef enum adt_type
{
	ZPL_ADT_TYPE_UNINITIALISED, /* node was not initialised, this is a programming error! */
	ZPL_ADT_TYPE_ARRAY,
	ZPL_ADT_TYPE_OBJECT,
	ZPL_ADT_TYPE_STRING,
	ZPL_ADT_TYPE_MULTISTRING,
	ZPL_ADT_TYPE_INTEGER,
	ZPL_ADT_TYPE_REAL,
} adt_type;

typedef enum adt_props
{
	ZPL_ADT_PROPS_NONE,
	ZPL_ADT_PROPS_NAN,
	ZPL_ADT_PROPS_NAN_NEG,
	ZPL_ADT_PROPS_INFINITY,
	ZPL_ADT_PROPS_INFINITY_NEG,
	ZPL_ADT_PROPS_FALSE,
	ZPL_ADT_PROPS_TRUE,
	ZPL_ADT_PROPS_NULL,
	ZPL_ADT_PROPS_IS_EXP,
	ZPL_ADT_PROPS_IS_HEX,

	// Used internally so that people can fill in real numbers they plan to write.
	ZPL_ADT_PROPS_IS_PARSED_REAL,
} adt_props;

typedef enum adt_naming_style
{
	ZPL_ADT_NAME_STYLE_DOUBLE_QUOTE,
	ZPL_ADT_NAME_STYLE_SINGLE_QUOTE,
	ZPL_ADT_NAME_STYLE_NO_QUOTES,
} adt_naming_style;

typedef enum adt_assign_style
{
	ZPL_ADT_ASSIGN_STYLE_COLON,
	ZPL_ADT_ASSIGN_STYLE_EQUALS,
	ZPL_ADT_ASSIGN_STYLE_LINE,
} adt_assign_style;

typedef enum adt_delim_style
{
	ZPL_ADT_DELIM_STYLE_COMMA,
	ZPL_ADT_DELIM_STYLE_LINE,
	ZPL_ADT_DELIM_STYLE_NEWLINE,
} adt_delim_style;

typedef enum adt_error
{
	ZPL_ADT_ERROR_NONE,
	ZPL_ADT_ERROR_INTERNAL,
	ZPL_ADT_ERROR_ALREADY_CONVERTED,
	ZPL_ADT_ERROR_INVALID_TYPE,
	ZPL_ADT_ERROR_OUT_OF_MEMORY,
} adt_error;

typedef struct adt_node
{
	char const*      name;
	struct adt_node* parent;

	/* properties */
	u8 type  : 4;
	u8 props : 4;
#ifndef ZPL_PARSER_DISABLE_ANALYSIS
	u8 cfg_mode          : 1;
	u8 name_style        : 2;
	u8 assign_style      : 2;
	u8 delim_style       : 2;
	u8 delim_line_width  : 4;
	u8 assign_line_width : 4;
#endif

	/* adt data */
	union
	{
		char const*      string;
		struct adt_node* nodes;    ///< zpl_array

		struct
		{
			union
			{
				f64 real;
				s64 integer;
			};

#ifndef ZPL_PARSER_DISABLE_ANALYSIS
			/* number analysis */
			s32 base;
			s32 base2;
			u8  base2_offset : 4;
			s8  exp          : 4;
			u8  neg_zero     : 1;
			u8  lead_digit   : 1;
#endif
		};
	};
} adt_node;

/* ADT NODE LIMITS
 * delimiter and assignment segment width is limited to 128 whitespace symbols each.
 * real number limits decimal position to 128 places.
 * real number exponent is limited to 64 digits.
 */

/**
 * @brief Initialise an ADT object or array
 *
 * @param node
 * @param backing Memory allocator used for descendants
 * @param name Node's name
 * @param is_array
 * @return error code
 */
ZPL_DEF u8 adt_make_branch( adt_node* node, allocator backing, char const* name, b32 is_array );

/**
 * @brief Destroy an ADT branch and its descendants
 *
 * @param node
 * @return error code
 */
ZPL_DEF u8 adt_destroy_branch( adt_node* node );

/**
 * @brief Initialise an ADT leaf
 *
 * @param node
 * @param name Node's name
 * @param type Node's type (use adt_make_branch for container nodes)
 * @return error code
 */
ZPL_DEF u8 adt_make_leaf( adt_node* node, char const* name, u8 type );


/**
 * @brief Fetch a node using provided URI string.
 *
 * This method uses a basic syntax to fetch a node from the ADT. The following features are available
 * to retrieve the data:
 *
 * - "a/b/c" navigates through objects "a" and "b" to get to "c"
 * - "arr/[foo=123]/bar" iterates over "arr" to find any object with param "foo" that matches the value "123", then gets its field called "bar"
 * - "arr/3" retrieves the 4th element in "arr"
 * - "arr/[apple]" retrieves the first element of value "apple" in "arr"
 *
 * @param node ADT node
 * @param uri Locator string as described above
 * @return adt_node*
 *
 * @see code/apps/examples/json_get.c
 */
ZPL_DEF adt_node* adt_query( adt_node* node, char const* uri );

/**
 * @brief Find a field node within an object by the given name.
 *
 * @param node
 * @param name
 * @param deep_search Perform search recursively
 * @return adt_node * node
 */
ZPL_DEF adt_node* adt_find( adt_node* node, char const* name, b32 deep_search );

/**
 * @brief Allocate an unitialised node within a container at a specified index.
 *
 * @param parent
 * @param index
 * @return adt_node * node
 */
ZPL_DEF adt_node* adt_alloc_at( adt_node* parent, sw index );

/**
 * @brief Allocate an unitialised node within a container.
 *
 * @param parent
 * @return adt_node * node
 */
ZPL_DEF adt_node* adt_alloc( adt_node* parent );

/**
 * @brief Move an existing node to a new container at a specified index.
 *
 * @param node
 * @param new_parent
 * @param index
 * @return adt_node * node
 */
ZPL_DEF adt_node* adt_move_node_at( adt_node* node, adt_node* new_parent, sw index );

/**
 * @brief Move an existing node to a new container.
 *
 * @param node
 * @param new_parent
 * @return adt_node * node
 */
ZPL_DEF adt_node* adt_move_node( adt_node* node, adt_node* new_parent );

/**
 * @brief Swap two nodes.
 *
 * @param node
 * @param other_node
 * @return
 */
ZPL_DEF void adt_swap_nodes( adt_node* node, adt_node* other_node );

/**
 * @brief Remove node from container.
 *
 * @param node
 * @return
 */
ZPL_DEF void adt_remove_node( adt_node* node );

/**
 * @brief Initialise a node as an object
 *
 * @param obj
 * @param name
 * @param backing
 * @return
 */
ZPL_DEF b8 adt_set_obj( adt_node* obj, char const* name, allocator backing );

/**
 * @brief Initialise a node as an array
 *
 * @param obj
 * @param name
 * @param backing
 * @return
 */
ZPL_DEF b8 adt_set_arr( adt_node* obj, char const* name, allocator backing );

/**
 * @brief Initialise a node as a string
 *
 * @param obj
 * @param name
 * @param value
 * @return
 */
ZPL_DEF b8 adt_set_str( adt_node* obj, char const* name, char const* value );

/**
 * @brief Initialise a node as a float
 *
 * @param obj
 * @param name
 * @param value
 * @return
 */
ZPL_DEF b8 adt_set_flt( adt_node* obj, char const* name, f64 value );

/**
 * @brief Initialise a node as a signed integer
 *
 * @param obj
 * @param name
 * @param value
 * @return
 */
ZPL_DEF b8 adt_set_int( adt_node* obj, char const* name, s64 value );

/**
 * @brief Append a new node to a container as an object
 *
 * @param parent
 * @param name
 * @return*
 */
ZPL_DEF adt_node* adt_append_obj( adt_node* parent, char const* name );

/**
 * @brief Append a new node to a container as an array
 *
 * @param parent
 * @param name
 * @return*
 */
ZPL_DEF adt_node* adt_append_arr( adt_node* parent, char const* name );

/**
 * @brief Append a new node to a container as a string
 *
 * @param parent
 * @param name
 * @param value
 * @return*
 */
ZPL_DEF adt_node* adt_append_str( adt_node* parent, char const* name, char const* value );

/**
 * @brief Append a new node to a container as a float
 *
 * @param parent
 * @param name
 * @param value
 * @return*
 */
ZPL_DEF adt_node* adt_append_flt( adt_node* parent, char const* name, f64 value );

/**
 * @brief Append a new node to a container as a signed integer
 *
 * @param parent
 * @param name
 * @param value
 * @return*
 */
ZPL_DEF adt_node* adt_append_int( adt_node* parent, char const* name, s64 value );

/* parser helpers */

/**
 * @brief Parses a text and stores the result into an unitialised node.
 *
 * @param node
 * @param base
 * @return*
 */
ZPL_DEF char* adt_parse_number( adt_node* node, char* base );

/**
 * @brief Parses and converts an existing string node into a number.
 *
 * @param node
 * @return
 */
ZPL_DEF adt_error adt_str_to_number( adt_node* node );

/**
 * @brief Prints a number into a a_file stream.
 *
 * The provided a_file handle can also be a memory mapped stream.
 *
 * @see file_stream_new
 * @param a_file
 * @param node
 * @return
 */
ZPL_DEF adt_error adt_print_number( file* a_file, adt_node* node );

/**
 * @brief Prints a string into a a_file stream.
 *
 * The provided a_file handle can also be a memory mapped stream.
 *
 * @see file_stream_new
 * @param a_file
 * @param node
 * @param escaped_chars
 * @param escape_symbol
 * @return
 */
ZPL_DEF adt_error adt_print_string( file* a_file, adt_node* node, char const* escaped_chars, char const* escape_symbol );

/* extensions */

#if defined( __STDC_VERSION__ ) && __STDC_VERSION__ >= 201112L
#	define adt_append( parent, name, value )                                                                                                                                      \
		_Generic( ( value ), char* : adt_append_str, char const* : adt_append_str, f64 : adt_append_flt, default : adt_append_int )( parent, name, value )
#	define adt_set( obj, name, value ) _Generic( ( value ), char* : adt_set_str, char const* : adt_set_str, f64 : adt_set_flt, default : adt_set_int )( obj, name, value )
#endif

/* deprecated */

ZPL_DEPRECATED_FOR( 18.0.0, adt_query )

ZPL_IMPL_INLINE adt_node* adt_get( adt_node* node, char const* uri )
{
	return adt_query( node, uri );
}

ZPL_DEPRECATED_FOR( 13.3.0, adt_str_to_number )

ZPL_IMPL_INLINE void adt_str_to_flt( adt_node* node )
{
	( void )adt_str_to_number( node );
}

ZPL_DEPRECATED_FOR( 17.0.0, adt_append_obj )

ZPL_IMPL_INLINE adt_node* adt_inset_obj( adt_node* parent, char const* name )
{
	return adt_append_obj( parent, name );
}

ZPL_DEPRECATED_FOR( 17.0.0, adt_append_arr )

ZPL_IMPL_INLINE adt_node* adt_inset_arr( adt_node* parent, char const* name )
{
	return adt_append_arr( parent, name );
}

ZPL_DEPRECATED_FOR( 17.0.0, adt_append_str )

ZPL_IMPL_INLINE adt_node* adt_inset_str( adt_node* parent, char const* name, char const* value )
{
	return adt_append_str( parent, name, value );
}

ZPL_DEPRECATED_FOR( 17.0.0, adt_append_flt )

ZPL_IMPL_INLINE adt_node* adt_inset_flt( adt_node* parent, char const* name, f64 value )
{
	return adt_append_flt( parent, name, value );
}

ZPL_DEPRECATED_FOR( 17.0.0, adt_append_int )

ZPL_IMPL_INLINE adt_node* adt_inset_int( adt_node* parent, char const* name, s64 value )
{
	return adt_append_int( parent, name, value );
}

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
