/* vim: ts=3 sw=3 sts=3 tw=80 sta noet list
*/
/**
 * \file      xt_enc.h
 * \brief     Data Types and global functions for various En?decoders
 * \author    tkieslich
 * \copyright See Copyright notice at the end of xt.h
 */

/// struct to keep track of a Arc4 encoding
struct xt_enc_arc4 {
	unsigned char  prng[256]; ///< Pseudo RNG, aka. the state
	uint8_t        i1;        ///< current index 1
	uint8_t        i2;        ///< current index 2
};


///  struct to keep track of a CRC encoding
struct xt_enc_crc {
	/// Encoded polynom table
	union {
		uint8_t       t8 [256];
		uint16_t      t16[256];
		uint32_t      t32[256];
	};
	/// runing CRC value
	union {
		uint8_t       crc8;
		uint16_t      crc16;
		uint32_t      crc32;
	};
	/// initial CRC value
	union {
		uint8_t       init8;
		uint16_t      init16;
		uint32_t      init32;
	};
	int              be;    ///< boolean BigEndian
	int             (*calc)( struct xt_enc_crc *crc, const char *data, size_t len );
};


// Constructors
// xt_enc_arc4.c
struct xt_enc_arc4 *xt_enc_arc4_check_ud ( lua_State *luaVM, int pos );
struct xt_enc_arc4 *xt_enc_arc4_create_ud( lua_State *luaVM );
int                 lxt_enc_arc4_New     ( lua_State *luaVM );
int                 luaopen_xt_enc_arc4  ( lua_State *luaVM );

// xt_enc_crc.c
struct xt_enc_crc  *xt_enc_crc_check_ud  ( lua_State *luaVM, int pos );
struct xt_enc_crc  *xt_enc_crc_create_ud ( lua_State *luaVM );
int                 lxt_enc_crc_New      ( lua_State *luaVM );
int                 luaopen_xt_enc_crc   ( lua_State *luaVM );

// xt_enc_b64.c
int                 luaopen_xt_enc_b64   ( lua_State *luaVM );

