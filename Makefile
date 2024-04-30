PROJECT = wakeonlan

CC = g++-11
ifeq ($(OS),Windows_NT)
LIBS = -lwsock32 -liphlpapi
LinuxFlags = 
else
LIBS = 
# https://stackoverflow.com/questions/77850769/fatal-threadsanitizer-unexpected-memory-mapping-when-running-on-linux-kernels
# LinuxFlags = -fsanitize=thread
LinuxFlags = 
endif

STD = -std=c++20
CFLAGS = -Wall -Wextra $(STD)
RELEASE_FLAGS = -O2
DEBUG_FLAGS = -g -DDEBUG
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))
SRC_FILES := $(call rwildcard,src,*.cpp)
INC_DIRS := $(wildcard include)
INC_FILES := $(call rwildcard,src,*.hpp)
OUT_DIR ?= build


.PHONY: all
all: release

.PHONY: debug
debug: $(SRC_FILES) $(INC_FILES)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(LinuxFlags) -I$(INC_DIRS) $(SRC_FILES) -o $(OUT_DIR)/$(PROJECT) $(LIBS)

.PHONY: release
release: $(SRC_FILES) $(INC_FILES)
	$(CC) $(CFLAGS) $(RELEASE_FLAGS) $(LinuxFlags) -I$(INC_DIRS) $(SRC_FILES) -o $(OUT_DIR)/$(PROJECT) $(LIBS)

.PHONY: docker
docker: docker_run

.PHONY: docker_build
docker_build:
	docker compose build

.PHONY: docker_run
docker_run:
	docker compose up -w

.PHONY: clear_build
clear_build:
	rm build/*

.PHONY: install_deps
install_deps:
	./install_deps.sh