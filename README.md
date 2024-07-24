<!--
Copyright 2023 Robert Bosch GmbH

SPDX-License-Identifier: Apache-2.0
-->

# Dynamic Simulation Environment - Standards Extensions


## Introduction

Standards extensions (adaptations) supporting the Dynamic Simulation Environment (DSE) Core Platform.

```
L- doc                      Documentation and image sources.
L- dse                      DSE related implementations (for reuse).
  L- ncodec                 Network Codec API, implementation of Binary Codec.
L- licenses                 3rd party software licenses.
L- modelica                 Adaptations relating to the Modelica Association Standards.
  L- fmi-ls-binary-codec    Binary Codec specification for FMI 2/3.
  L- fmi-ls-binary-to-text  String encoding for binary data, for FMI 2/3.
```



## Reference Implementations

### Network Codec API

* **[DSE NCodec](dse/ncodec/README.md)** supporting schemas:
  * [Automotive Bus](dse/ncodec/libs/automotive-bus/README.md) - CAN Bus (w. Flatbuffers based serialisation schema).



## Standards Extensions

### Modelica FMI Layered Standards

* **Dynamic Simulation Environment - FMI Layered Standard Binary Codec Selection** (**[dse-standards-fmi-ls-binary-codec](modelica/fmi-ls-binary-codec/README.md)**)\
  Method for the selection of an FMI Binary Variable Codec at runtime using either; the Binary Variable `mimeType` field (immutable), or an associated FMI String Variable which contains the MIME Type specifier of that FMI Binary Variable (mutable).

* **Dynamic Simulation Environment - FMI Layered Standard for Binary to Text Encoding** (**[dse-standards-fmi-ls-binary-to-text](modelica/fmi-ls-binary-to-text/README.md)**)\
  Method for the exchange of ***Binary Data over FMI String Variables*** by using a binary-to-text encoding.



## Contribute

Please refer to the [CONTRIBUTING.md](./CONTRIBUTING.md) file.



## License

Dynamic Simulation Environment Standards is open-sourced under the Apache-2.0 license.
See the [LICENSE](LICENSE) and [NOTICE](./NOTICE) files for details.



## References

* [Automotive Bus Schema](https://github.com/boschglobal/automotive-bus-schema)
* FMI Layered Standard examples and schemas:
  * https://github.com/modelica/fmi-standard/pull/1854
  * https://github.com/modelica/fmi-ls-xcp
