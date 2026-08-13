BUILD?=DEBUG
ifeq ($(BUILD),DEBUG)
	BUILD_DIR := build/debug
else ifeq ($(BUILD),RELEASE)
	BUILD_DIR := build/release
else
	$(error Unsupported BUILD option)
endif

ifdef VERBOSE
	DIRFLAG := --print-directory
else
	DIRFLAG := --no-print-directory
endif

MAKEFILES := angel sandbox

.PHONY: all $(MAKEFILES) clean
.SILENT:

all: $(MAKEFILES)

$(MAKEFILES):
	$(MAKE) $(DIRFLAG) BUILD=$(BUILD) -C $@ 

clean:
	$(info cleaning...)
	rm -rf $(BUILD_DIR)
