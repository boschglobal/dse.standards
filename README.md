<!--
Copyright 2023 Robert Bosch GmbH

SPDX-License-Identifier: Apache-2.0
-->

# Dynamic Simulation Environment - Standards


## Introduction

Standards contributions supporting the Dynamic Simulation Environment (DSE) Core Platform.

```
L- modelica     Contributions relating to the [Modelica Association Standards](https://github.com/modelica).
L- doc          Documentation and image source.
```


## Standards

### Modelica FMI Layered Standards

* **FMI Layered Standard for Binary Strings** (**[fmi-ls-binary-string](modelica/fmi-ls-binary-string/README.md)**)\
  Mechanism for supporting Binary Strings (with embedded NULL characters) in an FMI 2 String.

* **FMI Layered Standard for Binary Codecs** (**[fmi-ls-binary-codec](modelica/fmi-ls-binary-codec/README.md)**)\
  Method for the selection of an FMI Binary Variable Codec at runtime using either; the Binary Variable `mimeType` field (immutable), or an associated FMI String Variable which contains the MIME Type specifier of that FMI Binary Variable (mutable).



## Contribute

Please refer to the [CONTRIBUTING.md](./CONTRIBUTING.md) file.



## License

Dynamic Simulation Environment Standards is open-sourced under the Apache-2.0 license.
See the [LICENSE](LICENSE) and [NOTICE](./NOTICE) files for details.



## References

* FMI Layered Standard examples and schemas:
  * https://github.com/modelica/fmi-standard/pull/1854
  * https://github.com/modelica/fmi-ls-xcp