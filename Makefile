# Raylove build system (platform-agnostic core)
#
# make <platform> [GAME=...]    e.g. make kaios GAME=games/foo
# make build PLATFORM=kaios     same thing, explicit
# make platforms                list available platforms
# make config                   show effective settings
# make clean                    remove all build output
#
# Settings: copy config.mk.example to config.mk. The command line and
# environment override config.mk.
#
# Platform maintainers: see platforms/README.md. Everything platform
# specific (toolchain, Lua build, packaging) lives in platforms/<name>/.
# The engine sources live in $(SRC_DIR)/ and are exported to platform.mk
# as SRC_DIR (directory), SRCS (all .c files) and SRC_HDRS (all .h files).

SHELL := bash

# stage -> platform-deps -> platform-build must run in order
.NOTPARALLEL:

-include config.mk

GAME ?=
PLATFORM ?=
OUT ?= build
LUA_DIR ?= lua-5.2.4
SRC_DIR ?= src

VIRT_W ?= 240
VIRT_H ?= 320

# engine sources: every .c file in src/ (new modules are picked up automatically)
SRCS := $(sort $(wildcard $(SRC_DIR)/*.c))
SRC_HDRS := $(sort $(wildcard $(SRC_DIR)/*.h))

PLATFORMS := $(sort $(notdir $(patsubst %/,%,$(dir $(wildcard platforms/*/platform.mk)))))

#  load the selected platform
ifneq ($(PLATFORM),)
	ifeq ($(wildcard platforms/$(PLATFORM)/platform.mk),)
	$(error unknown PLATFORM '$(PLATFORM)'. Available: $(PLATFORMS))
	endif
	PLATFORM_DIR := platforms/$(PLATFORM)
	PLATFORM_OUT := $(OUT)/$(PLATFORM)
	PLATFORM_OBJ := $(PLATFORM_OUT)/obj
	PLATFORM_BUILD := $(PLATFORM_OUT)/build
	STAGE := $(PLATFORM_OBJ)/assets
	export PLATFORM PLATFORM_DIR PLATFORM_OUT PLATFORM_OBJ PLATFORM_BUILD STAGE
	include $(PLATFORM_DIR)/platform.mk
endif

export GAME OUT LUA_DIR VIRT_W VIRT_H SRC_DIR SRCS SRC_HDRS

.PHONY: all build stage platforms config clean \
 platform-deps platform-build platform-clean $(PLATFORMS)

all: platforms

# `make kaios` == `make build PLATFORM=kaios`
$(PLATFORMS):
	@$(MAKE) --no-print-directory PLATFORM=$@ build

# Hooks a platform.mk may attach recipes to (all optional except -build).
platform-deps:
platform-clean:

build: need-platform need-sources stage platform-deps platform-build

need-platform:
	@test -n "$(PLATFORM)" || { echo "error: no platform. Use 'make <platform>' ($(PLATFORMS)) or set PLATFORM in config.mk"; exit 1; }
.PHONY: need-platform

need-sources:
	@test -n "$(SRCS)" || { echo "error: no engine sources found in $(SRC_DIR)/"; exit 1; }
.PHONY: need-sources

# generic: unpack/copy the game and strip dev-only files
stage: need-platform
	@test -n "$(GAME)" || { echo "error: set GAME=<game dir | game.love> (or put it in config.mk)"; exit 1; }
	@test -e "$(GAME)" || { echo "error: GAME not found: $(GAME)"; exit 1; }
	rm -rf "$(STAGE)"
	mkdir -p "$(STAGE)"
	@case "$(GAME)" in \
		*.love) unzip -q -o "$(GAME)" -d "$(STAGE)" ;; \
		*) cp -r "$(GAME)"/. "$(STAGE)/" ;; \
	esac
	rm -rf "$(STAGE)/aseprite" "$(STAGE)/.git"
	find "$(STAGE)" \( -name '*.aseprite' -o -name '*.psd' -o -name '*.kra' -o -name '.DS_Store' \) -delete

platforms:
	@echo "Available platforms: $(PLATFORMS)"
	@echo "Build one with: make <platform> GAME=..."

config:
	@echo "PLATFORM = $(PLATFORM)"
	@echo "GAME = $(GAME)"
	@echo "OUT = $(OUT)"
	@echo "LUA_DIR = $(LUA_DIR)"
	@echo "SRC_DIR = $(SRC_DIR)"
	@echo "SRCS = $(SRCS)"
	@echo "VIRT = $(VIRT_W)x$(VIRT_H)"
	@$(MAKE) --no-print-directory platform-config 2>/dev/null || true

# platform.mk may define platform-config to print its own settings
.PHONY: platform-config
platform-config:

clean: platform-clean
	rm -rf "$(OUT)"
