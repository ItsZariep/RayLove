#include "raylove.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

// Main loop

static int loopRef = LUA_NOREF;
static int quitRequested = 0;

static void draw_error(void)
{
	ClearBackground((Color){90, 0, 0, 255});
	DrawText("RAYLOVE ERROR", 4, 4, 10, YELLOW);
	const char *p = errmsg;
	int row = 0;
	while (*p && row < 24)
	{
		char line[40];
		int n = 0;
		while (*p && *p != '\n' && n < 37)
		{
			line[n++] = *p++;
		}
		if (*p == '\n') p++;
		line[n] = 0;
		DrawText(line, 4, 20 + row * 12, 10, WHITE);
		row++;
	}
}

static void do_quit(void)
{
#if defined(PLATFORM_WEB)
	emscripten_cancel_main_loop();
	emscripten_run_script("try{window.close();}catch(e){}");
#else
	quitRequested = 2;
#endif
}

static void frame(void)
{
	update_music();
	if (!errmsg[0])
		push_key_events();

	frameActive = 1;
	BeginDrawing();
	BeginBlendMode(BLEND_ALPHA);

	if (errmsg[0])
	{
		draw_error();
	}
	else
	{
		enter_screen();
		lua_rawgeti(L, LUA_REGISTRYINDEX, loopRef);
		if (lua_pcall(L, 0, 1, 0) != 0)
		{
			set_error("love.run", lua_tostring(L, -1));
			lua_pop(L, 1);
		}
		else
		{
			if (!lua_isnil(L, -1))
				quitRequested = 1;
			lua_pop(L, 1);
		}
		leave_current();
		if (USE_VIRT)
		{
			BeginBlendMode(BLEND_ALPHA);
			DrawTexturePro(vscreen.texture,
				(Rectangle){0, 0, (float)VIRT_W, -(float)VIRT_H},
				(Rectangle){0, 0, (float)SCREEN_W, (float)SCREEN_H},
				(Vector2){0, 0}, 0, WHITE);
		}
	}

	EndDrawing();
	frameActive = 0;
	if (quitRequested == 1) do_quit();
}

static void run_file(const char *path)
{
	if (luaL_dofile(L, path) != 0)
	{
		set_error(path, lua_tostring(L, -1));
		lua_pop(L, 1);
	}
}

int main(int argc, char **argv)
{
	InitWindow(SCREEN_W, SCREEN_H, "Raylove");
	SetExitKey(KEY_NULL);
	InitAudioDevice();
	if (USE_VIRT)
	{
		vscreen = LoadRenderTexture(VIRT_W, VIRT_H);
		SetTextureFilter(vscreen.texture, TEXTURE_FILTER_BILINEAR);
	}
	curFont = GetFontDefault();
	init_keymap();

	L = luaL_newstate();
	luaL_openlibs(L);
	register_love();

	if (argc > 1)
		chdir(argv[1]);
	else
		chdir(GAME_DIR);

	run_file(PRELUDE_PATH);
	if (!errmsg[0])
	{
		if (luaL_dostring(L, "love._loadconf()") != 0)
		{
			set_error("conf.lua", lua_tostring(L, -1));
			lua_pop(L, 1);
		}
	}

	if (!errmsg[0])
		run_file("main.lua");

	if (!errmsg[0])
	{
		lua_getglobal(L, "love");
		lua_getfield(L, -1, "run");
		if (lua_pcall(L, 0, 1, 0) != 0)
		{
			// runs love.load
			set_error("love.load", lua_tostring(L, -1));
			lua_pop(L, 1);
		}
		else if (lua_isfunction(L, -1))
		{
			loopRef = luaL_ref(L, LUA_REGISTRYINDEX);
		}
		else
		{
			set_error("love.run", "did not return a function");
			lua_pop(L, 1);
		}
		lua_pop(L, 1); // love table
	}

#if defined(PLATFORM_WEB)
	emscripten_set_main_loop(frame, 0, 1);
#else
	SetTargetFPS(60);
	while (!WindowShouldClose() && quitRequested != 2)
		frame();
#endif

	lua_close(L);
	CloseAudioDevice();
	CloseWindow();
	return 0;
}
