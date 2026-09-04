V=1
SOURCE_DIR=src
BUILD_DIR=build

include $(N64_INST)/include/n64.mk

# The final ROM name you want to generate
all: n64-demo.z64

# Just list the filenames *inside* the src directory without the "src/" prefix
src = main.c
assets = 

# Define the final ROM build components
$(BUILD_DIR)/n64-demo.elf: $(src:%.c=$(BUILD_DIR)/%.o)

n64-demo.z64: N64_ROM_TITLE="N64 Demo"
n64-demo.z64: $(BUILD_DIR)/n64-demo.dfs

clean:
	rm -rf $(BUILD_DIR) *.z64
.PHONY: clean
