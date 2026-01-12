#include <stdio.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

int add_function(lua_State *L)
{
	int a = luaL_checkinteger(L, 1);
	int b = luaL_checkinteger(L, 2);
	lua_pushinteger(L, a + b);
	return 1; // return one value
}

int main(void)
{
	lua_State *L = luaL_newstate();  // create the Lua state
	luaL_openlibs(L);                // open standard libraries

	if (luaL_dostring(L, "print('Hello from Lua!')"))
		fprintf(stderr, "Lua error: %s\n", lua_tostring(L, -1));

	lua_register(L, "add", add_function);

	if (luaL_dostring(L, "print(add(10,15))"))
		fprintf(stderr, "Lua error: %s\n", lua_tostring(L, -1));

	lua_close(L);
	return 0;
}
