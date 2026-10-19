#include "raylove.h"

// love.filesystem

static int fs_load(lua_State *l)
{
	const char *path = luaL_checkstring(l, 1);
	int size = 0;
	unsigned char *data = LoadFileData(path, (void *)&size);
	if (!data)
	{
		lua_pushnil(l);
		lua_pushfstring(l, "could not read file %s", path);
		return 2;
	}
	int rc = luaL_loadbuffer(l, (const char *)data, (size_t)size, path);
	UnloadFileData(data);
	if (rc != 0)
	{
		lua_pushnil(l);
		lua_insert(l, -2); // nil, errmsg
		return 2;
	}
	return 1;
}

static int fs_read(lua_State *l)
{
	const char *path = luaL_checkstring(l, 1);
	int size = 0;
	unsigned char *data = LoadFileData(path, (void *)&size);
	if (!data)
	{
		lua_pushnil(l);
		lua_pushfstring(l, "could not read file %s", path);
		return 2;
	}
	lua_pushlstring(l, (const char *)data, (size_t)size);
	UnloadFileData(data);
	lua_pushinteger(l, size);
	return 2;
}

static int fs_getInfo(lua_State *l)
{
	const char *p = luaL_checkstring(l, 1);
	if (DirectoryExists(p))
	{
		lua_newtable(l);
		lua_pushstring(l, "directory");
		lua_setfield(l, -2, "type");
		return 1;
	}
	if (FileExists(p))
	{
		lua_newtable(l);
		lua_pushstring(l, "file");
		lua_setfield(l, -2, "type");
		lua_pushinteger(l, GetFileLength(p));
		lua_setfield(l, -2, "size");
		return 1;
	}
	lua_pushnil(l);
	return 1;
}

static int fs_write(lua_State *l)
{
	const char *path = luaL_checkstring(l, 1);
	size_t len;
	const char *data = luaL_checklstring(l, 2, &len);
	if (SaveFileData(path, (void *)data, (unsigned int)len))
	{
		lua_pushboolean(l, 1);
		return 1;
	}
	lua_pushnil(l);
	lua_pushstring(l, "could not write file");
	return 2;
}

const luaL_Reg filesystem_funcs[] =
{
	{"read", fs_read},
	{"load", fs_load},
	{"getInfo", fs_getInfo},
	{"write", fs_write},
	{NULL, NULL}
};
