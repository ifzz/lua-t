
struct xt_buf {
	size_t         len;   // length in bytes
	unsigned char  b[1];  // variable size buffer -> last in struct
};


enum xt_buf_fld_type {
	BUF_FLD_BIT,    /* X  Bit  wide field*/
	BUF_FLD_BYTE,   /* X  Byte wide field*/
	BUF_FLD_STR     /* string buffer field */
};


struct xt_buf_fld {
	enum  xt_buf_fld_type type;    /* type of field  */
	/* pointer to position in buffer according to type */
	uint8_t           *v8;
	uint16_t          *v16;
	uint32_t          *v32;
	uint64_t          *v64; 
	char              *vS;
	// accessor function pointers
	int              (*write) (lua_State *luaVM);
	int              (*read)  (lua_State *luaVM);
	/* helpers for bitwise access */
	uint8_t            shft;
	uint8_t            m8;      /* (*v8  & m8)  >> shft == actual_value */
	uint16_t           m16;     /* (*v16 & m16) >> shft == actual_value */
	uint32_t           m32;     /* (*v32 & m32) >> shft == actual_value */
	uint64_t           m64;     /* (*v64 & m64) >> shft == actual_value */
	/* size information */
	size_t             sz_bit;   /* size in bits   */
	size_t             ofs_bit;  /* how many bits  into the byte does it start    */
	size_t             ofs_byte; /* how many bytes into the buffer does it start  */
};


// Constructors
// l_xt_buf.c
int              luaopen_buf    (lua_State *luaVM);
struct xt_buf   *check_ud_buf   (lua_State *luaVM, int pos);
struct xt_buf   *create_ud_buf  (lua_State *luaVM, int size);


// l_xt_buf_fld.c
int                 luaopen_buf_fld      (lua_State *luaVM);
int                 c_new_buf_fld_bits   (lua_State *luaVM);
int                 c_new_buf_fld_byte   (lua_State *luaVM);
int                 c_new_buf_fld_string (lua_State *luaVM);
struct xt_buf_fld  *check_ud_buf_fld     (lua_State *luaVM, int pos);
struct xt_buf_fld  *create_ud_buf_fld    (lua_State *luaVM);
