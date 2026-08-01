CXX := g++

INCLUDES := -I../pastel
DEPFLAGS := -MMD -MD
WARNINGS :=
DEBUGFLAGS :=
DEFINES :=
OPT :=

LIBS := -lpastel
RPATH := -Wl,-rpath,'$$ORIGIN'

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

SRCS := $(call rwildcard,.,*.cpp) $(call rwildcard,.,*.c)
OBJS := $(SRCS:%=$(OBJ_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

ifeq ($(OS),Windows_NT)
	EXT := .exe
	POSTBUILD := cp $(ENGINE_LIB_DIR)/pastel.dll $(BIN_DIR)/ && \
				 echo && \
				 echo "copying pastel.dll to bin dir. (windows only)"
				
else
	EXT :=
	POSTBUILD :=
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
	$(POSTBUILD)

$(OBJ_DIR)/%.o: %
	mkdir -p $(dir $@)
	$(info $<...)
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(DEPS)
