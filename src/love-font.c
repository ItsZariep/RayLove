#include "raylove.h"

// love.graphics: fonts and text

static int g_newFont(lua_State *l)
{
	FontObj *f = (FontObj *)lua_newuserdata(l, sizeof(FontObj));
	memset(f, 0, sizeof(FontObj));
	if (lua_type(l, 1) == LUA_TSTRING)
	{
		const char *path = lua_tostring(l, 1);
		int size = (int)luaL_optnumber(l, 2, 12);
		f->font = LoadFontEx(path, size, NULL, 0);
		f->size = (float)size;
		f->owned = (f->font.texture.id != GetFontDefault().texture.id);
		if (!f->owned)
			TraceLog(LOG_WARNING, "font %s failed to load, using default", path);
	}
	else
	{
		f->font = GetFontDefault();
		f->size = (float)luaL_optnumber(l, 1, 12);
		f->owned = 0;
	}
	luaL_getmetatable(l, "love.Font");
	lua_setmetatable(l, -2);
	return 1;
}

static int g_setFont(lua_State *l)
{
	FontObj *f = (FontObj *)luaL_checkudata(l, 1, "love.Font");
	if ((void *)f == curFontPtr)
	{
		return 0;
	}
	if (curFontRef != LUA_NOREF)
	{
		luaL_unref(l, LUA_REGISTRYINDEX, curFontRef);
	}
	lua_pushvalue(l, 1);
	curFontRef = luaL_ref(l, LUA_REGISTRYINDEX);
	curFontPtr = (void *)f;
	curFont = f->font;
	curFontSize = f->size;
	return 0;
}

static int font_getWidth(lua_State *l)
{
	FontObj *f = (FontObj *)luaL_checkudata(l, 1, "love.Font");
	lua_pushnumber(l, MeasureTextEx(f->font, luaL_checkstring(l, 2), f->size, 0).x);
	return 1;
}

static int font_getHeight(lua_State *l)
{
	FontObj *f = (FontObj *)luaL_checkudata(l, 1, "love.Font");
	lua_pushnumber(l, f->size);
	return 1;
}

static int font_setFilter(lua_State *l)
{
	FontObj *f = (FontObj *)luaL_checkudata(l, 1, "love.Font");
	apply_filter(f->font.texture, strcmp(luaL_optstring(l, 2, "linear"), "nearest") == 0);
	return 0;
}

static int font_gc(lua_State *l)
{
	FontObj *f = (FontObj *)luaL_checkudata(l, 1, "love.Font");
	if (f->owned)
	{
		UnloadFont(f->font);
		f->owned = 0;
	}
	return 0;
}

// print(text, x, y, r, sx, sy)
static int g_print(lua_State *l)
{
	const char *text = luaL_checkstring(l, 1);
	float x  = (float)luaL_optnumber(l, 2, 0);
	float y  = (float)luaL_optnumber(l, 3, 0);
	float r  = (float)luaL_optnumber(l, 4, 0);
	float sx = (float)luaL_optnumber(l, 5, 1);
	float sy = (float)luaL_optnumber(l, 6, sx);
	if (r == 0 && sx == sy)
	{
		// fast path: no matrix push (menus draw outlined text many times per frame)
		DrawTextEx(curFont, text, (Vector2){x, y}, curFontSize * sx, 0, curColor);
	}
	else
	{
		rlPushMatrix();
		rlTranslatef(x, y, 0);
		if (r != 0)
		{
			rlRotatef(r * RAD2DEG, 0, 0, 1);
		}
		rlScalef(sx, sy, 1);
		DrawTextEx(curFont, text, (Vector2){0, 0}, curFontSize, 0, curColor);
		rlPopMatrix();
	}
	return 0;
}

const luaL_Reg graphics_font_funcs[] =
{
	{"newFont", g_newFont},
	{"setFont", g_setFont},
	{"print", g_print},
	{NULL, NULL}
};

const luaL_Reg font_methods[] =
{
	{"getWidth", font_getWidth},
	{"getHeight", font_getHeight},
	{"setFilter", font_setFilter},
	{"__gc", font_gc},
	{NULL, NULL}
};
