CC := gcc
CXX := g++

INCLUDES := -I.
DEPFLAGS := -MMD -MP
WARNINGS :=
DEFINES := -DPASTEL_EXPORT

#
# --- DEBUG & RELEASE MODE ---
#
BUILD?=DEBUG
ifeq ($(BUILD),DEBUG)
	DEFINES += -DDEBUG
	OPT := -O0
	DEBUGFLAGS := -g
	WARNINGS += -Wall -Wextra -Werror -Wno-error=unused-parameter -Wno-error=attributes -Wno-attributes
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

SRCS := $(call rwildcard,.,*.cpp) $(call rwildcard,.,*.c)

#
# --- CHECK WAYLAND SESSION ---
#
WAYLAND_SESSION := false
ifeq ($(OS),Windows_NT)
	LIBS += -lvulkan-1
	TARGET := $(LIB_DIR)/pastel.dll
	LIBFLAGS := -shared -Wl,--out-implib,$(LIB_DIR)/libpastel.a
else
	UNAME := $(shell uname)
ifeq ($(UNAME),Linux)
	LIBS += -lvulkan
	TARGET := $(LIB_DIR)/libpastel.so
	LIBFLAGS := -shared
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

#
# --- GENERATE WAYLAND FILES ---
# 
ifeq ($(WAYLAND_SESSION),true)
$(info )
$(info >>> detected wayland session, generating files...)
$(info )

WAYLAND_SHELL_DIR := external/wayland-protocols/xdg-shell
WAYLAND_SHELL_PROTOCOL := $(WAYLAND_SHELL_DIR)/xdg-shell.xml
XDG_SHELL_PROTOCOL_PATH := $(GEN_DIR)/xdg-shell-client-protocol.c
XDG_SHELL_PROTOCOL_HEADER_PATH := $(GEN_DIR)/xdg-shell-client-protocol.h

$(shell mkdir -p $(GEN_DIR))
$(info wayland-scanner xdg-shell-protocol.c)
$(shell wayland-scanner private-code $(WAYLAND_SHELL_PROTOCOL) $(XDG_SHELL_PROTOCOL_PATH))
$(info wayland-scanner xdg-shell-client-protocol.h)
$(shell wayland-scanner client-header $(WAYLAND_SHELL_PROTOCOL) $(XDG_SHELL_PROTOCOL_HEADER_PATH))
$(info )
$(info >>> finished file generation)
$(info )

# Update variables as necessary
SRCS += $(XDG_SHELL_PROTOCOL_PATH)
INCLUDES += -I$(GEN_DIR)
LIBS += -lwayland-client -lrt -lxkbcommon
endif

OBJS := $(SRCS:%=$(OBJ_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

#
# --- FLAGS ---
#
CXXFLAGS := $(INCLUDES) $(DEPFLAGS) $(WARNINGS) $(DEBUGFLAGS) $(DEFINES) $(OPT) -fPIC
LDFLAGS := $(LIBFLAGS) $(LIBDIRS)
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

$(OBJ_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(info $<...)
	$(CC) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(info $<...)
	$(CXX) $(CXXFLAGS) -std=c++20 -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)
