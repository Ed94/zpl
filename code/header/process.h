// file: header/process.h

/** @file process.c
@brief Process creation and manipulation methods
@defgroup process Process creation and manipulation methods

Gives you the ability to create a new process, wait for it to end or terminate it.
It also exposes standard I/O with configurable options.

@{
*/

ZPL_BEGIN_NAMESPACE
ZPL_BEGIN_C_DECLS
// TODO(zaklaus): Add Linux support

typedef enum
{
	ZPL_PR_OPTS_COMBINE_STD_OUTPUT = ZPL_BIT( 1 ),
	ZPL_PR_OPTS_INHERIT_ENV        = ZPL_BIT( 2 ),
	ZPL_PR_OPTS_CUSTOM_ENV         = ZPL_BIT( 3 ),
} pr_opts;

typedef struct
{
	zpl_file in, out, err;
	void *   f_stdin, *f_stdout, *f_stderr;
#ifdef ZPL_SYSTEM_WINDOWS
	void* win32_handle;
#else
	// todo
#endif
} pr;

typedef struct
{
	char* con_title;
	char* workdir;

	sw     env_count;
	char** env; // format: "var=name"

	u32 posx, posy;
	u32 resx, resy;
	u32 bufx, bufy;
	u32 fill_attr;
	u32 flags;
	b32 show_window;
} pr_si;

ZPL_DEF s32  pr_create( pr* process, const char** args, sw argc, pr_si si, pr_opts options );
ZPL_DEF void pr_destroy( pr* process );
ZPL_DEF void pr_terminate( pr* process, s32 err_code );
ZPL_DEF s32  pr_join( pr* process );

//! @}
ZPL_END_C_DECLS
ZPL_END_NAMESPACE
