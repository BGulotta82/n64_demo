V=1
SOURCE_DIR=src
BUILD_DIR=build
FILESYSTEM_DIR=filesystem
ASSET_DIR=assets
N64_INST ?= /n64_toolchain
MKSPRITE ?= $(N64_INST)/bin/mksprite
export N64_INST

include $(N64_INST)/include/n64.mk

N64_C_AND_CXX_FLAGS := $(filter-out -O2,$(N64_C_AND_CXX_FLAGS))
N64_C_AND_CXX_FLAGS += -O0 -g -fno-omit-frame-pointer -I$(SOURCE_DIR)/includes

SOURCES := $(sort $(shell find $(SOURCE_DIR) -name '*.c' -print))
OBJECTS := $(patsubst $(SOURCE_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))
DFS_FILES := $(shell find $(FILESYSTEM_DIR) -type f -print | sort)

SPRITE_SOURCES := $(shell find $(ASSET_DIR) -type f -name '*.png' -print | sort)
SPRITE_OUTPUTS := $(patsubst $(ASSET_DIR)/%.png,$(BUILD_DIR)/sprites/%.sprite,$(SPRITE_SOURCES))
FILESYSTEM_SPRITES := $(patsubst $(ASSET_DIR)/%.png,$(FILESYSTEM_DIR)/%.sprite,$(SPRITE_SOURCES))

all: sprites n64-demo.z64

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/sprites/%.sprite: $(ASSET_DIR)/%.png | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(MKSPRITE) -o $(dir $@) $<
	test -f $(dir $@)/$(notdir $(basename $@)).sprite

$(FILESYSTEM_DIR)/%.sprite: $(BUILD_DIR)/sprites/%.sprite | $(BUILD_DIR)
	mkdir -p $(dir $@)
	cp $< $@

sprites: $(SPRITE_OUTPUTS) $(FILESYSTEM_SPRITES)

$(BUILD_DIR)/n64-demo.elf: $(OBJECTS)

$(BUILD_DIR)/n64-demo.dfs: $(DFS_FILES)

n64-demo.z64: $(BUILD_DIR)/n64-demo.elf $(BUILD_DIR)/n64-demo.dfs

clean:
	rm -rf $(BUILD_DIR) *.z64 *.map

.PHONY: all clean sprites