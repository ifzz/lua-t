/* vim: ts=3 sw=3 sts=3 tw=80 sta noet list
*/
/**
 * \file      t_pck.h
 * \brief     data types for packers
 * \author    tkieslich
 * \copyright See Copyright notice at the end of t.h
 */

// ========== Buffer accessor Helpers

/* number of bits in a character */
#define NB                 CHAR_BIT

/* mask for one character (NB 1's) */
#define MC                 ((1 << NB) - 1)

/* size of a lua_Integer */
#define MXINT              ((int)sizeof(lua_Integer))

// Maximum bits that can be read or written
#define MXBIT              MXINT * NB

// Macro helpers
#define BIT_GET(b,n)       ( ((b) >> (NB-(n))) & 0x01 )
#define BIT_SET(b,n,v)     \
	( (1==v)              ? \
	 ((b) | (  (0x01) << (NB-(n))))    : \
	 ((b) & (~((0x01) << (NB-(n))))) )


#define BIG_ENDIAN         0
#define LITTLE_ENDIAN      1


// T.Pack is designed to work like Lua 5.3 pack/unpack support.  By the same
// time it shall have more convienience and be more explicit.

// TODO: make the check of enums based on Bit Masks to have subtype grouping
enum t_pck_t {
	// atomic packer types
	T_PCK_INT,      ///< Packer         Integer
	T_PCK_UNT,      ///< Packer         Unsigned Integer
	T_PCK_FLT,      ///< Packer         Float
	T_PCK_BOL,      ///< Packer         Boolean (1 Bit)
	T_PCK_BTS,      ///< Packer         Bit (integer x Bit) - signed
	T_PCK_BTU,      ///< Packer         Bit (integer x Bit) - unsigned
	T_PCK_RAW,      ///< Packer         Raw  -  string/utf8/binary
	// complex packer types
	T_PCK_ARR,      ///< Combinator     Array
	T_PCK_SEQ,      ///< Combinator     Sequence
	T_PCK_STR,      ///< Combinator     Struct
};

static const char *const t_pck_t_lst[] = {
	// atomic packer types
	"Int",          ///< Packer         Integer
	"UInt",         ///< Packer         Unsigned Integer
	"Float",        ///< Packer         Float
	"Boolean",      ///< Packer         Boolean (1 Bit)
	"BitSigned",    ///< Packer         Bit (integer x Bit)
	"BitUnsigned",  ///< Packer         Bit (integer x Bit)
	"Raw",          ///< Packer         Raw  -  string/utf8/binary
	// complex packer types
	"Array",        ///< Combinator     Array
	"Sequence",     ///< Combinator     Sequence
	"Struct",       ///< Combinator     Struct
};


/// The userdata struct for T.Pack/T.Pack.Struct
struct t_pck {
	enum  t_pck_t  t;   ///< type of packer
	/// size of packer -> various meanings
	///  -- for int/uint, float, raw =  the number of bytes
	///  -- for bit, bits and nibble =  the number of bits
	///  -- for Seq,Struct,Arr       =  the number of elements in this Combinator
	size_t         s;
	/// modifier -> various meanings
	///  -- for int/uint             = Endian (0=BIG_ENDIAN, 1=LITTLE_ENDIAN)
	///  -- for bit                  = Offset from beginning of byte (bit numbering: MSB 0)
	///  -- for raw                  = ??? (unused)
	///  -- for Arr                  = lua registry Reference for packer
	///  -- for Seq                  = lua registry Reference for the table
	///        idx[ i    ] = Pack
	///        idx[ s+i  ] = ofs
	///  -- for Struct               = lua registry Reference for the table
	///        idx[ i    ] = Pack
	///        idx[ s+i  ] = ofs
	///        idx[ 2s+i ] = name
	///        idx[ name ] = i
	int            m;
};


/// Union for serializing floats (taken from Lua 5.3)
union Ftypes {
	float       f;
	double      d;
	lua_Number  n;
	char buff[ 5 * sizeof( lua_Number ) ];  // enough for any float type
};


/// The userdata struct for T.Pack.Reader
struct t_pcr {
	int      r;   ///< reference to packer type
	size_t   o;   ///< offset from the beginning of the wrapping Struct
	//size_t          s;   ///< size of this Reader
};


// t_pck.c
// Constructors
struct t_pck *t_pck_check_ud ( lua_State *L, int pos, int check );
struct t_pck *t_pck_create_ud( lua_State *L, enum t_pck_t t, size_t s, int m );

// accessor helpers for the Packers
int t_pck_read ( lua_State *L, struct t_pck *p, const unsigned char *buffer);
int t_pck_write( lua_State *L, struct t_pck *p, unsigned char *buffer );

// helpers for the Packers
struct t_pck *t_pck_getpck( lua_State *L, int pos, size_t *bo );
int           t_pcr__callread ( lua_State *L, struct t_pck *pc, const unsigned char *b );

