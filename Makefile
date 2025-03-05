PYTHON = python
PYTEST = pytest
GO = go
CMAKE = cmake
BUILD_DIR = build
DEBUG_DIR = $(BUILD_DIR)/Debug
RELEASE_DIR = $(BUILD_DIR)/Release
VCPKG_TOOLCHAIN = C:/vcpkg/scripts/buildsystems/vcpkg.cmake  

ORDER_RECEIVER_DIR = order_receiver
ORDER_PROCESSOR_DIR = order_processor
RESTAURANT_API_DIR = restaraunt_api
TESTS_DIR = tests


RESTAURANT_API_SRC = $(RESTAURANT_API_DIR)/main.go
RESTAURANT_API_EXE = $(RESTAURANT_API_DIR)/restaurant_api


GOFLAGS = -v

all: build

build: build_dirs cmake_order_receiver cmake_order_processor build_restaurant_api

build_dirs:
	mkdir -p $(BUILD_DIR)

cmake_order_receiver:
	cd $(ORDER_RECEIVER_DIR) && $(CMAKE) -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=$(VCPKG_TOOLCHAIN)
	cd $(ORDER_RECEIVER_DIR)/$(BUILD_DIR) && $(CMAKE) --build . --config Debug

cmake_order_processor:
	cd $(ORDER_PROCESSOR_DIR) && $(CMAKE) -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=$(VCPKG_TOOLCHAIN)
	cd $(ORDER_PROCESSOR_DIR)/$(BUILD_DIR) && $(CMAKE) --build . --config Debug

build_restaurant_api:
	cd $(RESTAURANT_API_DIR) && $(GO) build -o $(RESTAURANT_API_EXE) $(GOFLAGS)


test:
	cd $(TESTS_DIR) && $(PYTEST) -v --full-trace


run: build
	@echo "Run order_receiver"
	$(ORDER_RECEIVER_DIR)/$(BUILD_DIR)/Debug/order_receiver &
	@echo "Run order_processor"
	$(ORDER_PROCESSOR_DIR)/$(BUILD_DIR)/Debug/order_processor &
	@echo "Run restaurant_api"
	cd $(RESTAURANT_API_DIR) && $(RESTAURANT_API_EXE) &


clean:
	rm -rf $(ORDER_RECEIVER_DIR)/$(BUILD_DIR) $(ORDER_PROCESSOR_DIR)/$(BUILD_DIR)
	cd $(RESTAURANT_API_DIR) && rm -f $(RESTAURANT_API_EXE)


help:
	@echo "Available Commands:"
	@echo "  make build    - Assemble all the project components"
	@echo "  make test     - run tests"
	@echo "  make run      - run all components"
	@echo "  make clean    - clear generated files"
	@echo "  make help     - Show this help"

.PHONY: all build build_dirs cmake_order_receiver cmake_order_processor build_restaurant_api test run clean help