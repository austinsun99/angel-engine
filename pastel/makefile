CXX := g++

INCLUDES := -I.
DEPFLAGS := -MMD -MD
WARNINGS :=
DEFINES :=

#
# --- DEBUG & RELEASE MODE ---
#
BUILD?=DEBUG
ifeq ($(BUILD),DEBUG)
	DEFINES += -DDEBUG
	OPT := -O0
	DEBUGFLAGS := -g
	WARNINGS += -Wall -Wextra -Werror -Wno-error=unused-variable
	BUILD_DIR := ../build/debug
else ifeq ($(BUILD),RELEASE)
	DEFINES += -DNDEBUG
	DEBUGFLAGS :=
	OPT := -O3
	BUILD_DIR := ../build/release
else
	$(error Unsupported BUILD option)
endif

LIBDIRS := 
LIBS :=

#
# --- FIND FILES ---
#
rwildcard = $(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

OBJ_DIR := $(BUILD_DIR)/obj/pastel
GEN_DIR := $(BUILD_DIR)/gen
LIB_DIR := $(BUILD_DIR)/lib

SRCS := $(call rwildcard,.,*.cpp,*.c)
OBJS := $(SRCS:%=$(OBJ_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

#
# --- CHECK WAYLAND SESSION ---
#
WAYLAND_SESSION := false
ifeq ($(OS),Windows_NT)
	EXT := .dll
else
	UNAME := $(shell uname)
ifeq ($(UNAME),Linux)
	EXT := .so
	DISPLAY_SERVER := $(shell \
					  if [ "$$XDG_SESSION_TYPE" = "wayland" ]; then echo "wayland"; \
					  elif [ -n "$$WAYLAND_DISPLAY" ]; then echo "wayland"; \
					  else echo "unknown"; fi \
					  )
	ifeq ($(DISPLAY_SERVER),wayland)
	WAYLAND_SESSION := true
	endif
endif
endif
TARGET := $(LIB_DIR)/libpastel$(EXT)

#
# --- GENERATE WAYLAND FILES ---
# 
ifeq ($(WAYLAND_SESSION),true)
$(info )
$(info >>> detected wayland session, generating files...)
$(info )

WAYLAND_SHELL_DIR := external/wayland-protocols/xdg-shell
WAYLAND_SHELL_PROTOCOL := $(WAYLAND_SHELL_DIR)/xdg-shell.xml

$(shell mkdir -p $(GEN_DIR))
$(info wayland-scanner xdg-shell-protocol.c)
$(shell wayland-scanner private-code $(WAYLAND_SHELL_PROTOCOL) $(GEN_DIR)/xdg-shell-protocol.c)
$(info wayland-scanner xdg-shell-client-protocol.h)
$(shell wayland-scanner client-header $(WAYLAND_SHELL_PROTOCOL) $(GEN_DIR)/xdg-shell-client-protocol.h)
$(info )
$(info >>> finished file generation)
$(info )

# Update variables as necessary
SRCS += $(GEN_DIR)/xdg-shell-protocol.c
INCLUDES += -I$(GEN_DIR)
LIBS += -lwayland-client -lrt
endif

#
# --- FLAGS ---
#
CXXFLAGS := -fPIC $(INCLUDES) $(DEPFLAGS) $(WARNINGS) $(DEBUGFLAGS) $(DEFINES) $(OPT)
LDFLAGS := -shared $(LIBDIRS)
LDLIBS := $(LIBS)

#
# --- RECIPES ---
#
.PHONY: all clean
.SILENT:

all: $(TARGET)

$(TARGET): $(OBJS)
	mkdir -p $(dir $@)
	$(info >>> $@ <<<)
	$(CXX) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $(TARGET)

$(OBJS): $(SRCS)
	mkdir -p $(dir $@)
	$(info $<...)
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(DEPS)
