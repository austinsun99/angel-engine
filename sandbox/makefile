CXX := g++

INCLUDES := -I../pastel
DEPFLAGS := -MMD -MD
WARNINGS :=
DEBUGFLAGS :=
DEFINES :=
OPT :=

LIBS := -lpastel
RPATH := -Wl,-rpath,'$$ORIGIN/../lib'

#
# --- DEBUG AND RELEASE
#
BUILD?=DEBUG
ifeq ($(BUILD),DEBUG)
	DEFINES += -DDEBUG
	OPT += -O0
	DEBUGFLAGS += -g
	WARNINGS += -Wall -Wextra -Werror -Wno-error=unused-variable
	BUILD_DIR := ../build/debug
else ifeq ($(BUILD),RELEASE)
	DEFINES += -DNDEBUG
	OPT += -O3
	BUILD_DIR := ../build/release
else
	$(error Unsupported BUILD option)
endif

#
# --- BUILD FLAGS ---
#

ENGINE_LIB_DIR := $(BUILD_DIR)/lib
LIBDIRS := -L$(ENGINE_LIB_DIR)

CXXFLAGS := $(INCLUDES) $(DEPFLAGS) $(WARNINGS) $(DEBUGFLAGS) $(DEFINES) $(OPT)
LDFLAGS := $(LIBDIRS) $(RPATH)
LDLIBS := $(LIBS)

#
# --- FIND FILES ---
#
rwildcard = $(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

OBJ_DIR := $(BUILD_DIR)/obj/sandbox
BIN_DIR := $(BUILD_DIR)/bin

SRCS := $(call rwildcard,.,*.cpp,*.c)
OBJS := $(SRCS:%=$(OBJ_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

ifeq ($(OS),Windows_NT)
	EXT := .exe
else
	UNAME := $(shell uname)
	ifeq ($(UNAME),Linux)
		EXT :=
	endif
endif
TARGET := $(BIN_DIR)/sandbox$(EXT)

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
