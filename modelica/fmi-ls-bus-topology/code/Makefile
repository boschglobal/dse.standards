# Copyright 2024 Robert Bosch GmbH
#
# SPDX-License-Identifier: Apache-2.0

export MAKE_NPROC ?= $(shell nproc)
GCC_BUILDER_IMAGE = ghcr.io/boschglobal/dse-gcc-builder:main
GDB_CMD = valgrind -q --leak-check=full --track-origins=yes --error-exitcode=808
# GDB_CMD = gdb -q -ex='set confirm on' -ex=run -ex=quit


default: build_docker

.PHONY: configure
configure:
	mkdir build;
	cd build; cmake ..

.PHONY: build
build:
	if [ ! -d "build" ]; then make configure; fi
	cd build; cmake --build . -j $(MAKE_NPROC)

.PHONY: do-test
do-test:
	cd build/tests; $(GDB_CMD) ./test

.PHONY: test
test: test_docker

.PHONY: build_docker
build_docker:
	@docker run -it --rm \
		--volume $$(pwd)/../../..:/tmp/repo \
		--workdir /tmp/repo/modelica/fmi-ls-bus-topology/code \
		$(GCC_BUILDER_IMAGE) \
		/bin/bash -c "make build"

.PHONY: test_docker
test_docker:
	@docker run -it --rm \
		--env GDB_CMD="$(GDB_CMD)" \
		--volume $$(pwd)/../../..:/tmp/repo \
		--workdir /tmp/repo/modelica/fmi-ls-bus-topology/code \
		$(GCC_BUILDER_IMAGE) \
		/bin/bash -c "make do-test"

.PHONY: clean
clean:
	rm -rf build
