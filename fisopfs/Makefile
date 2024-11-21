CC = gcc
CFLAGS := -ggdb3 -O2 -Wall -std=c11
CFLAGS += -Wno-unused-function -Wvla

# Necesario para pc de 64 bits?
CFLAGS += -D_FILE_OFFSET_BITS=64 
COMPILER = $(CC)
COMPILERFLAGS = $(CFLAGS)

extension = c

src_files ?= $(wildcard ./fs/*.$(extension))

# Flags for FUSE
LDLIBS := $(shell pkg-config fuse --cflags --libs)

# Name for the filesystem!
FS_NAME := fisopfs

all: build
o_src_files = $(patsubst %.$(extension),%.o,$(src_files))

build: $(o_src_files)
	$(COMPILER) $(o_src_files) $(FS_NAME).c -o $(FS_NAME) $(COMPILERFLAGS) $(LDLIBS)
%.o: %.$(extension)
	$(COMPILER) $(COMPILERFLAGS) -o $@ -c $<
	echo
	echo '-------------------' # visual marker to separate the output of each compilation (may or may not help)

format: .clang-files .clang-format
	xargs -r clang-format -i <$<

docker-build:
	docker build -t fisopfs:latest .

docker-run:
	docker container run -it --rm --name=fisopfs \
		-v $(shell pwd):/fisopfs \
		--device /dev/fuse \
		--cap-add SYS_ADMIN \
		--security-opt apparmor:unconfined \
		fisopfs:latest \
		bash 

docker-attach:
	docker exec -it fisopfs bash

clean:
	rm -rf $(EXEC) *.o core vgcore.* $(FS_NAME)
	rm -f $(o_src_files)

.PHONY: all build clean format docker-build docker-run docker-attach
