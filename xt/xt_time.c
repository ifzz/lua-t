/*
 * \file    l_timer.c
 * \detail  OOP wrapper for Time values. This is a thin wrapper around
 *          struct timeval
*/

#ifdef _WIN32
#include <WinSock2.h>
#include <winsock.h>
#include <time.h>
#include <stdint.h>
#include <WS2tcpip.h>
#include <Windows.h>
#else
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>    // gettimeofday()
#endif

#include "l_xt.h"
#include "xt_time.h"


/**--------------------------------------------------------------------------
 * \brief  adds timeval B to timeval A.
 * \param  *tA struct timeval pointer
 * \param  *tB struct timeval pointer
 * --------------------------------------------------------------------------*/
void xt_time_Add (struct timeval *tA, struct timeval *tB, struct timeval *tX)
{
	struct timeval tC;

	tC.tv_sec    = tB->tv_sec  + tA->tv_sec ;  // add seconds
	tC.tv_usec   = tB->tv_usec + tA->tv_usec ; // add microseconds
	tC.tv_sec   += tC.tv_usec / 1000000 ;      // add microsecond overflow to seconds
	tC.tv_usec  %= 1000000 ;                   // subtract overflow from microseconds
	tX->tv_sec   = tC.tv_sec;
	tX->tv_usec  = tC.tv_usec;
}


/**--------------------------------------------------------------------------
 * \brief  substract timeval B to timeval A.
 * \param  *tA struct timeval pointer
 * \param  *tB struct timeval pointer
 * --------------------------------------------------------------------------*/
void xt_time_Sub (struct timeval *tA, struct timeval *tB, struct timeval *tX)
{
	struct timeval tC;

	tC.tv_sec = (tB->tv_usec > tA->tv_usec)
		? tA->tv_sec - tB->tv_sec - 1
		: tA->tv_sec - tB->tv_sec;
		;
	tC.tv_usec   = tA->tv_usec;
	tC.tv_usec = (tB->tv_usec > tA->tv_usec)
		? 1000000 - (tB->tv_usec - tA->tv_usec)
		: tA->tv_usec - tB->tv_usec;
	tX->tv_sec   = tC.tv_sec;
	tX->tv_usec  = tC.tv_usec;
}


/**--------------------------------------------------------------------------
 * \brief  sets tm to time different between tm and now
 * \param  *tv struct timeval pointer
 * --------------------------------------------------------------------------*/
void xt_time_Since (struct timeval *tA)
{
	struct timeval tC;

	gettimeofday (&tC, 0);
	xt_time_Sub (&tC, tA, tA);
}


/**--------------------------------------------------------------------------
 * \brief  gets milliseconds worth of tm
 * \param  *tv struct timeval pointer
 * \return timeval value in milliseconds
 * --------------------------------------------------------------------------*/
long xt_time_Get_ms (struct timeval *tA)
{
	return tA->tv_sec*1000 + tA->tv_usec/1000;
}


/**--------------------------------------------------------------------------
 * construct a timer an Timer and return it.
 * \param   luaVM  The lua state.
 * \lparam  CLASS table Time
 * \lparam  int    time spam in milliseconds, if omitted time since epoch
 * \lreturn struct timeval userdata.
 * \return  The number of results to be passed back to the calling Lua script.
 * --------------------------------------------------------------------------*/
static int xt_time___call(lua_State *luaVM)
{
	lua_remove(luaVM, 1);
	return xt_time_new(luaVM);
}


/**--------------------------------------------------------------------------
 * create an Timer and return it.
 * \param   luaVM  The lua state.
 * \lparam  int    time spam in milliseconds, if omitted time since epoch
 * \lreturn struct timeval userdata.
 * \return  The number of results to be passed back to the calling Lua script.
 * --------------------------------------------------------------------------*/
int xt_time_new(lua_State *luaVM)
{
	struct timeval  *tv;
	int              ms;

	tv = xt_time_create_ud (luaVM);
	if (lua_isnumber(luaVM, 1)) {
		ms          = luaL_checkint(luaVM, 1);
		tv->tv_sec  = ms/1000;
		tv->tv_usec = (ms % 1000) * 1000;
	}

	return 1;
}


/**--------------------------------------------------------------------------
 * \brief   create an timer userdata and push to LuaStack.
 * \param   luaVM  The lua state.
 * \return  struct sockaddr_in*  pointer to the sockaddr
 * --------------------------------------------------------------------------*/
struct timeval *xt_time_create_ud(lua_State *luaVM)
{
	struct timeval *tv;

	tv = (struct timeval *) lua_newuserdata (luaVM, sizeof(struct timeval) );
	gettimeofday(tv, 0);
	luaL_getmetatable(luaVM, "xt.Time");
	lua_setmetatable(luaVM, -2);
	return tv;
}


/**--------------------------------------------------------------------------
 * \brief   check a value on the stack for being a struct sockaddr_in
 * \param   luaVM    The lua state.
 * \param   int      position on the stack
 * \return  struct sockaddr_in*  pointer to the sockaddr
 * --------------------------------------------------------------------------*/
struct timeval *xt_time_check_ud (lua_State *luaVM, int pos) {
	void *ud = luaL_checkudata(luaVM, pos, "xt.Time");
	luaL_argcheck(luaVM, ud != NULL, pos, "`Timer` expected");
	return (struct timeval *) ud;
}


/**--------------------------------------------------------------------------
 * \brief   set port the timer.
 * \param   luaVM    The lua state.
 * \lparam  sockaddr the sockaddr_in userdata.
 * \lparam  int      the port number.
 * \return  The number of results to be passed back to the calling Lua script.
 * --------------------------------------------------------------------------*/
static int xt_time_set (lua_State *luaVM) {
	struct timeval *tv = xt_time_check_ud(luaVM, 1);
	int             ms;

	if (lua_isnumber(luaVM, 2)) {
		ms = luaL_checkint(luaVM, 2);
		tv->tv_sec  = ms/1000;
		tv->tv_usec = (ms % 1000) * 1000;
	}
	else {
		gettimeofday(tv,0);
	}

	return 0;
}


/**--------------------------------------------------------------------------
 * \brief   get port the timer.
 * \param   luaVM    The lua state.
 * \lparam  sockaddr the sockaddr_in userdata.
 * \lreturn string   xxx.xxx.xxx.xxx formatted string representing sockkaddr IP.
 * \return  The number of results to be passed back to the calling Lua script.
 * --------------------------------------------------------------------------*/
static int xt_time_get (lua_State *luaVM) {
	struct timeval *tv = xt_time_check_ud(luaVM, 1);
	lua_pushinteger(luaVM, xt_time_Get_ms (tv));
	return 1;
}


/**--------------------------------------------------------------------------
 * \brief   prints out the timer.
 * \param   luaVM     The lua state.
 * \lparam  sockaddr  the sockaddr_in userdata.
 * \lreturn string    formatted string representing sockkaddr (IP:Port).
 * \return  The number of results to be passed back to the calling Lua script.
 * --------------------------------------------------------------------------*/
static int xt_time__tostring (lua_State *luaVM) {
	struct timeval *tv = xt_time_check_ud(luaVM, 1);
	lua_pushfstring(luaVM, "xt.Time{%d:%d}: %p",
			//tv->tv_sec*1000 + tv->tv_usec/1000,
			tv->tv_sec, tv->tv_usec,
			tv
	);
	return 1;
}


/**--------------------------------------------------------------------------
 * \brief   compares two time values.
 * \param   luaVM     The lua state.
 * \lparam  timeval   the timval userdata.
 * \lparam  timeval   timval userdata to compare to.
 * \lreturn boolean   true if equal otherwise false.
 * \return  The number of results to be passed back to the calling Lua script.
 * --------------------------------------------------------------------------*/
static int xt_time__eq (lua_State *luaVM) {
	struct timeval *tA = xt_time_check_ud(luaVM, 1);
	struct timeval *tB = xt_time_check_ud(luaVM, 2);
	if (tA->tv_sec == tB->tv_sec && tA->tv_usec == tB->tv_usec)
		lua_pushboolean(luaVM, 1);
	else
		lua_pushboolean(luaVM, 0);
	return 1;
}


/**--------------------------------------------------------------------------
 * \brief   adds two time values.
 * \param   luaVM     The lua state.
 * \lparam  timeval   the timval userdata.
 * \lparam  timeval   timval userdata to add to.
 * \lreturn timeval   the difference in time.
 * \return  The number of results to be passed back to the calling Lua script.
 * --------------------------------------------------------------------------*/
static int xt_time__add (lua_State *luaVM) {
	struct timeval *tA = xt_time_check_ud(luaVM, 1);
	struct timeval *tB = xt_time_check_ud(luaVM, 2);
	struct timeval *tC = xt_time_create_ud(luaVM);

	xt_time_Add (tA,tB,tC);
	return 1;
}


/**--------------------------------------------------------------------------
 * \brief   substract two time values.
 * \param   luaVM     The lua state.
 * \lparam  timeval   the timval userdata.
 * \lparam  timeval   timval userdata to add to.
 * \lreturn timeval   true if equal otherwise false.
 * \return  The number of results to be passed back to the calling Lua script.
 * --------------------------------------------------------------------------*/
static int xt_time__sub (lua_State *luaVM) {
	struct timeval *tA = xt_time_check_ud(luaVM, 1);
	struct timeval *tB = xt_time_check_ud(luaVM, 2);
	struct timeval *tC = xt_time_create_ud(luaVM);
	xt_time_Sub (tA,tB,tC);
	return 1;
}


/**
 * \brief      a system call to sleep (Lua lacks that)
 *             Lua has no build in sleep method.
 * \param      The Lua state.
 * \lparam     int  milliseconds to sleep
 * \return     0 return values
 */
static int xt_time_sleep(lua_State *luaVM)
{
#ifdef _WIN32
	fd_set dummy;
	int s;
#endif
	struct timeval *tv;
	long  sec, usec;
	if (lua_isnumber(luaVM, -1))  xt_time_new(luaVM);
	tv  = xt_time_check_ud(luaVM, -1);
	sec = tv->tv_sec; usec=tv->tv_usec;
#ifdef _WIN32
	s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	FD_ZERO(&dummy);
	FD_SET(s, &dummy);
	select(0, 0,0,&dummy, tv);
#else
	select(0, 0,0,0, tv);
#endif
	tv->tv_sec=sec; tv->tv_usec=usec;
	return 0;
}


/**
 * \brief    the metatble for the module
 */
static const struct luaL_Reg xt_time_fm [] = {
	{"__call",      xt_time___call},
	{NULL,   NULL}
};


/**
 * \brief      the Timer library definition
 *             assigns Lua available names to C-functions
 */
static const struct luaL_Reg xt_time_m [] = {
	{"new",     xt_time_new},
	{"set",     xt_time_set},
	{"get",     xt_time_get},
	{"sleep",   xt_time_sleep},
	{NULL,   NULL}
};


/**
 * \brief      the Time library class functions definition
 *             assigns Lua available names to C-functions
 */
static const luaL_Reg xt_time_cf [] =
{
	{"new",   xt_time_new},
	{"sleep", xt_time_sleep},
	{NULL,        NULL}
};


/**--------------------------------------------------------------------------
 * \brief   pushes the Timer library onto the stack
 *          - creates Metatable with functions
 *          - creates metatable with methods
 * \param   luaVM     The lua state.
 * \lreturn string    the library
 * \return  The number of results to be passed back to the calling Lua script.
 * --------------------------------------------------------------------------*/
int luaopen_xt_time (lua_State *luaVM) {
	// just make metatable known to be able to register and check userdata
	luaL_newmetatable(luaVM, "xt.Time");   // stack: functions meta
	luaL_newlib(luaVM, xt_time_m);
	lua_setfield(luaVM, -2, "__index");
	lua_pushcfunction(luaVM, xt_time__tostring);
	lua_setfield(luaVM, -2, "__tostring");
	lua_pushcfunction(luaVM, xt_time__eq);
	lua_setfield(luaVM, -2, "__eq");
	lua_pushcfunction(luaVM, xt_time__add);
	lua_setfield(luaVM, -2, "__add");
	lua_pushcfunction(luaVM, xt_time__sub);
	lua_setfield(luaVM, -2, "__sub");
	lua_pop(luaVM, 1);        // remove metatable from stack

	// Push the class onto the stack
	// this is avalable as Timer.localhost
	luaL_newlib(luaVM, xt_time_cf);
	// set the methods as metatable
	// this is only avalable a <instance>:func()
	luaL_newlib(luaVM, xt_time_fm);
	lua_setmetatable(luaVM, -2);
	return 1;
}
