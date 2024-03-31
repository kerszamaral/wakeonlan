PROJECT = wakeonlan
CC = g++
DEBUG = -DDEBUG
CFLAGS = -Wall -std=c++20 -g $(DEBUG)
SRC_FILES := $(shell find src/ -type f -name '*.cpp')
INC_DIRS := $(wildcard include)
INC_FILES := $(shell find include/ -type f -name '*.hpp')
OUT_DIR = build
ifeq ($(OS),Windows_NT)
LIBS = -lwsock32
else
LIBS = 
endif

.PHONY: run
run: build
	./$(OUT_DIR)/$(PROJECT)

.PHONY: build
build: $(SRC_FILES) $(INC_FILES)
	$(CC) $(CFLAGS) -I$(INC_DIRS) $(SRC_FILES) -o $(OUT_DIR)/$(PROJECT) $(LIBS)

.PHONY: docker
docker: docker_build docker_run

.PHONY: docker_build
docker_build:
	docker build -t $(PROJECT) .

.PHONY: docker_run
docker_run:
	docker run -it --rm $(PROJECT)

.PHONY: clear_build
clear_build:
	rm build/*