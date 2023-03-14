// a_file: header/regex.h

/** @a_file regex.c
@brief Regular expressions parser.
@defgroup regex Regex processor

Port of gb_regex with several bugfixes applied. This is a simple regex library and is fast to perform.

Supported Matching:
    @n ^       - Beginning of string
    @n $       - End of string
    @n .       - Match one (anything)
    @n |       - Branch (or)
    @n ()      - Capturing group
    @n []      - Any character included in set
    @n [^]     - Any character excluded from set
    @n +       - One or more  (greedy)
    @n +?      - One or more  (non-greedy)
    @n *       - Zero or more (greedy)
    @n *?      - Zero or more (non-greedy)
    @n ?       - Zero or once
    @n [BACKSLASH]XX     - Hex decimal digit (must be 2 digits)
    @n [BACKSLASH]meta   - Meta character
    @n [BACKSLASH]s      - Whitespace
    @n [BACKSLASH]S      - Not whitespace
    @n [BACKSLASH]d      - Digit
    @n [BACKSLASH]D      - Not digit
    @n [BACKSLASH]a      - Alphabetic character
    @n [BACKSLASH]l      - Lower case letter
    @n [BACKSLASH]u      - Upper case letter
    @n [BACKSLASH]w      - Word
    @n [BACKSLASH]W      - Not word
    @n [BACKSLASH]x      - Hex Digit
    @n [BACKSLASH]p      - Printable ASCII character
    @n --Whitespace--
    @n [BACKSLASH]t      - Tab
    @n [BACKSLASH]n      - New line
    @n [BACKSLASH]r      - Return carriage
    @n [BACKSLASH]v      - Vertical Tab
    @n [BACKSLASH]f      - Form feed

    @{
*/

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

typedef struct re
{
	allocator backing;
	sw        capture_count;
	char*     buf;
	sw        buf_len, buf_cap;
	b32       can_realloc;
} re;

typedef struct re_capture
{
	char const* str;
	sw          len;
} re_capture;

#define zplRegexError ZPL_NS regex_error

typedef enum regex_error
{
	ZPL_RE_ERROR_NONE,
	ZPL_RE_ERROR_NO_MATCH,
	ZPL_RE_ERROR_TOO_LONG,
	ZPL_RE_ERROR_MISMATCHED_CAPTURES,
	ZPL_RE_ERROR_MISMATCHED_BLOCKS,
	ZPL_RE_ERROR_BRANCH_FAILURE,
	ZPL_RE_ERROR_INVALID_QUANTIFIER,
	ZPL_RE_ERROR_INTERNAL_FAILURE,
} regex_error;

//! Compile regex pattern.
ZPL_DEF regex_error re_compile( re* re, allocator backing, char const* pattern, sw pattern_len );

//! Compile regex pattern using a buffer.
ZPL_DEF regex_error re_compile_from_buffer( re* re, char const* pattern, sw pattern_len, void* buffer, sw buffer_len );

//! Destroy regex object.
ZPL_DEF void re_destroy( re* re );

//! Retrieve number of retrievable captures.
ZPL_DEF sw re_capture_count( re* re );

//! Match input string and output captures of the occurence.
ZPL_DEF b32 re_match( re* re, char const* str, sw str_len, re_capture* captures, sw max_capture_count, sw* offset );

//! Match all occurences in an input string and output them into captures. Array of captures is allocated on the heap and needs to be freed afterwards.
ZPL_DEF b32 re_match_all( re* re, char const* str, sw str_len, sw max_capture_count, re_capture** out_captures );

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
