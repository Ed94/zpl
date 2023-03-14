// a_file: header/opts.h

/** @a_file opts.c
@brief CLI options processor
@defgroup cli CLI options processor

 Opts is a CLI options parser, it can parse flags, switches and arguments from command line
 and offers an easy way to express input errors as well as the ability to display help screen.

@{
 */

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

typedef enum
{
	ZPL_OPTS_STRING,
	ZPL_OPTS_FLOAT,
	ZPL_OPTS_FLAG,
	ZPL_OPTS_INT,
} opts_types;

typedef struct
{
	char const *name, *lname, *desc;
	u8          type;
	b32         met, pos;

	//! values
	union
	{
		string text;
		s64    integer;
		f64    real;
	};
} opts_entry;

typedef enum
{
	ZPL_OPTS_ERR_VALUE,
	ZPL_OPTS_ERR_OPTION,
	ZPL_OPTS_ERR_EXTRA_VALUE,
	ZPL_OPTS_ERR_MISSING_VALUE,
} opts_err_type;

typedef struct
{
	char* val;
	u8    type;
} opts_err;

typedef struct
{
	allocator    a_allocator;
	opts_entry*  entries;       ///< zpl_array
	opts_err*    errors;        ///< zpl_array
	opts_entry** positioned;    ///< zpl_array
	char const*  appname;
} opts;

//! Initializes options parser.

//! Initializes CLI options parser using specified memory allocator and provided application name.
//! @param a_opts Options parser to initialize.
//! @param allocator Memory allocator to use. (ex. heap())
//! @param app Application name displayed in help screen.
ZPL_DEF void opts_init( opts* a_opts, allocator allocator, char const* app );

//! Releases the resources used by options parser.
ZPL_DEF void opts_free( opts* a_opts );

//! Registers an option.

//! Registers an option with its short and long name, specifies option's type and its description.
//! @param a_opts Options parser to add to.
//! @param lname Shorter name of option. (ex. "f")
//! @param name Full name of option. (ex. "foo") Note that rest of the module uses longer names to manipulate a_opts.
//! @param desc Description shown in the help screen.
//! @param type Option's type (see opts_types)
//! @see opts_types
ZPL_DEF void opts_add( opts* a_opts, char const* name, char const* lname, const char* desc, u8 type );

//! Registers option as positional.

//! Registers added option as positional, so that we can pass it anonymously. Arguments are expected on the command input in the same order they were registered as.
//! @param a_opts
//! @param name Name of already registered option.
ZPL_DEF void opts_positional_add( opts* a_opts, char const* name );

//! Compiles CLI arguments.

// This method takes CLI arguments as input and processes them based on rules that were set up.
//! @param a_opts
//! @param argc Argument count in an array.
//! @param argv Array of arguments.
ZPL_DEF b32 opts_compile( opts* a_opts, int argc, char** argv );

//! Prints out help screen.

//! Prints out help screen with example usage of application as well as with all the flags available.
ZPL_DEF void opts_print_help( opts* a_opts );

//! Prints out parsing errors.

//! Prints out possible errors caused by CLI input.
ZPL_DEF void opts_print_errors( opts* a_opts );

//! Fetches a string from an option.

//! @param a_opts
//! @param name Name of an option.
//! @param fallback Fallback string we return if option wasn't found.
ZPL_DEF string opts_string( opts* a_opts, char const* name, char const* fallback );

//! Fetches a real number from an option.

//! @param a_opts
//! @param name Name of an option.
//! @param fallback Fallback real number we return if option was not found.
ZPL_DEF f64 opts_real( opts* a_opts, char const* name, f64 fallback );

//! Fetches an integer number from an option.

//! @param a_opts
//! @param name Name of an option.
//! @param fallback Fallback integer number we return if option was not found.
ZPL_DEF s64 opts_integer( opts* a_opts, char const* name, s64 fallback );

//! Checks whether an option was used.

//! @param a_opts
//! @param name Name of an option.
ZPL_DEF b32 opts_has_arg( opts* a_opts, char const* name );

//! Checks whether all positionals have been passed in.
ZPL_DEF b32 opts_positionals_filled( opts* a_opts );

//! @}

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
