#include "raylove.h"

// love.keyboard

typedef struct
{
	const char *name;
	int k1, k2;
} KeyMap;

static KeyMap keymap[80];
static int nkeys = 0;
static char nameBuf[36][2];

static void addkey(const char *n, int k1, int k2)
{
	keymap[nkeys].name = n;
	keymap[nkeys].k1 = k1;
	keymap[nkeys].k2 = k2;
	nkeys++;
}

void init_keymap(void)
{
	for (int i = 0; i < 26; i++)
	{
		nameBuf[i][0] = (char)('a' + i);
		nameBuf[i][1] = 0;
		addkey(nameBuf[i], KEY_A + i, 0);
	}
	for (int i = 0; i < 10; i++)
	{
		nameBuf[26 + i][0] = (char)('0' + i);
		nameBuf[26 + i][1] = 0;
		addkey(nameBuf[26 + i], KEY_ZERO + i, 0);
	}
	addkey("up", KEY_UP, 0);
	addkey("down", KEY_DOWN, 0);
	addkey("left", KEY_LEFT, 0);
	addkey("right", KEY_RIGHT, 0);
	addkey("space", KEY_SPACE, 0);
	addkey("return", KEY_ENTER, KEY_KP_ENTER);
	addkey("escape", KEY_ESCAPE, KEY_BACKSPACE);
	addkey("backspace", KEY_BACKSPACE, 0);
	addkey("f1", KEY_F1, 0);
	addkey("f2", KEY_F2, 0);
}

static int key_down(const KeyMap *k)
{
	return IsKeyDown(k->k1) || (k->k2 && IsKeyDown(k->k2));
}

static int key_pressed(const KeyMap *k)
{
	return IsKeyPressed(k->k1) || (k->k2 && IsKeyPressed(k->k2));
}

static int key_released(const KeyMap *k)
{
	return IsKeyReleased(k->k1) || (k->k2 && IsKeyReleased(k->k2));
}

static int kb_isDown(lua_State *l)
{
	int n = lua_gettop(l);
	for (int i = 1; i <= n; i++)
	{
		const char *s = luaL_checkstring(l, i);
		for (int k = 0; k < nkeys; k++)
		{
			if (strcmp(s, keymap[k].name) == 0 && key_down(&keymap[k]))
			{
				lua_pushboolean(l, 1);
				return 1;
			}
		}
	}
	lua_pushboolean(l, 0);
	return 1;
}

const luaL_Reg keyboard_funcs[] =
{
	{"isDown", kb_isDown},
	{"isScancodeDown", kb_isDown},
	{NULL, NULL}
};

// Event pushing (love.event.push is implemented in the Lua prelude)

static void push_event(const char *evt, const char *key)
{
	lua_getglobal(L, "love");
	lua_getfield(L, -1, "event");
	lua_getfield(L, -1, "push");
	lua_pushstring(L, evt);
	lua_pushstring(L, key);
	lua_pushstring(L, key);
	lua_pushboolean(L, 0);
	if (lua_pcall(L, 4, 0, 0) != 0) lua_pop(L, 1);
	lua_pop(L, 2);
}

void push_key_events(void)
{
	for (int k = 0; k < nkeys; k++)
	{
		if (key_pressed(&keymap[k])) push_event("keypressed", keymap[k].name);
		if (key_released(&keymap[k])) push_event("keyreleased", keymap[k].name);
	}
}
