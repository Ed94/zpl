// file: header/core/random.h


ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

typedef struct random
{
	u32 offsets[ 8 ];
	u32 value;
} random;

// NOTE: Generates from numerous sources to produce a decent pseudo-random seed
ZPL_DEF void random_init( random* r );
ZPL_DEF u32  random_gen_u32( random* r );
ZPL_DEF u32  random_gen_u32_unique( random* r );
ZPL_DEF u64  random_gen_u64( random* r );    // NOTE: (zpl_random_gen_u32() << 32) | zpl_random_gen_u32()
ZPL_DEF sw   random_gen_isize( random* r );
ZPL_DEF s64  random_range_i64( random* r, s64 lower_inc, s64 higher_inc );
ZPL_DEF sw   random_range_isize( random* r, sw lower_inc, sw higher_inc );
ZPL_DEF f64  random_range_f64( random* r, f64 lower_inc, f64 higher_inc );

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
