/* vim: ts=3 sw=3 sts=3 tw=80 sta noet list
*/
/**
 * \file      t_htp_srv.h
 * \brief     OOP wrapper for HTTP operation
 * \author    tkieslich
 * \copyright See Copyright notice at the end of t.h
 */


#include <string.h>               // memset

#include "t.h"
#include "t_htp.h"
#include "t_buf.h"


static int t_htp_con_read( lua_State *luaVM );


/** ---------------------------------------------------------------------------
 * Creates an T.Http.Server.
 * \param    luaVM    lua state.
 * \lparam   function WSAPI style request handler.
 * \return integer # of elements pushed to stack.
 *  -------------------------------------------------------------------------*/
static int
lt_htp_srv_New( lua_State *luaVM )
{
	struct t_htp_srv *s;
	struct t_elp     *l;

	if (lua_isfunction( luaVM, -1 ) && (l = t_elp_check_ud( luaVM, -2, 1 )))
	{
		s     = t_htp_srv_create_ud( luaVM );
		lua_insert( luaVM, -3 );
		s->rR = luaL_ref( luaVM, LUA_REGISTRYINDEX );
		s->lR = luaL_ref( luaVM, LUA_REGISTRYINDEX );
	}
	else
		return t_push_error( luaVM, "T.Http.Server( func ) requires a function as parameter" );
	return 1;
}


/**--------------------------------------------------------------------------
 * construct an HTTP Server
 * \param   luaVM    The lua state.
 * \lparam  CLASS    table Http.Server
 * \lparam  T.Socket sub protocol
 * \lreturn userdata struct t_htp_srv* ref.
 * \return  int    # of elements pushed to stack.
 * --------------------------------------------------------------------------*/
static int lt_htp_srv__Call( lua_State *luaVM )
{
	lua_remove( luaVM, 1 );
	return lt_htp_srv_New( luaVM );
}


/**--------------------------------------------------------------------------
 * create a t_htp_srv and push to LuaStack.
 * \param   luaVM  The lua state.
 *
 * \return  struct t_htp_srv*  pointer to the struct.
 * --------------------------------------------------------------------------*/
struct t_htp_srv
*t_htp_srv_create_ud( lua_State *luaVM )
{
	struct t_htp_srv *s;
	s = (struct t_htp_srv *) lua_newuserdata( luaVM, sizeof( struct t_htp_srv ));

	luaL_getmetatable( luaVM, "T.Http.Server" );
	lua_setmetatable( luaVM, -2 );
	return s;
}


/**--------------------------------------------------------------------------
 * Check if the item on stack position pos is an t_htp_srv struct and return it
 * \param  luaVM    the Lua State
 * \param  pos      position on the stack
 *
 * \return  struct t_htp_srv*  pointer to the struct.
 * --------------------------------------------------------------------------*/
struct t_htp_srv
*t_htp_srv_check_ud( lua_State *luaVM, int pos, int check )
{
	void *ud = luaL_checkudata( luaVM, pos, "T.Http.Server" );
	luaL_argcheck( luaVM, (ud != NULL || !check), pos, "`T.Http.Server` expected" );
	return (struct t_htp_srv *) ud;
}


/**--------------------------------------------------------------------------
 * Accept a connection from a Http.Server listener.
 * Called anytime a new connection gets established.
 * \param   luaVM     lua Virtual Machine.
 * \lparam  userdata  struct t_htp_srv.
 * \param   pointer to the buffer to read from(already positioned).
 * \lreturn value from the buffer a packers position according to packer format.
 * \return  integer number of values left on the stack.
 *  -------------------------------------------------------------------------*/
static int
lt_htp_srv_accept( lua_State *luaVM )
{
	struct t_htp_srv   *s     = lua_touserdata( luaVM, 1 );
	struct sockaddr_in *si_cli;
	struct t_sck       *c_sck;
	struct t_sck       *s_sck;
	struct t_htp_con   *c;      // new connection userdata

	lua_rawgeti( luaVM, LUA_REGISTRYINDEX, s->sR );
	t_stackDump( luaVM );
	s_sck = t_sck_check_ud( luaVM, -1, 1 );

	lt_sck_accept( luaVM );  //S: ssck,csck,cip
	c_sck  = t_sck_check_ud( luaVM, -2, 1 );
	si_cli = t_ipx_check_ud( luaVM, -1, 1 );

	lua_pushcfunction( luaVM, lt_elp_addhandle ); //S: ssck,csck,cip,addhandle
	lua_rawgeti( luaVM, LUA_REGISTRYINDEX, s->lR );
	t_elp_check_ud( luaVM, -1, 1 );
	lua_pushvalue( luaVM, -3 );
	lua_pushboolean( luaVM, 1 );                  // yepp, that's for reading
	lua_pushcfunction( luaVM, t_htp_con_read );   //S: ssck,csck,cip,addhandle,csck,true
	c      = (struct t_htp_con *) lua_newuserdata( luaVM, sizeof( struct t_htp_con ) );
	c->aR  = luaL_ref( luaVM, LUA_REGISTRYINDEX );
	c->sR  = luaL_ref( luaVM, LUA_REGISTRYINDEX );
	c->rR  = s->rR;                // copy function reference
	c->fd  = c_sck->fd;   //S: ssck,csck,cip,read,csck,true,con

	luaL_getmetatable( luaVM, "T.Http.Connection" );
	lua_setmetatable( luaVM, -2 );
	lua_call( luaVM, 5, 0 );
	//TODO: Check if that returns true or false; if false resize loop
	return 1;
}


/**--------------------------------------------------------------------------
 * Puts the http server on a T.Loop to listen to incoming requests.
 * \param   luaVM     lua Virtual Machine.
 * \lparam  userdata  struct t_htp_srv.
 * \lreturn value from the buffer a packers position according to packer format.
 * \return  integer number of values left on the stack.
 *  -------------------------------------------------------------------------*/
static int
lt_htp_srv_listen( lua_State *luaVM )
{
	struct t_htp_srv   *s   = t_htp_srv_check_ud( luaVM, 1, 1 );
	struct t_sck       *sc  = NULL;
	struct sockaddr_in *ip  = NULL;

	// reuse socket:listen()
	t_sck_listen( luaVM, 2 );

	sc = t_sck_check_ud( luaVM, -2, 1 );
	ip = t_ipx_check_ud( luaVM, -1, 1 );
	s->aR = luaL_ref( luaVM, LUA_REGISTRYINDEX );
	lua_pushvalue( luaVM, -1 );
	s->sR = luaL_ref( luaVM, LUA_REGISTRYINDEX );

	// TODO: cheaper to reimplement functionality -> less overhead?
	lua_pushcfunction( luaVM, lt_elp_addhandle ); //S: srv,sc,addhandle
	lua_rawgeti( luaVM, LUA_REGISTRYINDEX, s->lR );
	t_elp_check_ud( luaVM, -1, 1 );
	lua_pushvalue( luaVM, -3 );                  /// push socket
	lua_pushboolean( luaVM, 1 );                 //S: srv,sc,addhandle,loop,sck,true
	lua_pushcfunction( luaVM, lt_htp_srv_accept );
	lua_pushvalue( luaVM, 1 );                  /// push server instance

	lua_call( luaVM, 5, 0 );
	//TODO: Check if that returns tru or false; if false resize loop
	return  2;
}


/**--------------------------------------------------------------------------
 * __tostring (print) representation of a packer instance.
 * \param   luaVM      The lua state.
 * \lparam  xt_pack    the packer instance user_data.
 * \lreturn string     formatted string representing packer.
 * \return  The number of results to be passed back to the calling Lua script.
 * --------------------------------------------------------------------------*/
static int lt_htp_srv__tostring( lua_State *luaVM )
{
	struct t_htp_srv *s = t_htp_srv_check_ud( luaVM, 1, 1 );

	lua_pushfstring( luaVM, "T.Http.Server: %p", s );
	return 1;
}


/**--------------------------------------------------------------------------
 * __len (#) representation of an instance.
 * \param   luaVM      The lua state.
 * \lparam  userdata   the instance user_data.
 * \lreturn string     formatted string representing the instance.
 * \return  The number of results to be passed back to the calling Lua script.
 * --------------------------------------------------------------------------*/
static int lt_htp_srv__len( lua_State *luaVM )
{
	//struct t_wsk *wsk = t_wsk_check_ud( luaVM, 1, 1 );
	//TODO: something meaningful here?
	lua_pushinteger( luaVM, 4 );
	return 1;
}



/**--------------------------------------------------------------------------
 * Reads a chunk from socket.  Called anytime socket returns from read.
 * \param   luaVM     lua Virtual Machine.
 * \lparam  userdata  struct t_htp_srv.
 * \param   pointer to the buffer to read from(already positioned).
 * \lreturn value from the buffer a packers position according to packer format.
 * \return  integer number of values left on the stack.
 *  -------------------------------------------------------------------------*/
static int
t_htp_con_read( lua_State *luaVM )
{
	struct t_htp_con *c = (struct t_htp_con *) luaL_checkudata( luaVM, 1, "T.Http.Connection" );
	char              buffer[ BUFSIZ ];
	int               rcvd;
	struct t_sck     *c_sck;

	lua_rawgeti( luaVM, LUA_REGISTRYINDEX, c->sR );
	c_sck = t_sck_check_ud( luaVM, -1, 1 );

	lua_rawgeti( luaVM, LUA_REGISTRYINDEX, c->rR );
	// TODO: Idea
	// WS is in a state -> empty, receiving, sending
	// negotiate to read into the buffer initially or into the luaL_Buffer
	rcvd = t_sck_recv_tdp( luaVM, c_sck, &(buffer[ 0 ]), BUFSIZ );
	printf( "%s\n", buffer);
	return rcvd;
}



/**--------------------------------------------------------------------------
 * \brief    the metatble for the module
 * --------------------------------------------------------------------------*/
static const struct luaL_Reg t_htp_srv_fm [] = {
	{"__call",        lt_htp_srv__Call},
	{NULL,            NULL}
};


/**--------------------------------------------------------------------------
 * \brief    the metatble for the module
 * --------------------------------------------------------------------------*/
static const struct luaL_Reg t_htp_srv_cf [] = {
	{"new",           lt_htp_srv_New},
	{NULL,            NULL}
};


/**--------------------------------------------------------------------------
 * \brief      the buffer library definition
 *             assigns Lua available names to C-functions
 * --------------------------------------------------------------------------*/
static const luaL_Reg t_htp_srv_m [] = {
	{"listen",        lt_htp_srv_listen},
	{NULL,    NULL}
};


/**--------------------------------------------------------------------------
 * \brief   pushes this library onto the stack
 *          - creates Metatable with functions
 *          - creates metatable with methods
 * \param   luaVM     The lua state.
 * \lreturn string    the library
 * \return  The number of results to be passed back to the calling Lua script.
 * --------------------------------------------------------------------------*/
LUAMOD_API int luaopen_t_htp_srv( lua_State *luaVM )
{
	// T.Http.Server instance metatable
	luaL_newmetatable( luaVM, "T.Http.Server" );
	luaL_newlib( luaVM, t_htp_srv_m );
	lua_setfield( luaVM, -2, "__index" );
	lua_pushcfunction( luaVM, lt_htp_srv__len );
	lua_setfield( luaVM, -2, "__len");
	lua_pushcfunction( luaVM, lt_htp_srv__tostring );
	lua_setfield( luaVM, -2, "__tostring");
	lua_pop( luaVM, 1 );        // remove metatable from stack
	// T.Websocket class
	luaL_newlib( luaVM, t_htp_srv_cf );
	luaL_newlib( luaVM, t_htp_srv_fm );
	lua_setmetatable( luaVM, -2 );
	return 1;
}


/**
 * \brief      the (empty) t library definition
 *             assigns Lua available names to C-functions
 */
static const luaL_Reg t_htp_lib [] =
{
	//{"crypt",     t_enc_crypt},
	{NULL,        NULL}
};


/**
 * \brief     Export the t_htp libray to Lua
 * \param      The Lua state.
 * \return     1 return value
 */
LUAMOD_API int
luaopen_t_htp( lua_State *luaVM )
{
	luaL_newlib( luaVM, t_htp_lib );
	luaopen_t_htp_srv( luaVM );
	lua_setfield( luaVM, -2, "Server" );
	return 1;
}
