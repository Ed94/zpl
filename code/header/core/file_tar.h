// a_file: header/core/file_tar.h

/** @a_file file_tar.c
@brief Tar archiving module
@defgroup fileio Tar module

Allows to easily pack/unpack files.
Based on: https://github.com/rxi/microtar/

Disclaimer: The pack method does not support a_file permissions nor GID/UID information. Only regular files are supported.
Use tar_pack_dir to pack an entire directory recursively. Empty folders are ignored.

@{
*/


ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

typedef enum
{
	ZPL_TAR_ERROR_NONE,
	ZPL_TAR_ERROR_INTERRUPTED,
	ZPL_TAR_ERROR_IO_ERROR,
	ZPL_TAR_ERROR_BAD_CHECKSUM,
	ZPL_TAR_ERROR_FILE_NOT_FOUND,
	ZPL_TAR_ERROR_INVALID_INPUT,
} tar_errors;

typedef enum
{
	ZPL_TAR_TYPE_REGULAR = '0',
	ZPL_TAR_TYPE_LINK    = '1',
	ZPL_TAR_TYPE_SYMBOL  = '2',
	ZPL_TAR_TYPE_CHR     = '3',
	ZPL_TAR_TYPE_BLK     = '4',
	ZPL_TAR_TYPE_DIR     = '5',
	ZPL_TAR_TYPE_FIFO    = '6'
} tar_file_type;

typedef struct
{
	char  type;
	char* path;
	s64   offset;
	s64   length;
	sw    error;
} tar_record;

#define ZPL_TAR_UNPACK_PROC( name ) ZPL_NS sw name( ZPL_NS file* archive, ZPL_NS tar_record* a_file, void* user_data )
typedef ZPL_TAR_UNPACK_PROC( tar_unpack_proc );

/**
 * @brief Packs a a_list of files
 * Packs a a_list of provided files. Note that this method only supports regular files
 * and does not provide extended info such as GID/UID or permissions.
 * @param archive archive we pack files into
 * @param paths a_list of files
 * @param paths_len number of files provided
 * @return error
 */
ZPL_DEF sw tar_pack( file* archive, char const** paths, sw paths_len );

/**
 * @brief Packs an entire directory
 * Packs an entire directory of files recursively.
 * @param archive archive we pack files to
 * @param path folder to pack
 * @param a_allocator memory allocator to use (ex. heap())
 * @return error
 */
ZPL_DEF sw tar_pack_dir( file* archive, char const* path, allocator a_allocator );

/**
 * @brief Unpacks an existing archive
 * Unpacks an existing archive. Users provide a callback in which information about a_file is provided.
 * Library does not unpack files to the filesystem nor reads any a_file data.
 * @param archive archive we unpack files from
 * @param unpack_proc callback we call per each a_file parsed
 * @param user_data user provided data
 * @return error
 */
ZPL_DEF sw tar_unpack( file* archive, tar_unpack_proc* unpack_proc, void* user_data );

/**
 * @brief Unpacks an existing archive into directory
 * Unpacks an existing archive into directory. The folder structure will be re-created automatically.
 * @param archive archive we unpack files from
 * @param dest directory to unpack files to
 * @return error
 */
ZPL_DEF_INLINE sw tar_unpack_dir( file* archive, char const* dest );

ZPL_DEF ZPL_TAR_UNPACK_PROC( tar_default_list_file );
ZPL_DEF ZPL_TAR_UNPACK_PROC( tar_default_unpack_file );

//! @}

ZPL_IMPL_INLINE sw tar_unpack_dir( file* archive, char const* dest )
{
	return tar_unpack( archive, tar_default_unpack_file, zpl_cast( void* ) dest );
}

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
