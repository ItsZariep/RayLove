#include "raylove.h"

// love.timer

static double lastStepTime = 0, lastDt = 0;
static int stepStarted = 0;

static int t_getTime(lua_State *l)
{
	lua_pushnumber(l, GetTime());
	return 1;
}

static int t_getDelta(lua_State *l)
{
	lua_pushnumber(l, lastDt);
	return 1;
}

static int t_getFPS(lua_State *l)
{
	lua_pushinteger(l, GetFPS());
	return 1;
}

static int t_sleep(lua_State *l)
{
	(void)l;
	return 0;
}

static int t_step(lua_State *l)
{
	double now = GetTime();
	double dt = stepStarted ? now - lastStepTime : 0;
	stepStarted = 1;
	lastStepTime = now;
	if (dt > 0.05) dt = 0.05; // cap: a slow device must not spiral into more and more updates
	if (dt < 0) dt = 0;
	lastDt = dt;
	lua_pushnumber(l, dt);
	return 1;
}

const luaL_Reg timer_funcs[] =
{
	{"getTime", t_getTime},
	{"getDelta", t_getDelta},
	{"getFPS", t_getFPS},
	{"sleep", t_sleep},
	{"step", t_step},
	{NULL, NULL}
};
