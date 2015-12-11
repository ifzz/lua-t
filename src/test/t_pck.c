/* vim: ts=3 sw=3 sts=3 tw=80 sta noet list
*/
/**
 * \file      test/t_pck.c
 * \brief     Unit test for the lua-t bit/byte packaging source code
 * \author    tkieslich
 * \copyright See Copyright notice at the end of t.h
 */

#include "t_unittest.h"

static int
test_is_digit( )
{
	const char  *str = ">I3<i2bB>I5<I4h";
	char       **fmt = (char**)  &str;
	int          opt = *( *(fmt)++ );
	while (NULL != *fmt)
	{
		_assert( is_digit( opt ) == 1 );
	}
	return 0;
}


static int
test_is_packer( )
{
	lua_State    *L   = luaL_newstate( );
	const char   *str = ">I3<i2bB>I5<I4h";
	const char  **fmt =   &str;
	int           end = 1;
	int           bof = 0;
	int           n   = 0;
	struct t_pck *p   = t_pck_get_packer_definition_from_string( L, *fmt, &end, bof );
	while (NULL != p ) // add packers on stack
	{
		n++;
		p   = t_pck_get_packer_definition_from_string( L, *fmt, &end, bof );
	}
	return 0;
}


// Add all testable functions to the array
static const struct test_function all_tests [] = {
	{ "Test parsing format string",      test_is_digit },
	{ "Test creating a row of packers",  test_is_packer },
	{ NULL, NULL }
};

int
main()
{
	return test_execute( all_tests );
}
