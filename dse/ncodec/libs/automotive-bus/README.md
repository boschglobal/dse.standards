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


### Stream | PDU | FBS

MIME Type
: application/x-automotive-bus; interface=stream; type=pdu; schema=fbs

Flatbuffers file identifier
: SPDU


#### CAN Bus

MIME Type (minimal)
: application/x-automotive-bus; interface=stream; type=pdu; schema=fbs

MIME Type (extended)
: application/x-automotive-bus; interface=stream; type=pdu; schema=fbs; swc_id=1; ecu_id=2


##### Additional Properties

The following parameters can be encoded directly in the MIME Type string
or set with calls to `ncodec_config()`.

| Property | Type | Default |
| --- |--- |--- |
| swc_id | uint8_t | 0 (must be set for normal operation [^1]) |
| ecu_id | uint8_t | 0 |

[^1]: Message filtering on `swc_id` (i.e. filter if Tx Node = Rx Node) is
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
[==========] CODEC: Running 7 test(s).
[ RUN      ] test_trim
...
[==========] CODEC: 7 test(s) run.
[  PASSED  ] 7 test(s).
[==========] CAN FBS: Running 10 test(s).
...
[==========] CAN FBS: 10 test(s) run.
[  PASSED  ] 10 test(s).
[==========] PDU FBS: Running 14 test(s).
...
[==========] PDU FBS: 14 test(s) run.
[  PASSED  ] 14 test(s).
```



## Contribute

Please refer to the [CONTRIBUTING.md](../../../../CONTRIBUTING.md) file.


## License

Dynamic Simulation Environment Model C Library is open-sourced under the
Apache-2.0 license.

See the [LICENSE](../../../../LICENSE) and [NOTICE](../../../../NOTICE) files for details.


### Third Party Licenses

[Third Party Licenses](../../../../licenses/)
