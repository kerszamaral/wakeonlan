PROJECT = wakeonlan
CC = g++
CFLAGS = -Wall -std=c++23 -g
SRC_FILES := $(wildcard src/*.cpp src/*/*.cpp)
INC_DIRS := $(wildcard include)
OUT_DIR = build


all: $(PROJECT)

run: $(PROJECT)
	./$(OUT_DIR)/$(PROJECT)

$(PROJECT): $(SRC_FILES)
	$(CC) $(CFLAGS) -I$(INC_DIRS) $(SRC_FILES) -o $(OUT_DIR)/$(PROJECT) 

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