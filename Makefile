#-------------------------------------------------------------------------------
PLATFORM ?= PLATFORM_DESKTOP


#-------------------------------------------------------------------------------
GAME_NAME ?= Game
GAME_VERSION = 0.0.1

GAME_SOURCE_PATH ?= src
GAME_BUILD_PATH ?= build

GAME_BUILD_MODE ?= RELEASE


#-------------------------------------------------------------------------------
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
	ifeq ($(OS),Windows_NT)
		PLATFORM_OS = WINDOWS
	else
		UNAME = $(shell uname -s)

		ifeq ($(UNAME),Linux)
			PLATFORM_OS = LINUX
		else ifeq ($(UNAME),Darwin)
			PLATFORM_OS = OSX
		endif
	endif
endif


#-------------------------------------------------------------------------------
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
	GRAPHICS ?= GRAPHICS_API_OPENGL_33
else ifeq ($(PLATFORM),PLATFORM_ANDROID)
	GRAPHICS = GRAPHICS_API_OPENGL_ES2
endif


#-------------------------------------------------------------------------------
CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11
CPPFLAGS = -D$(PLATFORM) -D$(GRAPHICS)


ifeq ($(PLATFORM),PLATFORM_DESKTOP)
	ifeq ($(PLATFORM_OS),OSX)
		CC = clang
	endif
else ifeq ($(PLATFORM),PLATFORM_ANDROID)
	CFLAGS += -fPIC
endif

ifeq ($(GAME_BUILD_MODE), DEBUG)
	CFLAGS += -g
	CPPFLAGS += -DDEBUG
else ifeq ($(GAME_BUILD_MODE), RELEASE)
	CFLAGS += -s -O2
	CPPFLAGS += -DNDEBUG
endif


#-------------------------------------------------------------------------------
INCLUDE_PATHS = include src/external/raylib


ifeq ($(PLATFORM),PLATFORM_DESKTOP)
	INCLUDE_PATHS += src/external/raylib/external/glfw/include

	ifeq ($(PLATFORM_OS),WINDOWS)
		INCLUDE_PATHS += src/external/raylib/external/glfw/deps/mingw
	endif
else ifeq ($(PLATFORM),PLATFORM_ANDROID)
	INCLUDE_PATHS += src/external/android
endif

CPPFLAGS += $(foreach path,$(INCLUDE_PATHS),-I$(path))


#-------------------------------------------------------------------------------
LDLIBS = -lm
LDFLAGS = -Wl,--no-undefined

ifeq ($(PLATFORM),PLATFORM_DESKTOP)
	ifeq ($(PLATFORM_OS),WINDOWS)
		LDLIBS += -static-libgcc -lopengl32 -lgdi32 -lwinmm
	else ifeq ($(PLATFORM_OS),LINUX)
		LDLIBS += -lGL -lpthread -ldl -lrt -lX11
	else ifeq ($(PLATFORM_OS),OSX)
		LDLIBS += -framework OpenGL -framework Cocoa -framework IOKit \
				  -framework CoreAudio -framework CoreVideo
	endif
else ifeq ($(PLATFORM),PLATFORM_ANDROID)
	LDLIBS += -llog -landroid -lEGL -lGLESv2 -lOpenSLES
	LDFLAGS += -shared -u ANativeActivity_onCreate
endif


#-------------------------------------------------------------------------------
EXTERNAL_LIB_PATHS = raylib
EXTERNAL_LIB_IGNORE =

not-containing = $(foreach v,$2,$(if $(findstring $1,$v),,$v))


ifeq ($(PLATFORM),PLATFORM_DESKTOP)
else ifeq ($(PLATFORM),PLATFORM_ANDROID)
	EXTERNAL_LIB_PATHS += android
	EXTERNAL_LIB_IGNORE += rglfw.c
endif

SOURCES = $(foreach path,$(EXTERNAL_LIB_PATHS),$(wildcard src/external/$(path)/*.c))

ifneq ($(EXTERNAL_LIB_IGNORE),)
	SOURCES := $(foreach ignore,$(EXTERNAL_LIB_IGNORE),$(call not-containing,$(ignore),$(SOURCES)))
endif


#-------------------------------------------------------------------------------
SOURCES += $(wildcard $(GAME_SOURCE_PATH)/*.c) \
           $(wildcard $(GAME_SOURCE_PATH)/scenes/*.c)

PATH_SEP = /

ifeq ($(PLATFORM),PLATFORM_DESKTOP)
	ifeq ($(PLATFORM_OS),WINDOWS)
		PATH_SEP = \\
	endif
else ifeq ($(PLATFORM),PLATFORM_ANDROID)
	GAME_NAME := lib$(GAME_NAME).so
endif

SOURCES := $(subst /,$(PATH_SEP),$(SOURCES))
OBJECTS = $(subst $(GAME_SOURCE_PATH)$(PATH_SEP),$(GAME_BUILD_PATH)$(PATH_SEP),$(SOURCES:.c=.o))


#-------------------------------------------------------------------------------
mkdir = $(shell mkdir -p $1)
RM = rm -rf

ifeq ($(PLATFORM),PLATFORM_DESKTOP)
	ifeq ($(PLATFORM_OS),WINDOWS)
		mkdir = $(shell if not exist "$1" mkdir "$1")
	endif
endif


#-------------------------------------------------------------------------------
.PHONY: all
all: $(GAME_NAME)

$(GAME_NAME): $(OBJECTS)
	$(CC) $^ $(LDFLAGS) $(LDLIBS) -o $@

$(GAME_BUILD_PATH)$(PATH_SEP)%.o: $(GAME_SOURCE_PATH)/%.c
	$(call mkdir,$(@D))
	$(CC) -c $< $(CPPFLAGS) $(CFLAGS) -o $@

.PHONY: clean
clean:
	$(RM) $(GAME_BUILD_PATH)
	$(RM) $(GAME_NAME)

