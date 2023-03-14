// a_file: source/core/print.c


ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

sw printf_va( char const* fmt, va_list va )
{
	return fprintf_va( file_get_standard( ZPL_FILE_STANDARD_OUTPUT ), fmt, va );
}

sw printf_err_va( char const* fmt, va_list va )
{
	return fprintf_va( file_get_standard( ZPL_FILE_STANDARD_ERROR ), fmt, va );
}

sw fprintf_va( struct file* f, char const* fmt, va_list va )
{
	local_persist thread_local char buf[ ZPL_PRINTF_MAXLEN ];
	sw                              len = snprintf_va( buf, size_of( buf ), fmt, va );
	b32                             res = file_write( f, buf, len - 1 );    // NOTE: prevent extra whitespace
	return res ? len : -1;
}

char* bprintf_va( char const* fmt, va_list va )
{
	local_persist thread_local char buffer[ ZPL_PRINTF_MAXLEN ];
	snprintf_va( buffer, size_of( buffer ), fmt, va );
	return buffer;
}

sw asprintf_va( allocator allocator, char** buffer, char const* fmt, va_list va )
{
	local_persist thread_local char tmp[ ZPL_PRINTF_MAXLEN ];
	ZPL_ASSERT_NOT_NULL( buffer );
	sw res;
	res     = snprintf_va( tmp, size_of( tmp ), fmt, va );
	*buffer = alloc_str( allocator, tmp );
	return res;
}

sw printf( char const* fmt, ... )
{
	sw      res;
	va_list va;
	va_start( va, fmt );
	res = printf_va( fmt, va );
	va_end( va );
	return res;
}

sw printf_err( char const* fmt, ... )
{
	sw      res;
	va_list va;
	va_start( va, fmt );
	res = printf_err_va( fmt, va );
	va_end( va );
	return res;
}

sw fprintf( struct file* f, char const* fmt, ... )
{
	sw      res;
	va_list va;
	va_start( va, fmt );
	res = fprintf_va( f, fmt, va );
	va_end( va );
	return res;
}

char* bprintf( char const* fmt, ... )
{
	va_list va;
	char*   str;
	va_start( va, fmt );
	str = bprintf_va( fmt, va );
	va_end( va );
	return str;
}

sw asprintf( allocator allocator, char** buffer, char const* fmt, ... )
{
	sw      res;
	va_list va;
	va_start( va, fmt );
	res = asprintf_va( allocator, buffer, fmt, va );
	va_end( va );
	return res;
}

sw snprintf( char* str, sw n, char const* fmt, ... )
{
	sw      res;
	va_list va;
	va_start( va, fmt );
	res = snprintf_va( str, n, fmt, va );
	va_end( va );
	return res;
}

enum
{
	ZPL_FMT_MINUS = ZPL_BIT( 0 ),
	ZPL_FMT_PLUS  = ZPL_BIT( 1 ),
	ZPL_FMT_ALT   = ZPL_BIT( 2 ),
	ZPL_FMT_SPACE = ZPL_BIT( 3 ),
	ZPL_FMT_ZERO  = ZPL_BIT( 4 ),

	ZPL_FMT_CHAR   = ZPL_BIT( 5 ),
	ZPL_FMT_SHORT  = ZPL_BIT( 6 ),
	ZPL_FMT_INT    = ZPL_BIT( 7 ),
	ZPL_FMT_LONG   = ZPL_BIT( 8 ),
	ZPL_FMT_LLONG  = ZPL_BIT( 9 ),
	ZPL_FMT_SIZE   = ZPL_BIT( 10 ),
	ZPL_FMT_INTPTR = ZPL_BIT( 11 ),

	ZPL_FMT_UNSIGNED = ZPL_BIT( 12 ),
	ZPL_FMT_LOWER    = ZPL_BIT( 13 ),
	ZPL_FMT_UPPER    = ZPL_BIT( 14 ),
	ZPL_FMT_WIDTH    = ZPL_BIT( 15 ),

	ZPL_FMT_DONE = ZPL_BIT( 30 ),

	ZPL_FMT_INTS = ZPL_FMT_CHAR | ZPL_FMT_SHORT | ZPL_FMT_INT | ZPL_FMT_LONG | ZPL_FMT_LLONG | ZPL_FMT_SIZE | ZPL_FMT_INTPTR
};

typedef struct
{
	s32 base;
	s32 flags;
	s32 width;
	s32 precision;
} zpl__format_info;

internal sw zpl__print_string( char* text, sw max_len, zpl__format_info* info, char const* str )
{
	sw    res = 0, len = 0;
	sw    remaining = max_len;
	char* begin     = text;

	if ( str == NULL && max_len >= 6 )
	{
		res += strlcpy( text, "(null)", 6 );
		return res;
	}

	if ( info && info->precision >= 0 )
		len = strnlen( str, info->precision );
	else
		len = strlen( str );

	if ( info && ( info->width == 0 && info->flags & ZPL_FMT_WIDTH ) )
	{
		return res;
	}

	if ( info && ( info->width == 0 || info->flags & ZPL_FMT_MINUS ) )
	{
		if ( info->precision > 0 )
			len = info->precision < len ? info->precision : len;
		if ( res + len > max_len )
			return res;
		res  += strlcpy( text, str, len );
		text += res;

		if ( info->width > res )
		{
			sw padding = info->width - len;

			char pad = ( info->flags & ZPL_FMT_ZERO ) ? '0' : ' ';
			while ( padding-- > 0 && remaining-- > 0 )
				*text++ = pad, res++;
		}
	}
	else
	{
		if ( info && ( info->width > res ) )
		{
			sw   padding = info->width - len;
			char pad     = ( info->flags & ZPL_FMT_ZERO ) ? '0' : ' ';
			while ( padding-- > 0 && remaining-- > 0 )
				*text++ = pad, res++;
		}

		if ( res + len > max_len )
			return res;
		res += strlcpy( text, str, len );
	}

	if ( info )
	{
		if ( info->flags & ZPL_FMT_UPPER )
			str_to_upper( begin );
		else if ( info->flags & ZPL_FMT_LOWER )
			str_to_lower( begin );
	}

	return res;
}

internal sw zpl__print_char( char* text, sw max_len, zpl__format_info* info, char arg )
{
	char str[ 2 ] = "";
	str[ 0 ]      = arg;
	return zpl__print_string( text, max_len, info, str );
}

internal sw zpl__print_repeated_char( char* text, sw max_len, zpl__format_info* info, char arg )
{
	sw  res = 0;
	s32 rem = ( info ) ? ( info->width > 0 ) ? info->width : 1 : 1;
	res     = rem;
	while ( rem-- > 0 )
		*text++ = arg;

	return res;
}

internal sw zpl__print_i64( char* text, sw max_len, zpl__format_info* info, s64 value )
{
	char num[ 130 ];
	i64_to_str( value, num, info ? info->base : 10 );
	return zpl__print_string( text, max_len, info, num );
}

internal sw zpl__print_u64( char* text, sw max_len, zpl__format_info* info, u64 value )
{
	char num[ 130 ];
	u64_to_str( value, num, info ? info->base : 10 );
	return zpl__print_string( text, max_len, info, num );
}

internal sw zpl__print_f64( char* text, sw max_len, zpl__format_info* info, b32 is_hexadecimal, f64 arg )
{
	// TODO: Handle exponent notation
	sw    width, len, remaining = max_len;
	char* text_begin = text;

	if ( arg )
	{
		u64 value;
		if ( arg < 0 )
		{
			if ( remaining > 1 )
				*text = '-', remaining--;
			text++;
			arg = -arg;
		}
		else if ( info->flags & ZPL_FMT_MINUS )
		{
			if ( remaining > 1 )
				*text = '+', remaining--;
			text++;
		}

		value = zpl_cast( u64 ) arg;
		len   = zpl__print_u64( text, remaining, NULL, value );
		text  += len;

		if ( len >= remaining )
			remaining = min( remaining, 1 );
		else
			remaining -= len;
		arg -= value;

		if ( info->precision < 0 )
			info->precision = 6;

		if ( ( info->flags & ZPL_FMT_ALT ) || info->precision > 0 )
		{
			s64 mult = 10;
			if ( remaining > 1 )
				*text = '.', remaining--;
			text++;
			while ( info->precision-- > 0 )
			{
				value = zpl_cast( u64 )( arg * mult );
				len   = zpl__print_u64( text, remaining, NULL, value );
				text  += len;
				if ( len >= remaining )
					remaining = min( remaining, 1 );
				else
					remaining -= len;
				arg  -= zpl_cast( f64 ) value / mult;
				mult *= 10;
			}
		}
	}
	else
	{
		if ( remaining > 1 )
			*text = '0', remaining--;
		text++;
		if ( info->flags & ZPL_FMT_ALT )
		{
			if ( remaining > 1 )
				*text = '.', remaining--;
			text++;
		}
	}

	width = info->width - ( text - text_begin );
	if ( width > 0 )
	{
		char  fill = ( info->flags & ZPL_FMT_ZERO ) ? '0' : ' ';
		char* end  = text + remaining - 1;
		len        = ( text - text_begin );

		for ( len = ( text - text_begin ); len--; )
		{
			if ( ( text_begin + len + width ) < end )
				*( text_begin + len + width ) = *( text_begin + len );
		}

		len  = width;
		text += len;
		if ( len >= remaining )
			remaining = min( remaining, 1 );
		else
			remaining -= len;

		while ( len-- )
		{
			if ( text_begin + len < end )
				text_begin[ len ] = fill;
		}
	}

	return ( text - text_begin );
}

ZPL_NEVER_INLINE sw snprintf_va( char* text, sw max_len, char const* fmt, va_list va )
{
	char const* text_begin = text;
	sw          remaining  = max_len, res;

	while ( *fmt )
	{
		zpl__format_info info = { 0 };
		sw               len  = 0;
		info.precision        = -1;

		while ( *fmt && *fmt != '%' && remaining )
			*text++ = *fmt++;

		if ( *fmt == '%' )
		{
			do
			{
				switch ( *++fmt )
				{
					case '-' :
						{
							info.flags |= ZPL_FMT_MINUS;
							break;
						}
					case '+' :
						{
							info.flags |= ZPL_FMT_PLUS;
							break;
						}
					case '#' :
						{
							info.flags |= ZPL_FMT_ALT;
							break;
						}
					case ' ' :
						{
							info.flags |= ZPL_FMT_SPACE;
							break;
						}
					case '0' :
						{
							info.flags |= ( ZPL_FMT_ZERO | ZPL_FMT_WIDTH );
							break;
						}
					default :
						{
							info.flags |= ZPL_FMT_DONE;
							break;
						}
				}
			} while ( ! ( info.flags & ZPL_FMT_DONE ) );
		}

		// NOTE: Optional Width
		if ( *fmt == '*' )
		{
			int width = va_arg( va, int );
			if ( width < 0 )
			{
				info.flags |= ZPL_FMT_MINUS;
				info.width = -width;
			}
			else
			{
				info.width = width;
			}
			info.flags |= ZPL_FMT_WIDTH;
			fmt++;
		}
		else
		{
			info.width = zpl_cast( s32 ) str_to_i64( fmt, zpl_cast( char** ) & fmt, 10 );
			if ( info.width != 0 )
			{
				info.flags |= ZPL_FMT_WIDTH;
			}
		}

		// NOTE: Optional Precision
		if ( *fmt == '.' )
		{
			fmt++;
			if ( *fmt == '*' )
			{
				info.precision = va_arg( va, int );
				fmt++;
			}
			else
			{
				info.precision = zpl_cast( s32 ) str_to_i64( fmt, zpl_cast( char** ) & fmt, 10 );
			}
			info.flags &= ~ZPL_FMT_ZERO;
		}

		switch ( *fmt++ )
		{
			case 'h' :
				if ( *fmt == 'h' )
				{    // hh => char
					info.flags |= ZPL_FMT_CHAR;
					fmt++;
				}
				else
				{    // h => short
					info.flags |= ZPL_FMT_SHORT;
				}
				break;

			case 'l' :
				if ( *fmt == 'l' )
				{    // ll => long long
					info.flags |= ZPL_FMT_LLONG;
					fmt++;
				}
				else
				{    // l => long
					info.flags |= ZPL_FMT_LONG;
				}
				break;

				break;

			case 'z' :    // NOTE: uw
				info.flags |= ZPL_FMT_UNSIGNED;
				// fallthrough
			case 't' :    // NOTE: sw
				info.flags |= ZPL_FMT_SIZE;
				break;

			default :
				fmt--;
				break;
		}

		switch ( *fmt )
		{
			case 'u' :
				info.flags |= ZPL_FMT_UNSIGNED;
				// fallthrough
			case 'd' :
			case 'i' :
				info.base = 10;
				break;

			case 'o' :
				info.base = 8;
				break;

			case 'x' :
				info.base  = 16;
				info.flags |= ( ZPL_FMT_UNSIGNED | ZPL_FMT_LOWER );
				break;

			case 'X' :
				info.base  = 16;
				info.flags |= ( ZPL_FMT_UNSIGNED | ZPL_FMT_UPPER );
				break;

			case 'f' :
			case 'F' :
			case 'g' :
			case 'G' :
				len = zpl__print_f64( text, remaining, &info, 0, va_arg( va, f64 ) );
				break;

			case 'a' :
			case 'A' :
				len = zpl__print_f64( text, remaining, &info, 1, va_arg( va, f64 ) );
				break;

			case 'c' :
				len = zpl__print_char( text, remaining, &info, zpl_cast( char ) va_arg( va, int ) );
				break;

			case 's' :
				len = zpl__print_string( text, remaining, &info, va_arg( va, char* ) );
				break;

			case 'r' :
				len = zpl__print_repeated_char( text, remaining, &info, va_arg( va, int ) );
				break;

			case 'p' :
				info.base  = 16;
				info.flags |= ( ZPL_FMT_LOWER | ZPL_FMT_UNSIGNED | ZPL_FMT_ALT | ZPL_FMT_INTPTR );
				break;

			case '%' :
				len = zpl__print_char( text, remaining, &info, '%' );
				break;

			default :
				fmt--;
				break;
		}

		fmt++;

		if ( info.base != 0 )
		{
			if ( info.flags & ZPL_FMT_UNSIGNED )
			{
				u64 value = 0;
				switch ( info.flags & ZPL_FMT_INTS )
				{
					case ZPL_FMT_CHAR :
						value = zpl_cast( u64 ) zpl_cast( u8 ) va_arg( va, int );
						break;
					case ZPL_FMT_SHORT :
						value = zpl_cast( u64 ) zpl_cast( u16 ) va_arg( va, int );
						break;
					case ZPL_FMT_LONG :
						value = zpl_cast( u64 ) va_arg( va, unsigned long );
						break;
					case ZPL_FMT_LLONG :
						value = zpl_cast( u64 ) va_arg( va, unsigned long long );
						break;
					case ZPL_FMT_SIZE :
						value = zpl_cast( u64 ) va_arg( va, uw );
						break;
					case ZPL_FMT_INTPTR :
						value = zpl_cast( u64 ) va_arg( va, uptr );
						break;
					default :
						value = zpl_cast( u64 ) va_arg( va, unsigned int );
						break;
				}

				len = zpl__print_u64( text, remaining, &info, value );
			}
			else
			{
				s64 value = 0;
				switch ( info.flags & ZPL_FMT_INTS )
				{
					case ZPL_FMT_CHAR :
						value = zpl_cast( s64 ) zpl_cast( s8 ) va_arg( va, int );
						break;
					case ZPL_FMT_SHORT :
						value = zpl_cast( s64 ) zpl_cast( s16 ) va_arg( va, int );
						break;
					case ZPL_FMT_LONG :
						value = zpl_cast( s64 ) va_arg( va, long );
						break;
					case ZPL_FMT_LLONG :
						value = zpl_cast( s64 ) va_arg( va, long long );
						break;
					case ZPL_FMT_SIZE :
						value = zpl_cast( s64 ) va_arg( va, uw );
						break;
					case ZPL_FMT_INTPTR :
						value = zpl_cast( s64 ) va_arg( va, uptr );
						break;
					default :
						value = zpl_cast( s64 ) va_arg( va, int );
						break;
				}

				len = zpl__print_i64( text, remaining, &info, value );
			}
		}

		text += len;
		if ( len >= remaining )
			remaining = min( remaining, 1 );
		else
			remaining -= len;
	}

	*text++ = '\0';
	res     = ( text - text_begin );
	return ( res >= max_len || res < 0 ) ? -1 : res;
}

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
