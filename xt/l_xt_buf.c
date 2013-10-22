//
//
#include <memory.h>               // memset
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>            // htonl

#include "l_xt.h"
#include "l_xt_buf.h"


// inline helper functions
/**
  * \brief  convert lon long (64bit) from network to host and vice versa
  * \param  uint64_t value 64bit integer
  * \return uint64_t endianess corrected integer
  */
static inline uint64_t htonll(uint64_t value)
{
	uint64_t high_part = htonl( (uint64_t)(value >> 32) );
	uint64_t low_part  = htonl( (uint64_t)(value & 0xFFFFFFFFLL) );
	return (((uint64_t)low_part) << 32) | high_part;
}


/**
  * \brief  gets a numeric value in a buf segment according to mask and
  *         shift
  * \param  uint64_t the position in the buffer (pointer)
  * \param  uint64_t out_mask  in relation to a 8 byte integer
  * \param  uint64_t out_shift to right end of 64 bit integer
  * \return uint64_t
  */
static inline uint64_t get_segment_value_bits (
		uint64_t  *v,
		uint64_t   mask,
		uint64_t   shift)
{
	return ((htonll (*v) & mask) >> shift);
}

/**
 * \brief     convert 8bit integer to BCD
 * \param     val  8bit integer
 * \return    8bit integer encoding of a 2 digit BCD number
 */
int l_byteToBcd(lua_State *luaVM)
{
	uint8_t val = luaL_checkint(luaVM, 1);
	lua_pushinteger(luaVM, (val/10*16) + (val%10) );
	return 1;
}

/**
 * \brief     convert 16bit integer to BCD
 * \param     val  16bit integer
 * \return    16bit integer encoding of a BCD coded number
 */
int l_shortToBcd(lua_State *luaVM)
{
	uint16_t val = luaL_checkint(luaVM, 1);
	lua_pushinteger(luaVM,
			(val/1000   *4096 ) +
		 ( (val/100%10)* 256 ) +
		 ( (val/10%10) * 16 ) +
			(val%10)
	);
	return 1;
}


/**
  * \brief  sets a numeric value in a qtc_pfield struct according to mask and
  *         shift
  * \param  lua_State
  * \param  struct qtc_pfield  the field to operate on
  * \param  uint64_t value the value to put into the pointer to the main buffer
  * \return int    that's what gets returned to Lua, meaning the one value on
  *                the stack
  */
static inline void set_segment_value_numeric (
		uint64_t  *v,
		uint64_t   mask,
		uint64_t   shift,
		uint64_t   nv)
{
	//uint64_t cmpr;
	//cmpr = (64 == a->bits) ?  (uint64_t)0x1111111111111111 : ((uint64_t) 0x0000000000000001) << a->bits; // 2^bits
	//if ( value < cmpr ) {
	*v = htonll( ( htonll(*v) & ~mask) | (nv << shift) );
	//}
	//else
	//	return luaL_error(luaVM,
	//	         "The value %d is too big for a %d bits wide field",
	//	         value, a->bits);
}


/**
 * \brief     creates the buffer for the the network function
 * \detail    it creates the buffer used to stor all information and send them
 *            out to the network. In order to guarantee the binary operations
 *            for 64 bit integers it must be 8 bytes longer than what gets send
 *            out. By the same time the 8bytes padding in the ned provide space
 *            for two bytes used as placeholder for the CRC16 checksum if needed
 * \param     lua state
 * \return    integer   how many elements are placed on the Lua stack
*/
static int c_new_buf(lua_State *luaVM)
{
	int                    sz;
	struct xt_buf  __attribute__ ((unused)) *b;

	sz  = luaL_checkint(luaVM, 2);
	b   = create_ud_buf(luaVM, sz);

	return 1;
}


/**--------------------------------------------------------------------------
 * \brief   create a xt_buf and push to LuaStack.
 * \param   luaVM  The lua state.
 * \return  struct xt_buf*  pointer to the socket xt_buf
 * --------------------------------------------------------------------------*/
struct xt_buf *create_ud_buf(lua_State *luaVM, int size)
{
	struct xt_buf  *b;
	size_t          sz;

	sz = sizeof(struct xt_buf) + (size - 1) * sizeof(unsigned char);
	b  = (struct xt_buf *) lua_newuserdata(luaVM, sz);
	memset(b->b, 0, size * sizeof(unsigned char));

	b->len = size;
	luaL_getmetatable(luaVM, "L.Buffer");
	lua_setmetatable(luaVM, -2);
	return b;
}


/**
 * \brief  gets the value of the element
 * \param  position in bytes
 * \param  offset   in bits
 * \param  len   in bits
 *
 * \return pointer to struct buf
 */
struct xt_buf *check_ud_buf (lua_State *luaVM, int pos) {
	void *ud = luaL_checkudata(luaVM, pos, "L.Buffer");
	luaL_argcheck(luaVM, ud != NULL, pos, "`xt.Buffer` expected");
	return (struct xt_buf *) ud;
}


/**
 * \brief  gets the value of the element
 * \param  position in bytes
 * \param  offset   in bits
 * \param  len   in bits
 *
 * \return integer 1 left on the stack
 */
static int l_read_number_bits (lua_State *luaVM) {
	int                   len;    // how many bits to write
	int                   ofs;    // starting with the x bit
	uint64_t             *v;
	uint64_t              mask;
	uint64_t              shft;
	struct xt_buf *b   = check_ud_buf(luaVM, 1);

	ofs = luaL_checkint(luaVM, 2);
	len = luaL_checkint(luaVM, 3);

	shft = 64 - (ofs%8) - len;
	mask = ( 0xFFFFFFFFFFFFFFFF >> (64-len)) << shft;

	v = (uint64_t *) &(b->b[ ofs/8 ]);
	lua_pushinteger(luaVM, get_segment_value_bits(v, mask, shft));
	return 1;
}


/**
 * \brief  gets a byte wide the value from stream
 * \lparam  position in bytes
 *
 * \return integer 1 left on the stack
 */
static int l_read_8 (lua_State *luaVM) {
	uint8_t       *v;
	int            p = luaL_checkint(luaVM,2); // starting byte  b->b[pos]
	struct xt_buf *b   = check_ud_buf(luaVM, 1);

	v = (uint8_t *) &(b->b[ p ]);
	lua_pushinteger(luaVM, (int) *v);
	return 1;
}


/**
 * \brief  gets a short 2 byte wide value from stream
 * \lparam  position in bytes
 *
 * \return integer 1 left on the stack
 */
static int l_read_16 (lua_State *luaVM) {
	uint16_t      *v;
	int            p = luaL_checkint(luaVM,2); // starting byte  b->b[pos]
	struct xt_buf *b = check_ud_buf(luaVM, 1);

	v = (uint16_t *) &(b->b[ p ]);
	lua_pushinteger(luaVM, (int) htons (*v));
	return 1;
}


/**
 * \brief  gets a long 4 byte wide value from stream
 * \lparam  position in bytes
 *
 * \return integer 1 left on the stack
 */
static int l_read_32 (lua_State *luaVM) {
	uint32_t      *v;
	int            p = luaL_checkint(luaVM,2); // starting byte  b->b[pos]
	struct xt_buf *b = check_ud_buf(luaVM, 1);

	v = (uint32_t *) &(b->b[ p ]);
	lua_pushinteger(luaVM, (int) htonl (*v));
	return 1;
}


/**
 * \brief  gets a long long 8 byte wide value from stream
 * \lparam  position in bytes
 *
 * \return integer 1 left on the stack
 */
static int l_read_64 (lua_State *luaVM) {
	uint64_t      *v;
	int            p = luaL_checkint(luaVM,2); // starting byte  b->b[pos]
	struct xt_buf *b = check_ud_buf(luaVM, 1);

	v = (uint64_t *) &(b->b[ p ]);
	lua_pushinteger(luaVM, (int) htonll (*v));
	return 1;
}


/**
 * \brief    sets a value at a position in the stream
 *
 * \return integer 0 left on the stack
 */
static int l_write_number_bits(lua_State *luaVM) {
	int            len;    // how many bits to write
	int            ofs;    // starting with the x bit
	uint64_t      *v;
	uint64_t       mask;
	uint64_t       shft;
	struct xt_buf *b     = check_ud_buf(luaVM, 1);

	ofs = luaL_checkint(luaVM, 2);
	len = luaL_checkint(luaVM, 3);

	shft = 64 - (ofs%8) - len;
	mask = ( 0xFFFFFFFFFFFFFFFF >> (64-len)) << shft;

	v = (uint64_t *) &(b->b[ ofs/8 ]);

	set_segment_value_numeric(
		v, mask, shft,
		(uint64_t) luaL_checknumber(luaVM, 4));
	return 0;
}


/**
 * \brief  sets a char 2 byte wide value in stream
 * \lparam  position in bytes
 * \lparam  value
 *
 * \return integer 0 left on the stack
 */
static int l_write_8(lua_State *luaVM) {
	uint8_t       *v;
	int            p = luaL_checkint(luaVM,2); // starting byte  b->b[pos]
	struct xt_buf *b = check_ud_buf(luaVM, 1);

	v  = (uint8_t *) &(b->b[ p ]);
	*v = (uint8_t) luaL_checknumber(luaVM, 3);
	return 0;
}


/**
 * \brief  sets a short 2 byte wide value in stream
 * \lparam  position in bytes
 * \lparam  value
 *
 * \return integer 0 left on the stack
 */
static int l_write_16(lua_State *luaVM) {
	uint16_t      *v;
	int            p = luaL_checkint(luaVM,2); // starting byte  b->b[pos]
	struct xt_buf *b = check_ud_buf(luaVM, 1);

	v  = (uint16_t *) &(b->b[ p ]);
	*v = htons( (uint16_t) luaL_checknumber(luaVM, 3) );
	return 0;
}


/**
 * \brief  sets a long 4 byte wide value in stream
 * \lparam  position in bytes
 * \lparam  value
 *
 * \return integer 0 left on the stack
 */
static int l_write_32(lua_State *luaVM) {
	uint32_t      *v;
	int            p = luaL_checkint(luaVM,2); // starting byte  b->b[pos]
	struct xt_buf *b = check_ud_buf(luaVM, 1);

	v  = (uint32_t *) &(b->b[ p ]);
	*v = htonl( (uint32_t) luaL_checknumber(luaVM, 3) );
	return 0;
}


/**
 * \brief  sets a long long 8 byte wide value in stream
 * \lparam  position in bytes
 * \lparam  value
 *
 * \return integer 0 left on the stack
 */
static int l_write_64(lua_State *luaVM) {
	uint64_t      *v;
	int            p = luaL_checkint(luaVM,2); // starting byte  b->b[pos]
	struct xt_buf *b = check_ud_buf(luaVM, 1);

	v  = (uint64_t *) &(b->b[ p ]);
	*v = htonll( (uint64_t) luaL_checknumber(luaVM, 3) );
	return 0;
}


/**
 * \brief    gets the content of the Stream in Hex
 * lreturn   string buffer representation in Hexadecimal
 *
 * \return integer 0 left on the stack
 */
static int l_get_hex_string(lua_State *luaVM) {
	int            l,c;
	char          *sbuf;
	struct xt_buf *b   = check_ud_buf(luaVM, 1);

	sbuf = malloc(3 * b->len * sizeof( char ));
	memset(sbuf, 0, 3 * b->len * sizeof( char ) );

	c = 0;
	for (l=0; l < (int) b->len; l++) {
		c += snprintf(sbuf+c, 4, "%02X ", b->b[l]);
	}
	lua_pushstring(luaVM, sbuf);
	return 1;
}


/**
 * \brief     returns len of the buffer
 * \param     lua state
 * \return    integer   how many elements are placed on the Lua stack
*/
static int l_get_len(lua_State *luaVM)
{
	struct xt_buf *b;

	b   = check_ud_buf(luaVM, 1);
	lua_pushinteger(luaVM, (int) b->len);
	return 1;
}


/**--------------------------------------------------------------------------
 * \brief   tostring representation of a buffer stream.
 * \param   luaVM     The lua state.
 * \lparam  sockaddr  the buffer-Stream in user_data.
 * \lreturn string    formatted string representing buffer.
 * \return  The number of results to be passed back to the calling Lua script.
 * --------------------------------------------------------------------------*/
static int l_stream_tostring (lua_State *luaVM) {
	struct xt_buf *bs = check_ud_buf(luaVM, 1);
	lua_pushfstring(luaVM, "Stream{%d}: %p", bs->len, bs);
	return 1;
}


/**
 * \brief    the metatble for the module
 */
static const struct luaL_Reg l_buf_fm [] = {
	{"__call",      c_new_buf},
	{NULL,   NULL}
};


/**
 * \brief      the buffer library definition
 *             assigns Lua available names to C-functions
 */
static const luaL_Reg l_buf_m [] =
{
	{"readBits",     l_read_number_bits},
	{"writeBits",    l_write_number_bits},
	{"read8",        l_read_8},
	{"read16",       l_read_16},
	{"read32",       l_read_32},
	{"read64",       l_read_64},
	{"write8",       l_write_8},
	{"write16",      l_write_16},
	{"write32",      l_write_32},
	{"write64",      l_write_64},
	{"length",       l_get_len},
	{"toHex",        l_get_hex_string},
	{"Segment",      c_new_buf_seg},
	{NULL,           NULL}
};


/**--------------------------------------------------------------------------
 * \brief   pushes the BufferBuffer library onto the stack
 *          - creates Metatable with functions
 *          - creates metatable with methods
 * \param   luaVM     The lua state.
 * \lreturn string    the library
 * \return  The number of results to be passed back to the calling Lua script.
 * --------------------------------------------------------------------------*/
LUAMOD_API int luaopen_buf (lua_State *luaVM)
{
	luaL_newmetatable(luaVM, "L.Buffer");   // stack: functions meta
	luaL_newlib(luaVM, l_buf_m);
	lua_setfield(luaVM, -2, "__index");
	lua_pushcfunction(luaVM, l_get_len);
	lua_setfield(luaVM, -2, "__len");
	lua_pushcfunction(luaVM, l_stream_tostring);
	lua_setfield(luaVM, -2, "__tostring");
	lua_pop(luaVM, 1);        // remove metatable from stack
	// empty Buffer class = {}, this is the actual return of this function
	lua_createtable(luaVM, 0, 0);
	luaopen_buf_seg(luaVM);
	lua_setfield(luaVM, -2, "Segment");
	luaL_newlib(luaVM, l_buf_fm);
	lua_setmetatable(luaVM, -2);
	return 1;
}

