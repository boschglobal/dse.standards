<!--
Copyright 2023 Robert Bosch GmbH

SPDX-License-Identifier: Apache-2.0
-->

# NCodec Example Implementation

The NCodec example implementation has the following files:

```
L- dse/ncodec           NCodec API source code.
  L- example            Example implementation.
    L- codec.c          A reference implementation of a Codec (NCodecVTable).
    L- dynamic.c        Implementation of a dynamically linked Codec.
    L- example.c        Example showing how to use the NCodec API.
    L- stream.c         A reference implementation of a Stream (NCodecStreamVTable).
    L- static.c         Implementation of a statically linked Codec.
```


## Running the Examples

```bash
# Build the Network Codec example.
$ cd dse/ncodec/example
$ make
...

# Run the statically linked example.
$ build/example_static
Message is: Hello World says simple network codec

# Run the dynamically linked example.
$ build/example_dynamic build/libcodec.so
Message is: Hello World says simple network codec
```