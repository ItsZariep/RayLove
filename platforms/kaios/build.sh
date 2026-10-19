#!/usr/bin/env bash

# KaiOS packaging step. Run by platforms/kaios/platform.mk (make kaios);
# the root Makefile has already staged the game into $STAGE and exported
# the settings below.

set -euo pipefail

: "${STAGE:?run via 'make kaios'}"
: "${PLATFORM_OUT:?run via 'make kaios'}"
PLATFORM_OBJ="${PLATFORM_OBJ:-$PLATFORM_OUT/obj}"
PLATFORM_BUILD="${PLATFORM_BUILD:-$PLATFORM_OUT/build}"
LUA_DIR="${LUA_DIR:-lua-5.2.4}"
VIRT_W="${VIRT_W:-240}"
VIRT_H="${VIRT_H:-320}"
SCREEN_W="${KAIOS_SCREEN_W:-240}"
SCREEN_H="${KAIOS_SCREEN_H:-320}"
MEM_MB="${KAIOS_MEM_MB:-64}"
SHRINK_AUDIO="${KAIOS_SHRINK_AUDIO:-1}"
EMSDK_ENV="${KAIOS_EMSDK_ENV:-$HOME/emsdk-3.1.29/emsdk_env.sh}"
RAYLIB="${KAIOS_RAYLIB:-$HOME/.local/include/raylib/kaios/libraylib.a}"
RAYLIB_INC="${KAIOS_RAYLIB_INC:-$(dirname "$RAYLIB")}"
SHELLFILE="${KAIOS_SHELLFILE:-common/shell.html}"
LUA_LIB="${KAIOS_LUA_LIB:-$PLATFORM_OBJ/lua/liblua.a}"
APP_TEMPLATE="${KAIOS_APP_TEMPLATE:-}"

[ -d "$STAGE" ] || { echo "error: $STAGE missing"; exit 1; }
[ -f "$SHELLFILE" ] || { echo "error: shell file not found: $SHELLFILE"; exit 1; }
[ -f "$RAYLIB" ] || { echo "error: raylib not found: $RAYLIB"; exit 1; }
[ -f "$LUA_LIB" ] || { echo "error: liblua.a not found: $LUA_LIB"; exit 1; }

source "$EMSDK_ENV" > /dev/null 2>&1

SRC_DIR="${SRC_DIR:-src}"
if [ -n "${SRCS:-}" ]; then
	read -r -a SRC_FILES <<< "$SRCS"
else
	SRC_FILES=("$SRC_DIR"/*.c)
fi
[ -e "${SRC_FILES[0]}" ] || { echo "error: no engine sources found in $SRC_DIR/"; exit 1; }


# Shrink audio: big WAVs -> mono 22kHz ogg, the rest -> mono 22kHz wav
if [ "$SHRINK_AUDIO" = "1" ]; then
	if command -v ffmpeg > /dev/null; then
		find "$STAGE" -name '*.wav' | while read -r f; do
			size=$(stat -c%s "$f")
			if [ "$size" -gt $((1024 * 1024)) ]; then
				ffmpeg -nostdin -loglevel error -y -i "$f" -ac 1 -ar 22050 -c:a libvorbis -q:a 1 "${f%.wav}.ogg" && rm "$f"
			else
				ffmpeg -nostdin -loglevel error -y -i "$f" -ac 1 -ar 22050 -c:a pcm_s16le "$f.tmp.wav" && mv "$f.tmp.wav" "$f"
			fi
		done
	else
		echo "WARNING: ffmpeg not found, audio left untouched (big WAVs may not fit in KaiOS memory)"
	fi
fi

# Compile
mkdir -p "$PLATFORM_BUILD"
rm -f "$PLATFORM_BUILD/index.html" "$PLATFORM_BUILD/index.js" "$PLATFORM_BUILD/index.data"
emcc "${SRC_FILES[@]}" -o "$PLATFORM_BUILD/index.html" -Wall -O3 -DPLATFORM_WEB \
	-DSCREEN_W="$SCREEN_W" -DSCREEN_H="$SCREEN_H" -DVIRT_W="$VIRT_W" -DVIRT_H="$VIRT_H" \
	-I"$SRC_DIR" -I"$LUA_DIR/src" -I"$RAYLIB_INC" \
	"$RAYLIB" "$LUA_LIB" \
	-s USE_GLFW=3 -s WASM=0 -s TOTAL_MEMORY=$((MEM_MB * 1024 * 1024)) --memory-init-file 0 \
	--shell-file "$SHELLFILE" \
	--preload-file "$STAGE@/assets" \
	--preload-file src/prelude.lua@/raylove/prelude.lua \
	2> >(grep -v "libxml2.so.2" >&2)

echo
du -sh "$PLATFORM_BUILD/index.js" "$PLATFORM_BUILD/index.data" "$STAGE"

if [ -n "$APP_TEMPLATE" ]; then
	cp "$PLATFORM_BUILD/index.html" "$PLATFORM_BUILD/index.js" "$PLATFORM_BUILD/index.data" "$APP_TEMPLATE/"
	echo "copied to $APP_TEMPLATE/"
else
	echo "copy $PLATFORM_BUILD/index.{html,js,data} into your app-template/ (next to manifest.webapp), or set KAIOS_APP_TEMPLATE in config.mk"
fi