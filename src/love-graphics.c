#include "raylove.h"

// Render target management (virtual resolution + canvas)

void leave_current(void)
{
	if (activeCanvas)
	{
		EndTextureMode();
		activeCanvas = NULL;
	}
	else if (inVscreen)
	{
		EndTextureMode();
		inVscreen = 0;
	}
}

void enter_screen(void)
{
	if (frameActive && USE_VIRT)
	{
		BeginTextureMode(vscreen);
		inVscreen = 1;
	}
}

// love.graphics: state, primitives, transforms

static int g_setColor(lua_State *l)
{
	double c[4] = {1, 1, 1, 1};
	if (lua_istable(l, 1))
	{
		for (int i = 0; i < 4; i++)
		{
			lua_rawgeti(l, 1, i + 1);
			if (lua_isnumber(l, -1))
				c[i] = lua_tonumber(l, -1);
			lua_pop(l, 1);
		}
	}
	else
	{
		c[0] = luaL_checknumber(l, 1);
		c[1] = luaL_checknumber(l, 2);
		c[2] = luaL_checknumber(l, 3);
		c[3] = luaL_optnumber(l, 4, 1);
	}
	curColor = (Color){f2b(c[0]), f2b(c[1]), f2b(c[2]), f2b(c[3])};
	return 0;
}

static int g_setBackgroundColor(lua_State *l)
{
	bgR = luaL_checknumber(l, 1);
	bgG = luaL_checknumber(l, 2);
	bgB = luaL_checknumber(l, 3);
	bgA = luaL_optnumber(l, 4, 1);
	return 0;
}

static int g_getBackgroundColor(lua_State *l)
{
	lua_pushnumber(l, bgR);
	lua_pushnumber(l, bgG);
	lua_pushnumber(l, bgB);
	lua_pushnumber(l, bgA);
	return 4;
}

static int g_clear(lua_State *l)
{
	double r = bgR, g = bgG, b = bgB, a = bgA;
	if (lua_isnumber(l, 1))
	{
		r = lua_tonumber(l, 1);
		g = luaL_optnumber(l, 2, 0);
		b = luaL_optnumber(l, 3, 0);
		a = luaL_optnumber(l, 4, 1);
	}
	ClearBackground((Color){f2b(r), f2b(g), f2b(b), f2b(a)});
	return 0;
}

static int g_rectangle(lua_State *l)
{
	const char *mode = luaL_checkstring(l, 1);
	float x = (float)luaL_checknumber(l, 2), y = (float)luaL_checknumber(l, 3);
	float w = (float)luaL_checknumber(l, 4), h = (float)luaL_checknumber(l, 5);
	if (mode[0] == 'f')
		DrawRectangleRec((Rectangle){x, y, w, h}, curColor);
	else
		DrawRectangleLinesEx((Rectangle){x, y, w, h}, lineWidth, curColor);
	return 0;
}

static int g_circle(lua_State *l)
{
	const char *mode = luaL_checkstring(l, 1);
	float x = (float)luaL_checknumber(l, 2), y = (float)luaL_checknumber(l, 3);
	float r = (float)luaL_checknumber(l, 4);
	if (mode[0] == 'f')
		DrawCircle((int)x, (int)y, r, curColor);
	else
		DrawCircleLines((int)x, (int)y, r, curColor);
	return 0;
}

static int g_line(lua_State *l)
{
	int n = lua_gettop(l);
	for (int i = 1; i + 3 <= n; i += 2)
	{
		Vector2 a = {(float)luaL_checknumber(l, i), (float)luaL_checknumber(l, i + 1)};
		Vector2 b = {(float)luaL_checknumber(l, i + 2), (float)luaL_checknumber(l, i + 3)};
		DrawLineEx(a, b, lineWidth, curColor);
	}
	return 0;
}

static int g_points(lua_State *l)
{
	int n = lua_gettop(l);
	for (int i = 1; i + 1 <= n; i += 2)
	{
		DrawRectangle((int)luaL_checknumber(l, i), (int)luaL_checknumber(l, i + 1), 1, 1, curColor);
	}
	return 0;
}

static int g_setLineWidth(lua_State *l)
{
	lineWidth = (float)luaL_checknumber(l, 1);
	return 0;
}

static int g_setBlendMode(lua_State *l)
{
	const char *m = luaL_checkstring(l, 1);
	const char *a = luaL_optstring(l, 2, "alphamultiply");
	int mode = BLEND_ALPHA;
	if (!strcmp(m, "add"))
		mode = BLEND_ADDITIVE;
	else if (!strcmp(m, "subtract"))
		mode = BLEND_SUBTRACT_COLORS;
	else if (!strcmp(m, "multiply"))
		mode = BLEND_MULTIPLIED;
	else if (!strcmp(m, "alpha") && !strcmp(a, "premultiplied"))
		mode = BLEND_ALPHA_PREMULTIPLY;
	BeginBlendMode(mode);
	return 0;
}

static int g_setDefaultFilter(lua_State *l)
{
	const char *f = luaL_optstring(l, 1, "linear");
	defaultFilterPoint = (strcmp(f, "nearest") == 0);
	return 0;
}

static int g_origin(lua_State *l)
{
	(void)l;
	rlLoadIdentity();
	return 0;
}

static int g_present(lua_State *l)
{
	(void)l;
	return 0;
}

static int g_isActive(lua_State *l)
{
	lua_pushboolean(l, 1);
	return 1;
}

int g_noop(lua_State *l)
{
	(void)l;
	return 0;
}

static int g_push(lua_State *l)
{
	(void)l;
	rlPushMatrix();
	return 0;
}

static int g_pop(lua_State *l)
{
	(void)l;
	rlPopMatrix();
	return 0;
}

static int g_translate(lua_State *l)
{
	rlTranslatef((float)luaL_checknumber(l, 1), (float)luaL_checknumber(l, 2), 0);
	return 0;
}

static int g_scale(lua_State *l)
{
	float sx = (float)luaL_checknumber(l, 1);
	rlScalef(sx, (float)luaL_optnumber(l, 2, sx), 1);
	return 0;
}

static int g_rotate(lua_State *l)
{
	rlRotatef((float)luaL_checknumber(l, 1) * RAD2DEG, 0, 0, 1);
	return 0;
}

static int g_getWidth(lua_State *l)
{
	lua_pushinteger(l, VIRT_W);
	return 1;
}

static int g_getHeight(lua_State *l)
{
	lua_pushinteger(l, VIRT_H);
	return 1;
}

static int g_getDimensions(lua_State *l)
{
	lua_pushinteger(l, VIRT_W);
	lua_pushinteger(l, VIRT_H);
	return 2;
}

const luaL_Reg graphics_funcs[] =
{
	{"setColor", g_setColor},
	{"setBackgroundColor", g_setBackgroundColor},
	{"getBackgroundColor", g_getBackgroundColor},
	{"clear", g_clear},
	{"rectangle", g_rectangle},
	{"circle", g_circle},
	{"line", g_line},
	{"points", g_points},
	{"setLineWidth", g_setLineWidth},
	{"setBlendMode", g_setBlendMode},
	{"setDefaultFilter", g_setDefaultFilter},
	{"origin", g_origin},
	{"present", g_present},
	{"isActive", g_isActive},
	{"push", g_push},
	{"pop", g_pop},
	{"translate", g_translate},
	{"scale", g_scale},
	{"rotate", g_rotate},
	{"getWidth", g_getWidth},
	{"getHeight", g_getHeight},
	{"getDimensions", g_getDimensions},
	{"setScissor", g_noop},
	{"setLineStyle", g_noop},
	{NULL, NULL}
};
