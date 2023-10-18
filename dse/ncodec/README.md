<!--
Copyright 2023 Robert Bosch GmbH

SPDX-License-Identifier: Apache-2.0
-->

# Dynamic Simulation Environment - Network Codec API

## Introduction

Network Codec API of the Dynamic Simulation Environment (DSE) Core Platform.


### Project Structure

```
L- dse/ncodec           NCodec API source code.
  L- example            Example implementation.
  L- libs               Codec Libraries.
    L- automotive-bus   Automotive Bus Schema reference implementation.
      L- tests          Supporting tests.
```


## Usage

### Toolchains

The example implementations and Codec Libraries of the Network Codec are built
using containerised toolchains. Those are available from the DSE C Library and
can be built as follows:

```bash
$ git clone https://github.com/boschglobal/dse.clib.git
$ cd dse.clib
$ make docker
```

Alternatively, the latest Docker Images are available on ghcr.io and can be
used as follows:

```bash
$ export GCC_BUILDER_IMAGE=ghcr.io/boschglobal/dse-gcc-builder:main
$ export GCC_TESTER_IMAGE=ghcr.io/boschglobal/dse-python-builder:main
```


### Examples

An example implementation is described at this location [example/README.md](example/README.md)


### Codec Libraries

The following Codec Libraries are included as a part of this project:

* [Automotive Bus Schema](libs/automotive-bus/README.md) supporting:
  * CAN Bus - streaming frame interface using Flatbuffers.


## Contribute

Please refer to the [CONTRIBUTING.md](../../CONTRIBUTING.md) file.


## License

Dynamic Simulation Environment Standards is open-sourced under the
Apache-2.0 license.

See the [LICENSE](../../LICENSE) and [NOTICE](../../NOTICE) files for details.


### Third Party Licenses

[Third Party Licenses](../../licenses/)