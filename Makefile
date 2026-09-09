V=1
SOURCE_DIR=src
BUILD_DIR=build
FILESYSTEM_DIR=filesystem
N64_INST ?= /n64_toolchain
export N64_INST

include $(N64_INST)/include/n64.mk

# Debug build
N64_C_AND_CXX_FLAGS := $(filter-out -O2,$(N64_C_AND_CXX_FLAGS))
N64_C_AND_CXX_FLAGS += -O0 -g -fno-omit-frame-pointer -I$(SOURCE_DIR)/includes

SOURCES := $(sort $(shell find $(SOURCE_DIR) -name '*.c' -print))
OBJECTS := $(patsubst $(SOURCE_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))
DFS_FILES := $(shell find $(FILESYSTEM_DIR) -type f -print | sort)

all: n64-demo.z64

$(BUILD_DIR):
	mkdir -p $@

# If n64.mk already builds the ELF from SOURCES, leave this as-is.
# Otherwise add a real link rule here.
$(BUILD_DIR)/n64-demo.elf: $(OBJECTS)

# Build the DFS image from the filesystem directory
$(BUILD_DIR)/n64-demo.dfs: $(DFS_FILES) 

# ROM must rebuild when either the ELF or DFS changes
n64-demo.z64: $(BUILD_DIR)/n64-demo.elf $(BUILD_DIR)/n64-demo.dfs

clean:
	rm -rf $(BUILD_DIR) *.z64 *.map

.PHONY: all clean