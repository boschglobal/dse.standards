<!--
Copyright 2023 Robert Bosch GmbH

SPDX-License-Identifier: Apache-2.0
-->

# Network Codec for Automotive Bus Schema

Implementation of Network Codec supporting the
[Automotive Bus Stream](https://github.com/boschglobal/automotive-bus-schema/blob/main/schemas/stream/frame.fbs).
interface schema.


## Schemas

### Stream | Frame | FBS

MIME Type
: application/x-automotive-bus; interface=stream; type=frame; schema=fbs

Flatbuffers file identifier
: SFRA


#### CAN Bus

MIME Type (minimal)
: application/x-automotive-bus; interface=stream; type=frame; bus=can; schema=fbs

MIME Type (extended)
: application/x-automotive-bus; interface=stream; type=frame; bus=can; schema=fbs; bus_id=1; node_id=2; interface_id=3


##### Additional Properties

The following parameters can be encoded directly in the MIME Type string
or set with calls to `ncodec_config()`.

| Property | Type | Default |
| --- |--- |--- |
| bus_id | uint8_t | 0 |
| node_id | uint8_t | 0 (must be set for normal operation [^1]) |
| interface_id | uint8_t | 0 |

[^1]: Message filtering on `node_id` (i.e. filter if Tx Node = Rx Node) is
only enabled when this parameter is set.


## Development

```bash
# Build
$ git clone https://github.com/boschglobal/dse.standards.git
$ cd dse.standards/dse/ncodec/libs/automotive-bus
$ make
...
$ ls build/*.a
build/libautomotive-bus-codec.a*

# Testing
$ make test
...
[==========] CODEC: Running 5 test(s).
[ RUN      ] test_trim
[       OK ] test_trim
[ RUN      ] test_codec_config
[       OK ] test_codec_config
[ RUN      ] test_codec_stat
[       OK ] test_codec_stat
[ RUN      ] test_ncodec_create_close
[       OK ] test_ncodec_create_close
[ RUN      ] test_ncodec_create_failon_mime
[       OK ] test_ncodec_create_failon_mime
[==========] CODEC: 5 test(s) run.
[  PASSED  ] 5 test(s).
[==========] CAN FBS: Running 7 test(s).
[ RUN      ] test_can_fbs_no_stream
[       OK ] test_can_fbs_no_stream
[ RUN      ] test_can_fbs_no_buffer
[       OK ] test_can_fbs_no_buffer
[ RUN      ] test_can_fbs_flush
[       OK ] test_can_fbs_flush
[ RUN      ] test_can_fbs_write
[       OK ] test_can_fbs_write
[ RUN      ] test_can_fbs_readwrite
[       OK ] test_can_fbs_readwrite
[ RUN      ] test_can_fbs_readwrite_frames
[       OK ] test_can_fbs_readwrite_frames
[ RUN      ] test_can_fbs_readwrite_messages
[       OK ] test_can_fbs_readwrite_messages
[==========] CAN FBS: 7 test(s) run.
[  PASSED  ] 7 test(s).
```



## Contribute

Please refer to the [CONTRIBUTING.md](../../../../CONTRIBUTING.md) file.


## License

Dynamic Simulation Environment Model C Library is open-sourced under the
Apache-2.0 license.

See the [LICENSE](../../../../LICENSE) and [NOTICE](../../../../NOTICE) files for details.


### Third Party Licenses

[Third Party Licenses](../../../../licenses/)