BUILD?=DEBUG
ifeq ($(BUILD),DEBUG)
	BUILD_DIR := build/debug
else ifeq ($(BUILD),RELEASE)
	BUILD_DIR := build/release
else
	$(error Unsupported BUILD option)
endif

MAKEFILES := pastel sandbox

.PHONY: all $(MAKEFILES) clean
.SILENT:

all: $(MAKEFILES)

$(MAKEFILES):
	$(MAKE) --no-print-directory BUILD=$(BUILD) -C $@ 

clean:
	$(info cleaning...)
	rm -rf $(BUILD_DIR)
