#include "raylove.h"

// Shared state

lua_State *L;
char errmsg[1024];

Color curColor = {255, 255, 255, 255};
double bgR = 0, bgG = 0, bgB = 0, bgA = 1;
float lineWidth = 1.0f;
int defaultFilterPoint = 0;

RenderTexture2D vscreen;
int inVscreen = 0;
LImage *activeCanvas = NULL;
int frameActive = 0;

Font curFont;
float curFontSize = 10;
int curFontRef = LUA_NOREF;
void *curFontPtr = NULL;

// Helpers

unsigned char f2b(double v)
{
	if (v < 0) v = 0;
	if (v > 1) v = 1;
	return (unsigned char)(v * 255.0 + 0.5);
}

void *test_udata(lua_State *l, int idx, const char *tname)
{
	void *p = lua_touserdata(l, idx);
	if (p && lua_getmetatable(l, idx))
	{
		luaL_getmetatable(l, tname);
		if (lua_rawequal(l, -1, -2))
		{
			lua_pop(l, 2);
			return p;
		}
		lua_pop(l, 2);
	}
	return NULL;
}

void set_error(const char *where, const char *msg)
{
	if (errmsg[0]) return; // keep the first error
	snprintf(errmsg, sizeof(errmsg), "%s: %s", where, msg ? msg : "(unknown)");
	TraceLog(LOG_ERROR, "%s", errmsg);
}

void apply_filter(Texture2D t, int point)
{
	SetTextureFilter(t, point ? TEXTURE_FILTER_POINT : TEXTURE_FILTER_BILINEAR);
}
