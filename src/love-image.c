#include "raylove.h"

// love.graphics: images, canvases, quads

static int g_newImage(lua_State *l)
{
	const char *path = luaL_checkstring(l, 1);
	LImage *im = (LImage *)lua_newuserdata(l, sizeof(LImage));
	memset(im, 0, sizeof(LImage));
	im->tex = LoadTexture(path);
	if (im->tex.id == 0)
	{
		return luaL_error(l, "could not load image: %s", path);
	}
	im->w = im->tex.width;
	im->h = im->tex.height;
	apply_filter(im->tex, defaultFilterPoint);
	luaL_getmetatable(l, "love.Image");
	lua_setmetatable(l, -2);
	return 1;
}

static int g_newCanvas(lua_State *l)
{
	int w = (int)luaL_optnumber(l, 1, VIRT_W);
	int h = (int)luaL_optnumber(l, 2, VIRT_H);
	LImage *im = (LImage *)lua_newuserdata(l, sizeof(LImage));
	memset(im, 0, sizeof(LImage));
	im->rt = LoadRenderTexture(w, h);
	if (im->rt.id == 0)
	{
		return luaL_error(l, "could not create canvas %dx%d", w, h);
	}
	im->tex = im->rt.texture;
	im->canvas = 1;
	im->w = w;
	im->h = h;
	apply_filter(im->tex, defaultFilterPoint);
	luaL_getmetatable(l, "love.Image");
	lua_setmetatable(l, -2);
	return 1;
}

static int g_setCanvas(lua_State *l)
{
	LImage *im = NULL;
	if (lua_istable(l, 1))
	{
		lua_rawgeti(l, 1, 1);
		im = (LImage *)test_udata(l, -1, "love.Image");
		lua_pop(l, 1);
	}
	else if (!lua_isnoneornil(l, 1))
	{
		im = (LImage *)luaL_checkudata(l, 1, "love.Image");
	}
	if (im && !im->canvas) return luaL_error(l, "setCanvas: not a canvas");
	leave_current();
	if (im)
	{
		BeginTextureMode(im->rt);
		activeCanvas = im;
	}
	else
	{
		enter_screen();
	}
	return 0;
}

static int img_getWidth(lua_State *l)
{
	lua_pushinteger(l, ((LImage *)luaL_checkudata(l, 1, "love.Image"))->w);
	return 1;
}

static int img_getHeight(lua_State *l)
{
	lua_pushinteger(l, ((LImage *)luaL_checkudata(l, 1, "love.Image"))->h);
	return 1;
}

static int img_getDimensions(lua_State *l)
{
	LImage *im = (LImage *)luaL_checkudata(l, 1, "love.Image");
	lua_pushinteger(l, im->w);
	lua_pushinteger(l, im->h);
	return 2;
}

static int img_setFilter(lua_State *l)
{
	LImage *im = (LImage *)luaL_checkudata(l, 1, "love.Image");
	apply_filter(im->tex, strcmp(luaL_optstring(l, 2, "linear"), "nearest") == 0);
	return 0;
}

static int img_gc(lua_State *l)
{
	LImage *im = (LImage *)luaL_checkudata(l, 1, "love.Image");
	if (im->canvas)
	{
		if (im->rt.id) UnloadRenderTexture(im->rt);
	}
	else if (im->tex.id)
	{
		UnloadTexture(im->tex);
	}
	im->tex.id = 0;
	im->rt.id = 0;
	return 0;
}

static int g_newQuad(lua_State *l)
{
	Rectangle *q = (Rectangle *)lua_newuserdata(l, sizeof(Rectangle));
	q->x = (float)luaL_checknumber(l, 1);
	q->y = (float)luaL_checknumber(l, 2);
	q->width = (float)luaL_checknumber(l, 3);
	q->height = (float)luaL_checknumber(l, 4);
	luaL_getmetatable(l, "love.Quad");
	lua_setmetatable(l, -2);
	return 1;
}

// draw(drawable [, quad], x, y, r, sx, sy, ox, oy)
static int g_draw(lua_State *l)
{
	LImage *im = (LImage *)luaL_checkudata(l, 1, "love.Image");
	int a = 2;
	Rectangle src = {0, 0, (float)im->w, (float)im->h};
	Rectangle *q = (Rectangle *)test_udata(l, 2, "love.Quad");
	if (q)
	{
		src = *q;
		a = 3;
	}

	float x = (float)luaL_optnumber(l, a, 0);
	float y = (float)luaL_optnumber(l, a + 1, 0);
	float r = (float)luaL_optnumber(l, a + 2, 0);
	float sx = (float)luaL_optnumber(l, a + 3, 1);
	float sy = (float)luaL_optnumber(l, a + 4, sx);
	float ox = (float)luaL_optnumber(l, a + 5, 0);
	float oy = (float)luaL_optnumber(l, a + 6, 0);

	float w = src.width, h = src.height;
	Rectangle s = src;
	if (im->canvas)
	{
		// GL textures are upside down
		s.y = (float)im->h - s.y - h;
		s.height = -h;
	}
	if (sx < 0)
	{
		s.width = -s.width;
		sx = -sx;
		ox = w - ox;
	}
	if (sy < 0)
	{
		s.height = -s.height;
		sy = -sy;
		oy = h - oy;
	}

	Rectangle dst = {x, y, w * sx, h * sy};
	DrawTexturePro(im->tex, s, dst, (Vector2){ox * sx, oy * sy}, r * RAD2DEG, curColor);
	return 0;
}

const luaL_Reg graphics_image_funcs[] =
{
	{"newImage", g_newImage},
	{"newCanvas", g_newCanvas},
	{"setCanvas", g_setCanvas},
	{"newQuad", g_newQuad},
	{"draw", g_draw},
	{NULL, NULL}
};

const luaL_Reg image_methods[] =
{
	{"getWidth", img_getWidth},
	{"getHeight", img_getHeight},
	{"getDimensions", img_getDimensions},
	{"setFilter", img_setFilter},
	{"__gc", img_gc},
	{NULL, NULL}
};
