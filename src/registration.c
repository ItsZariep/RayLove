#include "raylove.h"

// Registration: builds the global `love` table and the userdata metatables

static void register_sub(const char *name, const luaL_Reg *funcs)
{
	lua_newtable(L);
	luaL_setfuncs(L, funcs, 0);
	lua_setfield(L, -2, name);
}

static void register_type(const char *tname, const luaL_Reg *methods)
{
	luaL_newmetatable(L, tname);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, methods, 0);
	lua_pop(L, 1);
}

void register_love(void)
{
	lua_newtable(L);

	// love.graphics is assembled from several modules
	lua_newtable(L);
	luaL_setfuncs(L, graphics_funcs, 0);
	luaL_setfuncs(L, graphics_image_funcs, 0);
	luaL_setfuncs(L, graphics_font_funcs, 0);
	lua_setfield(L, -2, "graphics");

	register_sub("keyboard", keyboard_funcs);
	register_sub("timer", timer_funcs);
	register_sub("filesystem", filesystem_funcs);
	register_sub("audio", audio_funcs);
	lua_setglobal(L, "love");

	register_type("love.Image", image_methods);
	register_type("love.Font", font_methods);
	register_type("love.Source", source_methods);
	luaL_newmetatable(L, "love.Quad");
	lua_pop(L, 1);
}
