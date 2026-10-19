# KaiOS platform (asm.js via emscripten, 240x320 feature phones)

# Included by the root Makefile when PLATFORM=kaios.
# Settings are namespaced KAIOS_* and can be set in config.mk.

KAIOS_SCREEN_W ?= 240
KAIOS_SCREEN_H ?= 320
KAIOS_MEM_MB ?= 64 # 32 small, 64 default, 128 extreme
KAIOS_SHRINK_AUDIO ?= 1
KAIOS_EMSDK_ENV ?= $(HOME)/emsdk-3.1.29/emsdk_env.sh
KAIOS_RAYLIB ?= $(HOME)/.local/include/raylib/kaios/libraylib.a
KAIOS_RAYLIB_INC ?= $(dir $(KAIOS_RAYLIB))
KAIOS_SHELLFILE ?= common/shell.html
KAIOS_APP_TEMPLATE ?=

KAIOS_LUA_BUILD := $(PLATFORM_OBJ)/lua
KAIOS_LUA_LIB := $(KAIOS_LUA_BUILD)/liblua.a

export KAIOS_SCREEN_W KAIOS_SCREEN_H KAIOS_MEM_MB KAIOS_SHRINK_AUDIO \
 KAIOS_EMSDK_ENV KAIOS_RAYLIB KAIOS_RAYLIB_INC KAIOS_SHELLFILE \
 KAIOS_APP_TEMPLATE KAIOS_LUA_LIB

platform-deps: $(KAIOS_LUA_LIB)

$(KAIOS_LUA_LIB):
	rm -rf "$(KAIOS_LUA_BUILD)"
	mkdir -p "$(PLATFORM_OBJ)"
	cp -r "$(LUA_DIR)/src" "$(KAIOS_LUA_BUILD)"
	source "$(KAIOS_EMSDK_ENV)" >/dev/null 2>&1 && \
	emmake $(MAKE) -C "$(KAIOS_LUA_BUILD)" generic CC=emcc AR="emar rcu" RANLIB=emranlib

platform-build:
	$(PLATFORM_DIR)/build.sh

platform-clean:
	@true

platform-config:
	@echo "KAIOS_SCREEN = $(KAIOS_SCREEN_W)x$(KAIOS_SCREEN_H)"
	@echo "KAIOS_MEM_MB = $(KAIOS_MEM_MB)"
	@echo "KAIOS_SHRINK_AUDIO = $(KAIOS_SHRINK_AUDIO)"
	@echo "KAIOS_EMSDK_ENV = $(KAIOS_EMSDK_ENV)"
	@echo "KAIOS_RAYLIB = $(KAIOS_RAYLIB)"
	@echo "KAIOS_SHELLFILE = $(KAIOS_SHELLFILE)"
	@echo "KAIOS_APP_TEMPLATE = $(KAIOS_APP_TEMPLATE)"
