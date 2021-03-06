SHELL := /bin/bash
RM    := rm -rf
MKDIR := mkdir -p
BUILD_DIR ?= build
ROOT_DIR := $(CURDIR)

debug: BUILD_TYPE=Debug
debug: -DCMAKE_EXPORT_COMPILE_COMMANDS=1
debug: ./$(BUILD_DIR)/Makefile
	@ $(MAKE) -C $(BUILD_DIR) -j8

sanitize_address: BUILD_TYPE=Debug
sanitize_address: EXTRA_ARGS=-DCMAKE_CXX_FLAGS=-fsanitize=address
sanitize_address: ./$(BUILD_DIR)/Makefile
	@ $(MAKE) -C $(BUILD_DIR) -j8

sanitize_leak: BUILD_TYPE=Debug
sanitize_leak: EXTRA_ARGS=-DCMAKE_CXX_FLAGS=-fsanitize=leak
sanitize_leak: ./$(BUILD_DIR)/Makefile
	@ $(MAKE) -C $(BUILD_DIR) -j8

sanitize_thread: BUILD_TYPE=Debug
sanitize_thread: EXTRA_ARGS=-DCMAKE_CXX_FLAGS=-fsanitize=thread
sanitize_thread: ./$(BUILD_DIR)/Makefile
	@ $(MAKE) -C $(BUILD_DIR) -j8

sanitize_undefined: BUILD_TYPE=Debug
sanitize_undefined: EXTRA_ARGS=-DCMAKE_CXX_FLAGS=-fsanitize=undefined
sanitize_undefined: ./$(BUILD_DIR)/Makefile
	@ $(MAKE) -C $(BUILD_DIR) -j8

perf_tests: BUILD_TYPE=Release
perf_tests: EXTRA_ARGS=-DCATCH_CONFIG_ENABLE_BENCHMARKING
perf_tests: ./$(BUILD_DIR)/Makefile
	@ $(MAKE) -C $(BUILD_DIR) -j8

release: BUILD_TYPE=Release
release: ./$(BUILD_DIR)/Makefile
	@ $(MAKE) -C $(BUILD_DIR) -j8

all: ./$(BUILD_DIR)/Makefile
	@ $(MAKE) -C $(BUILD_DIR) -j8

./$(BUILD_DIR)/Makefile:
	@  ($(MKDIR) -p $(BUILD_DIR) > /dev/null)
	@  (cd $(BUILD_DIR) > /dev/null 2>&1 && cmake $(EXTRA_ARGS) -DCMAKE_INSTALL_PREFIX=`pwd`/install -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) $(ROOT_DIR))

clean:
	@ $(MAKE) -C $(BUILD_DIR) clean

do_test:
	@  (cd $(BUILD_DIR) > /dev/null && ctest -L unit --verbose)

install:
	@  (cd $(BUILD_DIR) > /dev/null && make install)

distclean:
	@  ($(MKDIR) $(BUILD_DIR) > /dev/null)
	@  (cd $(BUILD_DIR) > /dev/null 2>&1 && cmake $(ROOT_DIR) > /dev/null 2>&1)
	@- $(MAKE) --silent -C $(BUILD_DIR) clean || true
	@- $(RM) ./$(BUILD_DIR)/Makefile
	@- $(RM) ./$(BUILD_DIR)/CMake*
	@- $(RM) ./$(BUILD_DIR)/cmake.*
	@- $(RM) ./$(BUILD_DIR)/*.cmake
	@- $(RM) ./$(BUILD_DIR)/*.txt

ifeq ($(findstring distclean,$(MAKECMDGOALS)),)
	$(MAKECMDGOALS): ./$(BUILD_DIR)/Makefile
	@ $(MAKE) -C $(BUILD_DIR) $(MAKECMDGOALS)
endif
