CC = gcc
FLAGS = -Wall -Wextra -g
DEST_DIR = ./bin
APP = ${DEST_DIR}/myshell

all: $(APP)

.PHONY clean:
	rm -rf $(DEST_DIR)

$(APP): src/main.c
	@mkdir -p $(DEST_DIR)
	$(CC) $(FLAGS) -o $(APP) src/main.c