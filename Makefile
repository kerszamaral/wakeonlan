PROJECT = wakeonlan

ifeq ($(OS),Windows_NT)
CC = g++
LIBS = -lwsock32 -liphlpapi
LinuxFlags = 
else
ifeq (, $(shell which gcc-13))
CC = gcc
else
CC = g++-13
endif
LIBS = 
LinuxFlags = -fsanitize=thread
endif

CFLAGS = -Wall -std=c++23 -g
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))
SRC_FILES := $(call rwildcard,src,*.cpp)
INC_DIRS := $(wildcard include)
INC_FILES := $(call rwildcard,src,*.hpp)
OUT_DIR ?= build


.PHONY: all
all: debug

.PHONY: debug
debug: $(SRC_FILES) $(INC_FILES)
	$(CC) $(CFLAGS) -DDEBUG $(LinuxFlags) -I$(INC_DIRS) $(SRC_FILES) -o $(OUT_DIR)/$(PROJECT) $(LIBS)

.PHONY: release
release: $(SRC_FILES) $(INC_FILES)
	$(CC) $(CFLAGS) $(LinuxFlags) -I$(INC_DIRS) $(SRC_FILES) -o $(OUT_DIR)/$(PROJECT) $(LIBS)

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