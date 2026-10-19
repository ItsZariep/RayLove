#ifndef RAYLOVE_H
#define RAYLOVE_H

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "raylib.h"
#include "rlgl.h"

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#ifndef SCREEN_W
#define SCREEN_W 240
#endif
#ifndef SCREEN_H
#define SCREEN_H 320
#endif
#ifndef VIRT_W
#define VIRT_W SCREEN_W
#endif
#ifndef VIRT_H
#define VIRT_H SCREEN_H
#endif
#define USE_VIRT ((VIRT_W) != (SCREEN_W) || (VIRT_H) != (SCREEN_H))

#ifndef PRELUDE_PATH
#define PRELUDE_PATH "/raylove/prelude.lua"
#endif
#ifndef GAME_DIR
#define GAME_DIR "/assets"
#endif

#define MAX_INST 3
#define MAX_ACTIVE_MUSIC 8

// Types

typedef struct
{
	Texture2D tex;
	RenderTexture2D rt;
	int canvas;
	int w, h;
} LImage;

typedef struct
{
	Font font;
	float size;
	int owned;
} FontObj;

typedef struct SoundBank
{
	Wave wave;
	Sound inst[MAX_INST];
	int n;
	int refs;
} SoundBank;

typedef struct
{
	SoundBank *bank;
	Music *music;
	float pitch, volume, appliedPitch;
	int cur;
} Source;

// Shared state (defined in common.c)

extern lua_State *L;
extern char errmsg[1024];

extern Color curColor;
extern double bgR, bgG, bgB, bgA;
extern float lineWidth;
extern int defaultFilterPoint;

extern RenderTexture2D vscreen;
extern int inVscreen;
extern LImage *activeCanvas;
extern int frameActive;

extern Font curFont;
extern float curFontSize;
extern int curFontRef;
extern void *curFontPtr;

// common.c

unsigned char f2b(double v);
void *test_udata(lua_State *l, int idx, const char *tname);
void set_error(const char *where, const char *msg);
void apply_filter(Texture2D t, int point);

// love-graphics.c

void leave_current(void);
void enter_screen(void);
int g_noop(lua_State *l);

// love-keyboard.c

void init_keymap(void);
void push_key_events(void);

// love-audio.c

void update_music(void);

// registration.c

void register_love(void);

// Function tables (each defined in its module, consumed by registration.c)

extern const luaL_Reg graphics_funcs[];       // love-graphics.c
extern const luaL_Reg graphics_image_funcs[]; // love-image.c
extern const luaL_Reg graphics_font_funcs[];  // love-font.c
extern const luaL_Reg image_methods[];        // love-image.c
extern const luaL_Reg font_methods[];         // love-font.c
extern const luaL_Reg keyboard_funcs[];       // love-keyboard.c
extern const luaL_Reg timer_funcs[];          // love-timer.c
extern const luaL_Reg filesystem_funcs[];     // love-filesystem.c
extern const luaL_Reg audio_funcs[];          // love-audio.c
extern const luaL_Reg source_methods[];       // love-audio.c

#endif
