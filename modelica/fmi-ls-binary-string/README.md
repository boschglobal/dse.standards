<!--
SPDX-FileCopyrightText: 2023 Robert Bosch GmbH

SPDX-License-Identifier: Apache-2.0
-->

# FMI Layered Standard Binary String


__Contents__
- [Introduction](#introduction)
- [Layered Standard Manifest File](#manifest)
- [FMU with Binary Strings](#binary_string)
- [Known Limitations of this Standard](#limitations)

---

<a name="introduction"></a>

## 1. Introduction

> **Note:** This layered standard applies on the Modelica FMI 2 Standard _only_.


### 1.1 Intent of this Document

FMUs may exchange binary data which represents serialized data objects. These data objects may contain embedded null characters ('\0') which prevents them from being handled as a null-terminated string, where the length of the string is inferred by the position of the null character (at the end of the string). Therefore, a serialized data object needs to be represented by a binary string where the length of the binary string is stored in a separate variable.

This layered standard describes how a binary string may be realised with an `fmi2String` variable and an associated `fmi2Integer` variable. The FMU Importer is thus able to represent serialized data objects with these combined variables.


### 1.2 Overview of the Approach

The general approach is as follows:

1. The FMU Exporter creates a binary string variable by associating an `fmi2String` variable with an `fmi2Integer` variable using an annotation (on the `fmi2String` variable). The associated `fmi2Integer` variable holds the length of the binary string.

2. Both the FMU Importer and Exporter consider the value assigned to an `fmi2String` variable, which represents a binary string, as a void pointer (i.e. casting to a void pointer from the char pointer type of `fmi2String`). The length of the binary string is maintained in the associated `fmi2Integer` variable.


### 1.3 Remarks regarding this Approach

This layered standard applies on the Modelica FMI 2 Standard _only_. The Modelica FMI 3 Standard introduces support for binary strings via the `fmi3Binary` type (and an associated variable holding the length of the `fmi3Binary` variable).



---
<a name="manifest"></a>

## 2. Layered Standard Manifest File

This layered standard defines additional capability flags:


| Attribute   | Description |
| ----------- | ----------- |
| version | Version of this layered standard which the FMU implements. |
| supportsBinaryStringLengthValueReference | Indicates that a Binary String may be created by annotating a String variable with the _Value Reference (VR)_ of an associated Integer variable. |
| supportsBinaryStringLengthNamedReference | Indicates that a Binary String may be created by annotating a String variable with the _Name_ of an associated Integer variable. |


The manifest schema may be found here: [schema/fmi-ls-binary-string.xsd](schema/fmi-ls-binary-string.xsd)



---
<a name="binary_string"></a>

## 3. FMU with Binary Strings

### 3.1 Configuration

A binary string is created by adding one of these annotations to a String variable. The annotation should reference a Integer variable.

| Annotation   | Description |
| ----------- | ----------- |
| dse.standards.fmi-ls-binary-string.vref | The length of the binary string contained in this String variable is maintained in the Integer variable specified by the variable reference (vref). |
| dse.standards.fmi-ls-binary-string.vname | The length of the binary string contained in this String variable is maintained in the Integer variable specified by the variable name (vname). |


### 3.2 Examples

__Example Variable Annotations__

```
<?xml version="1.0" encoding="UTF-8"?>
<fmiModelDescription fmiVersion="3.0" modelName="Example">
  <ModelVariables>
    <String name="binary-object" valueReference="1" causality="input"/>
        <Annotations>
            <Annotation type="dse.standards.fmi-ls-binary-string.vref">2<Annotation>
            <Annotation type="dse.standards.fmi-ls-binary-string.vname">binary-length<Annotation>
        <Annotations>
    </String>
    <Integer name="binary-length" valueReference="2" causality="input">
        <Start value="0" />
    </Integer>
  </ModelVariables>
```



---
<a name="limitations"></a>

## 4. Known Limitations of this Standard

### 4.1 Compatibility with FMI 2 Standard Only

This layered standard applies on the Modelica FMI 2 Standard _only_.
