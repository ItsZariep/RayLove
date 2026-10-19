#include "raylove.h"

// love.audio

static Music *activeMusic[MAX_ACTIVE_MUSIC];
static int nActive = 0;

static const char *resolve_audio(const char *path, char *buf, size_t n)
{
	if (FileExists(path)) return path;
	const char *dot = strrchr(path, '.');
	size_t len = dot ? (size_t)(dot - path) : strlen(path);
	if (len + 5 < n)
	{
		memcpy(buf, path, len);
		strcpy(buf + len, ".ogg");
		if (FileExists(buf)) return buf;
	}
	return NULL;
}

static void music_add(Music *m)
{
	for (int i = 0; i < nActive; i++)
	{
		if (activeMusic[i] == m)
			return;
	}
	if (nActive < MAX_ACTIVE_MUSIC) activeMusic[nActive++] = m;
}

static void music_remove(Music *m)
{
	for (int i = 0; i < nActive; i++)
	{
		if (activeMusic[i] == m)
		{
			activeMusic[i] = activeMusic[--nActive];
			return;
		}
	}
}

void update_music(void)
{
	for (int i = 0; i < nActive; i++)
	{
		UpdateMusicStream(*activeMusic[i]);
	}
}

static int a_newSource(lua_State *l)
{
	const char *path = luaL_checkstring(l, 1);
	const char *type = luaL_optstring(l, 2, "static");
	char alt[256];
	const char *use = resolve_audio(path, alt, sizeof(alt));
	if (!use)
	{
		return luaL_error(l, "could not open audio file: %s", path);
	}

	Source *s = (Source *)lua_newuserdata(l, sizeof(Source));
	memset(s, 0, sizeof(Source));
	s->pitch = 1;
	s->volume = 1;
	s->appliedPitch = 1;

	if (strcmp(type, "stream") == 0)
	{
		s->music = (Music *)malloc(sizeof(Music));
		*s->music = LoadMusicStream(use);
		if (s->music->ctxData == NULL)
		{
			free(s->music);
			s->music = NULL;
			return luaL_error(l, "could not decode music: %s", use);
		}
	}
	else
	{
		SoundBank *b = (SoundBank *)calloc(1, sizeof(SoundBank));
		b->wave = LoadWave(use);
		if (b->wave.data == NULL)
		{
			free(b);
			return luaL_error(l, "could not decode sound: %s", use);
		}
		b->refs = 1;
		s->bank = b;
	}
	luaL_getmetatable(l, "love.Source");
	lua_setmetatable(l, -2);
	return 1;
}

static void src_play(Source *s)
{
	if (s->music)
	{
		SetMusicVolume(*s->music, s->volume);
		if (s->pitch != s->appliedPitch)
		{
			SetMusicPitch(*s->music, s->pitch);
			s->appliedPitch = s->pitch;
		}
		PlayMusicStream(*s->music);
		music_add(s->music);
		return;
	}
	SoundBank *b = s->bank;
	int idx = -1;
	for (int i = 0; i < b->n; i++)
	{
		if (!IsSoundPlaying(b->inst[i]))
		{
			idx = i;
			break;
		}
	}
	if (idx < 0)
	{
		if (b->n < MAX_INST)
		{
			b->inst[b->n] = LoadSoundFromWave(b->wave);
			idx = b->n++;
		}
		else
		{
			idx = s->cur++ % b->n;
			StopSound(b->inst[idx]);
		}
	}
	SetSoundVolume(b->inst[idx], s->volume);
	SetSoundPitch(b->inst[idx], s->pitch);
	PlaySound(b->inst[idx]);
}

static void src_stop(Source *s)
{
	if (s->music)
	{
		StopMusicStream(*s->music);
		music_remove(s->music);
		return;
	}
	for (int i = 0; i < s->bank->n; i++)
	{
		StopSound(s->bank->inst[i]);
	}
}

static int source_play(lua_State *l)
{
	src_play((Source *)luaL_checkudata(l, 1, "love.Source"));
	return 0;
}

static int source_stop(lua_State *l)
{
	src_stop((Source *)luaL_checkudata(l, 1, "love.Source"));
	return 0;
}

static int source_setVolume(lua_State *l)
{
	Source *s = (Source *)luaL_checkudata(l, 1, "love.Source");
	s->volume = (float)luaL_checknumber(l, 2);
	if (s->music)
	{
		SetMusicVolume(*s->music, s->volume);
	}
	else
	{
		for (int i = 0; i < s->bank->n; i++)
		{
			if (IsSoundPlaying(s->bank->inst[i]))
			{
				SetSoundVolume(s->bank->inst[i], s->volume);
			}
		}
	}
	return 0;
}

static int source_getVolume(lua_State *l)
{
	lua_pushnumber(l, ((Source *)luaL_checkudata(l, 1, "love.Source"))->volume);
	return 1;
}

static int source_setPitch(lua_State *l)
{
	Source *s = (Source *)luaL_checkudata(l, 1, "love.Source");
	s->pitch = (float)luaL_checknumber(l, 2);
	if (s->pitch < 0.1f) s->pitch = 0.1f;
	if (s->music && s->pitch != s->appliedPitch) // only touch raylib when it changed
	{
		SetMusicPitch(*s->music, s->pitch);
		s->appliedPitch = s->pitch;
	}
	return 0;
}

static int source_getPitch(lua_State *l)
{
	lua_pushnumber(l, ((Source *)luaL_checkudata(l, 1, "love.Source"))->pitch);
	return 1;
}

static int source_setLooping(lua_State *l)
{
	Source *s = (Source *)luaL_checkudata(l, 1, "love.Source");
	if (s->music) s->music->looping = lua_toboolean(l, 2) ? true : false;
	return 0;
}

static int source_isPlaying(lua_State *l)
{
	Source *s = (Source *)luaL_checkudata(l, 1, "love.Source");
	int p = 0;
	if (s->music)
	{
		p = IsMusicStreamPlaying(*s->music);
	}
	else
	{
		for (int i = 0; i < s->bank->n; i++)
		{
			if (IsSoundPlaying(s->bank->inst[i])) p = 1;
		}
	}
	lua_pushboolean(l, p);
	return 1;
}

static int source_clone(lua_State *l)
{
	Source *s = (Source *)luaL_checkudata(l, 1, "love.Source");
	if (s->music)
	{
		// streams are not cloned
		lua_pushvalue(l, 1);
		return 1;
	}
	Source *c = (Source *)lua_newuserdata(l, sizeof(Source));
	*c = *s;
	c->bank->refs++;
	c->cur = 0;
	luaL_getmetatable(l, "love.Source");
	lua_setmetatable(l, -2);
	return 1;
}

static int source_gc(lua_State *l)
{
	Source *s = (Source *)luaL_checkudata(l, 1, "love.Source");
	if (s->music)
	{
		music_remove(s->music);
		UnloadMusicStream(*s->music);
		free(s->music);
		s->music = NULL;
	}
	if (s->bank)
	{
		if (--s->bank->refs <= 0)
		{
			for (int i = 0; i < s->bank->n; i++)
				UnloadSound(s->bank->inst[i]);
			UnloadWave(s->bank->wave);
			free(s->bank);
		}
		s->bank = NULL;
	}
	return 0;
}

static int a_play(lua_State *l)
{
	return source_play(l);
}

static int a_stop(lua_State *l)
{
	return lua_isnoneornil(l, 1) ? 0 : source_stop(l);
}

static int a_setVolume(lua_State *l)
{
	SetMasterVolume((float)luaL_checknumber(l, 1));
	return 0;
}

static int a_getVolume(lua_State *l)
{
	lua_pushnumber(l, 1.0);
	return 1;
}

const luaL_Reg audio_funcs[] =
{
	{"newSource", a_newSource},
	{"play", a_play},
	{"stop", a_stop},
	{"setVolume", a_setVolume},
	{"getVolume", a_getVolume},
	{NULL, NULL}
};

const luaL_Reg source_methods[] =
{
	{"play", source_play},
	{"stop", source_stop},
	{"setVolume", source_setVolume},
	{"getVolume", source_getVolume},
	{"setPitch", source_setPitch},
	{"getPitch", source_getPitch},
	{"setLooping", source_setLooping},
	{"isPlaying", source_isPlaying},
	{"setFilter", g_noop},
	{"clone", source_clone},
	{"__gc", source_gc},
	{NULL, NULL}
};
