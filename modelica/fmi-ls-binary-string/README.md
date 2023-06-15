<!--
SPDX-FileCopyrightText: 2023 Robert Bosch GmbH

SPDX-License-Identifier: Apache-2.0
-->

# FMI Layered Standard Binary String Backport


__Contents__
- [Introduction](#introduction)
- [Layered Standard Manifest File](#manifest)
- [FMU with Binary Strings](#binary_string)
- [Known Limitations of this Standard](#limitations)

---

<a name="introduction"></a>

## 1. Introduction

> **Note:** This layered standard proposal applies on the Modelica FMI 2 Standard _only_.


### 1.1 Intent of this Document

FMUs may exchange binary data which represents serialized data objects. These data objects may contain embedded null characters ('\0') which prevents them from being handled as a null-terminated string; where the length of the string is inferred by the position of the null character (at the end of the string). Therefore, a serialized data object needs to be represented by a binary string where the length of the binary string is stored in a separate variable.

This layered standard backports the Modelica FMI 3 Binary API functions into the FMI 2 header collection. All behaviour of the backported Binary API functions follow the existing definition provided by the Modelica FMI 3 Standard (for Binary variables).

Additionally, the mechanism for annotating an `fmi2String` variable, to indicate that the Binary API should be used to access that variable, is described.


### 1.2 Overview of the Approach

The general approach is as follows:

1. The FMU Exporter implements the [Binary API](headers/fmi2Binary.h) (`fmi2GetBinary` and `fmi2SetBinary`).

2. The FMU Exporter creates a binary string variable by annotating an `fmi2String` variable with the required annotation (see below).

3. The FMU Importer, when identifying the specified annotation, uses the [Binary API](headers/fmi2Binary.h) to access the binary string.


### 1.3 Remarks regarding this Approach

This layered standard applies on the Modelica FMI 2 Standard _only_.


---

<a name="manifest"></a>

## 2. Layered Standard Manifest File

This layered standard defines additional capability flags:


| Attribute   | Description |
| ----------- | ----------- |
| version | Version of this layered standard which the FMU implements. |
| supportsBinaryString | Indicates that the headers/fmi2Binary.h (`fmi2GetBinary` and `fmi2SetBinary`) is supported and should be used for annotated String variables. |


The manifest schema may be found here: [schema/fmi-ls-binary-string.xsd](schema/fmi-ls-binary-string.xsd)


---

<a name="binary_string"></a>

## 3. FMU with Binary Strings

### 3.1 Binary API

The Binary API is defined in this header file: [headers/fmi2Binary.h](headers/fmi2Binary.h). It can be placed alongside the existing FMI 2 header files. The API is derived from the FMI 3 header files where the Binary API was originally defined.


### 3.2 Configuration

A binary string is created by adding the following annotation to an existing String variable (under the tool `dse.standards.fmi-ls-binary-string`).

| Annotation   | Description |
| ----------- | ----------- |
| `binary` | The presence of this annotation on a String variable indicated that the [Binary API](headers/fmi2Binary.h) should be used. |


### 3.3 Examples

__Example Variable Annotations__

```
<?xml version="1.0" encoding="UTF-8"?>
<fmiModelDescription fmiVersion="2.0" modelName="Example">
  <ModelVariables>
    <String name="binary-object" valueReference="1" causality="input"/>
      <Annotations>
        <Tool name="dse.standards.fmi-ls-binary-string">
          <Annotation name="binary"><Annotation>
        </Tool>
      <Annotations>
    </String>
    <String name="binary-object" valueReference="2" causality="output"/>
      <Annotations>
        <Tool name="dse.standards.fmi-ls-binary-string">
          <Annotation name="binary"><Annotation>
        </Tool>
      <Annotations>
    </String>
  </ModelVariables>
```

> Note: In the above example a single binary object is presented by two String variables, one with causality of `input` and the other with causality of `output`. This enables a bidirectional buffer.


<a name="limitations"></a>

## 4. Known Limitations of this Standard

### 4.1 Compatibility with FMI 2 Standard Only

This layered standard applies on the Modelica FMI 2 Standard _only_.
