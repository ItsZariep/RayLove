
LINUX_CC ?= gcc
LINUX_CFLAGS ?= -Wall -O2 -g

LINUX_SCREEN_W ?= 240
LINUX_SCREEN_H ?= 320

LINUX_RAYLIB_CFLAGS ?= $(shell pkg-config --cflags raylib 2>/dev/null)
LINUX_RAYLIB_LIBS ?= $(shell pkg-config --libs raylib 2>/dev/null || echo -lraylib -lGL -lm -lpthread -ldl -lrt -lX11)

LINUX_SYSLIBS ?= -lm -ldl -lpthread

LINUX_LUA_BUILD := $(PLATFORM_OUT)/lua
LINUX_LUA_LIB := $(LINUX_LUA_BUILD)/liblua.a
LINUX_BIN := $(PLATFORM_OUT)/raylove.linux

LINUX_PRELUDE := $(abspath prelude.lua)
LINUX_GAME_DIR := $(abspath $(STAGE))

platform-deps: $(LINUX_LUA_LIB)

$(LINUX_LUA_LIB):
	rm -rf "$(LINUX_LUA_BUILD)"
	mkdir -p "$(PLATFORM_OUT)"
	cp -r "$(LUA_DIR)/src" "$(LINUX_LUA_BUILD)"
	$(MAKE) -C "$(LINUX_LUA_BUILD)" posix CC=$(LINUX_CC)

platform-build: $(LINUX_BIN)
	@echo "built $(LINUX_BIN)"


.PHONY: $(LINUX_BIN)
$(LINUX_BIN): raylove.c $(LINUX_LUA_LIB)
	$(LINUX_CC) raylove.c -o "$@" $(LINUX_CFLAGS) -DPLATFORM_DESKTOP \
		-DSCREEN_W=$(LINUX_SCREEN_W) -DSCREEN_H=$(LINUX_SCREEN_H) \
		-DVIRT_W=$(VIRT_W) -DVIRT_H=$(VIRT_H) \
		-DPRELUDE_PATH='"$(LINUX_PRELUDE)"' -DGAME_DIR='"$(LINUX_GAME_DIR)"' \
		-I"$(LUA_DIR)/src" $(LINUX_RAYLIB_CFLAGS) \
		"$(LINUX_LUA_LIB)" $(LINUX_RAYLIB_LIBS) $(LINUX_SYSLIBS)

.PHONY: run
run: build
	"$(abspath $(LINUX_BIN))" "$(LINUX_GAME_DIR)"

platform-clean:
	@true

platform-config:
	@echo "LINUX_CC = $(LINUX_CC)"
	@echo "LINUX_SCREEN = $(LINUX_SCREEN_W)x$(LINUX_SCREEN_H)"
	@echo "LINUX_RAYLIB_CFLAGS= $(LINUX_RAYLIB_CFLAGS)"
	@echo "LINUX_RAYLIB_LIBS = $(LINUX_RAYLIB_LIBS)"