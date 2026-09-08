V=1
SOURCE_DIR=src
BUILD_DIR=build
N64_INST ?= /n64_toolchain
export N64_INST

include $(N64_INST)/include/n64.mk

# Use a no-optimization debug build so breakpoints and source stepping work reliably.
N64_C_AND_CXX_FLAGS := $(filter-out -O2,$(N64_C_AND_CXX_FLAGS)) -O0 -g
N64_C_AND_CXX_FLAGS += -fno-omit-frame-pointer
# Ensure compiler finds project headers (now centralized under src/includes)
N64_C_AND_CXX_FLAGS += -I$(SOURCE_DIR)/includes

# The final ROM name you want to generate
all: n64-demo.z64

# Discover every .c file under src/ recursively and convert them to build object paths
SOURCES := $(sort $(shell find $(SOURCE_DIR) -name '*.c' -print))
OBJECTS := $(patsubst $(SOURCE_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))
assets = 

# Define the final ROM build components
$(BUILD_DIR)/n64-demo.elf: $(OBJECTS)

n64-demo.z64: N64_ROM_TITLE="N64 Demo"
n64-demo.z64: $(BUILD_DIR)/n64-demo.dfs

clean:
	rm -rf $(BUILD_DIR) *.z64
.PHONY: clean
