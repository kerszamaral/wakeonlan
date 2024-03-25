PROJECT = wakeonlan
CC = g++
CFLAGS = -Wall -std=c++23 -g
SRC_FILES = src/**.cpp
OUT_DIR = build


all: $(PROJECT)

run: $(PROJECT)
	./$(OUT_DIR)/$(PROJECT)

$(PROJECT): $(SRC_FILES)
	$(CC) $(CFLAGS) $(SRC_FILES) -o $(OUT_DIR)/$(PROJECT)

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