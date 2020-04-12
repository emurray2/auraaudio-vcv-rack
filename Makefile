# If RACK_DIR is not defined when calling the Makefile, default to two directories above

RACK_DIR ?= ../..

MACHINE = $(shell $(CC) -dumpmachine)
ifneq (, $(findstring apple, $(MACHINE)))
	ARCH_MAC := 1
	ARCH := mac
else ifneq (, $(findstring mingw, $(MACHINE)))
	ARCH_WIN := 1
	ARCH := win
	LDFLAGS += -lopengl32 -lgdi32
	ifneq ( ,$(findstring x86_64, $(MACHINE)))
		ARCH_WIN_64 := 1
		BITS := 64
	else ifneq (, $(findstring i686, $(MACHINE)))
		ARCH_WIN_32 := 1
		BITS := 32
	endif
else ifneq (, $(findstring linux, $(MACHINE)))
	ARCH_LIN := 1
	ARCH := lin
else
$(error Could not determine architecture of $(MACHINE). Try hacking around in arch.mk)
endif

# FLAGS will be passed to both the C and C++ compiler
FLAGS +=
CFLAGS +=
CXXFLAGS +=

# Careful about linking to shared libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine, but they should be added to this plugin's build system.
LDFLAGS +=

# Add .cpp files to the build
SOURCES += $(wildcard src/*.cpp)

# Add files to the ZIP package when running `make dist`
# The compiled plugin and "plugin.json" are automatically added.
DISTRIBUTABLES += res
DISTRIBUTABLES += $(wildcard LICENSE*)

# Include the Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk
