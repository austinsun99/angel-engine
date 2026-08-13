CC := gcc
CXX := g++

INCLUDES := -I../angel
DEPFLAGS := -MMD -MD
WARNINGS := -Wall -Wextra -Werror -Wno-error=unused-parameter
DEBUGFLAGS := -g
DEFINES :=
OPT := -O0
LDFLAGS :=
BUILD_DIR := ../build/debug

LIBS := -langel
RPATH := -Wl,-rpath,'$$ORIGIN/../lib'

ENGINE_LIB_DIR := $(BUILD_DIR)/lib
LIBDIRS := -L$(ENGINE_LIB_DIR)

CXXFLAGS := $(INCLUDES) $(DEPFLAGS) $(WARNINGS) $(DEBUGFLAGS) $(DEFINES) $(OPT)
LDFLAGS += $(LIBDIRS) $(RPATH)
LDLIBS := $(LIBS)

#
# --- FIND FILES ---
#
rwildcard = $(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

OBJ_DIR := $(BUILD_DIR)/obj/tests
BIN_DIR := $(BUILD_DIR)/bin

SRCS := $(call rwildcard,.,*.cpp) $(call rwildcard,.,*.c)
OBJS := $(SRCS:%=$(OBJ_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

ifeq ($(OS),Windows_NT)
	EXT := .exe
	POSTBUILD := cp $(ENGINE_LIB_DIR)/angel.dll $(BIN_DIR)/ && \
				 echo && \
				 echo "copying angel.dll to bin dir. (windows only)"
else
	EXT :=
	POSTBUILD :=
endif
TARGET := $(BIN_DIR)/angel_tests$(EXT)

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

$(OBJ_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(info $<...)
	$(CC) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(info $<...)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)/tests
	rm -rf $(TARGET)

-include $(DEPS)
