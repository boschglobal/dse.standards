<!--
Copyright 2023 Robert Bosch GmbH

SPDX-License-Identifier: Apache-2.0
-->

# Dynamic Simulation Environment - Standards Extensions


## Introduction

Standards extensions (adaptations) supporting the Dynamic Simulation Environment (DSE) Core Platform.

```
dse.standards
├── doc                         Documentation and image sources.
├── licenses                    3rd party software licenses.
└── modelica                    Adaptations relating to the Modelica Association Standards.
    ├── fmi-ls-binary-codec     Binary Codec specification for FMI 2/3.
    ├── fmi-ls-binary-to-text   String encoding for binary data, for FMI 2/3.
    ├── fmi-ls-bus-topology     Bus Topologies and Virtual Bus/Networks, for FMI 2/3.
    └── fmi-ls-pdu-net          PDU Network providing Virtual Networks and Bus Models, for FMI 2/3.
```


## Reference Implementations

* **[Automotive Bus][ab_schema]** - Supporting schemas (w/ Flatbuffers).
* **[DSE NCodec][dse_ncodec]** - NCodec implementation
* **[DSE ModelC - PDU Net][dse_modelc_pdunet]** - PDU Net headers and associated code.
* **[DSE FMI - FMU][dse_fmi_fmu]** - Reference FMU implementation (and Importer).


## Standards Extensions

### Modelica FMI Layered Standards

The following Layered Standards are provided within the scope of the Dynamic Simulation Environment. These Layered Standards may be used to integrate solution components from the Dynamic Simulation Environment; such as the [FMI ModelC FMU](https://github.com/boschglobal/dse.fmi?tab=readme-ov-file#fmi-modelc-fmu) - which uses these Layered Standards to implement an ECU Network that exchanges CAN Bus Messages using only FMI String variables.

* **Dynamic Simulation Environment - FMI Layered Standard Binary Codec Selection**\
  (**[dse-standards-fmi-ls-binary-codec](modelica/fmi-ls-binary-codec/README.md)**)\
  Method for the selection of an FMI Binary Variable Codec at runtime using either; the Binary Variable `mimeType` field (immutable), or an associated FMI String Variable which contains the MIME Type specifier of that FMI Binary Variable (mutable).

* **Dynamic Simulation Environment - FMI Layered Standard for Binary to Text Encoding**\
  (**[dse-standards-fmi-ls-binary-to-text](modelica/fmi-ls-binary-to-text/README.md)**)\
  Method for the exchange of ***Binary Data over FMI String Variables*** by using a binary-to-text encoding.

* **Dynamic Simulation Environment - FMI Layered Standard for Bus Topology (Virtual Bus/Network)**\
  (**[dse-standards-fmi-ls-bus-topology](modelica/fmi-ls-bus-topology/README.md)**)\
  Method for the realisation of Bus Topology to implement Virtual Bus/Networks and exchange Network Messages (e.g. CAN Frames) ***using only FMI Binary and/or String Variables***.

* **Dynamic Simulation Environment - FMI Layered Standard for PDU Network (Virtual Networks and Bus Models)**\
  (**[dse-standards-fmi-ls-pdu-net][fmi-ls-pdu-net]**)\
  Method for the realisation of PDU based Virtual Networks and Bus Models, supporting all vECU Levels in composite simulations. Supports Automotive Networks (CAN, FlexRay, LIN, Ethernet) and ECU Networks (Signal, Struct, Symbol, XCP).



## Contribute

Please refer to the [CONTRIBUTING.md](./CONTRIBUTING.md) file.



## License

Dynamic Simulation Environment Standards is open-sourced under the Apache-2.0 license.
See the [LICENSE](LICENSE) and [NOTICE](./NOTICE) files for details.



<!--- DSE Standards Links --->
[fmi-ls-binary-codec]: modelica/fmi-ls-binary-codec/README.md
[fmi-ls-binary-to-text]: modelica/fmi-ls-binary-to-text/README.md
[fmi-ls-bus-topology]: modelica/fmi-ls-bus-topology/README.md
[fmi-ls-pdu-net]: modelica/fmi-ls-pdu-net/README.md


<!--- Repo Links --->
[ab_schema]: https://github.com/boschglobal/automotive-bus-schema
[dse_ncodec]: https://github.com/boschglobal/dse.ncodec
[dse_modelc_pdunet]: https://github.com/boschglobal/dse.modelc/dse/modelc/pdunet.h
[dse_fmi_fmu]: https://github.com/boschglobal/dse.fmi/dse/fmu


<!--- Reference Links --->
[ls-example-pull-1854]: https://github.com/modelica/fmi-standard/pull/1854
[ls-example-xcp]: https://github.com/modelica/fmi-ls-xcp
