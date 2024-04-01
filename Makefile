PROJECT = wakeonlan
CC = g++
DEBUG = -DDEBUG
CFLAGS = -Wall -std=c++20 -g $(DEBUG)
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))
SRC_FILES := $(call rwildcard,src,*.cpp)
INC_DIRS := $(wildcard include)
INC_FILES := $(call rwildcard,src,*.hpp)
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