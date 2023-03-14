// a_file: header/hashing.h

/** @a_file hashing.c
@brief Hashing and Checksum Functions
@defgroup hashing Hashing and Checksum Functions

Several hashing methods used by zpl internally but possibly useful outside of it. Contains: adler32, crc32/64, fnv32/64/a and murmur32/64

@{
*/

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

ZPL_DEF u32 adler32( void const* data, sw len );

ZPL_DEF u32 crc32( void const* data, sw len );
ZPL_DEF u64 crc64( void const* data, sw len );

// These use FNV-1 algorithm
ZPL_DEF u32 fnv32( void const* data, sw len );
ZPL_DEF u64 fnv64( void const* data, sw len );
ZPL_DEF u32 fnv32a( void const* data, sw len );
ZPL_DEF u64 fnv64a( void const* data, sw len );

ZPL_DEF u8* base64_encode( allocator a, void const* data, sw len );
ZPL_DEF u8* base64_decode( allocator a, void const* data, sw len );

//! Based on MurmurHash3
ZPL_DEF u32 murmur32_seed( void const* data, sw len, u32 seed );

//! Based on MurmurHash2
ZPL_DEF u64 murmur64_seed( void const* data, sw len, u64 seed );

//! Default seed of 0x9747b28c
ZPL_DEF_INLINE u32 murmur32( void const* data, sw len );

//! Default seed of 0x9747b28c
ZPL_DEF_INLINE u64 murmur64( void const* data, sw len );

//! @}

ZPL_IMPL_INLINE u32 murmur32( void const* data, sw len )
{
	return murmur32_seed( data, len, 0x9747b28c );
}

ZPL_IMPL_INLINE u64 murmur64( void const* data, sw len )
{
	return murmur64_seed( data, len, 0x9747b28c );
}

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
