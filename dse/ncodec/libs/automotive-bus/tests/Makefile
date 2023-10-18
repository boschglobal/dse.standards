# Copyright 2023 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

GCC_BUILDER_IMAGE ?= ghcr.io/boschglobal/dse-gcc-builder:main
GDB_CMD ?= valgrind -q --leak-check=yes
# GDB_CMD ?= gdb -q -ex='set confirm on' -ex=run -ex=quit

default: build_docker

.PHONY: all
all: build
	cd build; make

build:
	mkdir build;
	cd build; cmake ..


.PHONY: test
test: test_docker

.PHONY: do-test
do-test:
	cd build; $(GDB_CMD) ./test_codec


.PHONY: build_docker
build_docker:
	@docker run -it --rm \
		--volume $$(pwd)/../../../../..:/tmp/repo \
		--workdir /tmp/repo/dse/ncodec/libs/automotive-bus/tests \
		$(GCC_BUILDER_IMAGE) \
		/bin/bash -c "make all"


.PHONY: test_docker
test_docker:
	@docker run -it --rm \
		--volume $$(pwd)/../../../../..:/tmp/repo \
		--env GDB_CMD="$(GDB_CMD)" \
		--workdir /tmp/repo/dse/ncodec/libs/automotive-bus/tests \
		$(GCC_BUILDER_IMAGE) \
		/bin/bash -c "make do-test"


.PHONY: clean
clean:
	rm -rf build
