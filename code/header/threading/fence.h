// a_file: header/threading/fence.h

// Fences

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS

ZPL_DEF void yield_thread( void );
ZPL_DEF void mfence( void );
ZPL_DEF void sfence( void );
ZPL_DEF void lfence( void );

ZPL_END_C_DECLS
ZPL_END_NAMESPACE
