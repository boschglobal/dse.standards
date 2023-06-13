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

> **Note:** This layered standard proposal applies on the Modelica FMI 2 Standard _only_.


### 1.1 Intent of this Document

FMUs may exchange binary data which represents serialized data objects. These data objects may contain embedded null characters ('\0') which prevents them from being handled as a null-terminated string, where the length of the string is inferred by the position of the null character (at the end of the string). Therefore, a serialized data object needs to be represented by a binary string where the length of the binary string is stored in a separate variable. Additionally, to support reuse of the buffer containing binary data, a second variable may be used to store the size of the buffer which contains the binary data.

This layered standard describes how a binary string may be realised with an `fmi2String` variable and associated `fmi2Integer` variables. The FMU Importer is thus able to represent serialized data objects with these combined variables.


### 1.2 Overview of the Approach

The general approach is as follows:

1. The FMU Exporter creates a binary string variable by associating an `fmi2String` variable with:
    * A variable holding the _length_ of the binary string. Represented by an `fmi2Integer` variable, and associated by using an annotation on the `fmi2String` variable.
    * (optional) A variable holding the _size_ of the buffer containing the binary string. Represented by an `fmi2Integer` variable, and associated by using an annotation on the `fmi2String` variable.

2. Both the FMU Importer and Exporter consider the value assigned to an `fmi2String` variable, which represents a binary string, as a void pointer (i.e. casting to a void pointer from the char pointer type of `fmi2String`). The _length_ of the binary string is maintained in the associated `fmi2Integer` variable.

3. Both the FMU Importer and Exporter may optionally share the buffer assigned to an `fmi2String` variable by exchanging the _size_ of the binary string in an associated `fmi2Integer` variable. In such a usage pattern, calling `fmi2SetString` implicitly gives ownership of the shared buffer to the Exporter (FMU), and calling `fmi2GetString` implicitly returns ownership of the shared buffer to the Importer (Master). Important behavioral aspects are as follows:
    * When either the Importer or Exporter has implicit ownership of the shared buffer they may reallocate or resize the buffer. The new address and size of the buffer should be reflected in the relevant variables.
    * When either the Importer or Exporter __does not have__ implicit ownership of the shared buffer, they __may not__ read, write or modify __any__ of the variables associated with a binary string - these should be considered as volatile and may change at anytime (especially in the case of buffer reallocation).


### 1.3 Remarks regarding this Approach

This layered standard applies on the Modelica FMI 2 Standard _only_.

The Modelica FMI 3 Standard introduces support for binary strings via the `fmi3Binary` type (and an associated variable holding the length of the `fmi3Binary` variable). There is no support for shared buffers in the FMI 3 Standard.



---
<a name="manifest"></a>

## 2. Layered Standard Manifest File

This layered standard defines additional capability flags:


| Attribute   | Description |
| ----------- | ----------- |
| version | Version of this layered standard which the FMU implements. |
| supportsBinaryStringLengthValueReference | Indicates that a Binary String may be created by annotating a String variable with the _Value Reference (VR)_ of an associated Integer variable. |
| supportsBinaryStringLengthNamedReference | Indicates that a Binary String may be created by annotating a String variable with the _Name_ of an associated Integer variable. |
| supportsBinaryStringSizeValueReference | Indicates that the buffer containing a Binary String may be shared between the Importer and Exporter by annotating a String variable with the _Value Reference (VR)_ of an associated Integer variable. |
| supportsBinaryStringSizeNamedReference | Indicates that the buffer containing a Binary String may be shared between the Importer and Exporter by annotating a String variable with the _Name_ of an associated Integer variable. |


The manifest schema may be found here: [schema/fmi-ls-binary-string.xsd](schema/fmi-ls-binary-string.xsd)



---
<a name="binary_string"></a>

## 3. FMU with Binary Strings

### 3.1 Configuration

A binary string is created by adding one of these annotations to a String variable. The annotation should reference a Integer variable.

| Annotation   | Description |
| ----------- | ----------- |
| dse.standards.fmi-ls-binary-string-length.vref | The length of the binary string contained in this String variable is maintained in the Integer variable specified by the variable reference (vref). |
| dse.standards.fmi-ls-binary-string-length.vname | The length of the binary string contained in this String variable is maintained in the Integer variable specified by the variable name (vname). |


Additionally, the buffer (or allocated memory) containing a binary string may be shared between the Importer and Exporter by adding one of the following annotations to a String variable. The annotation should reference a Integer variable.

| Annotation   | Description |
| ----------- | ----------- |
| dse.standards.fmi-ls-binary-string-size.vref | The size of the buffer holding the binary string contained in this String variable is maintained in the Integer variable specified by the variable reference (vref). |
| dse.standards.fmi-ls-binary-string-size.vname | The size of the buffer holding the binary string contained in this String variable is maintained in the Integer variable specified by the variable name (vname). |


### 3.2 Examples

__Example Variable Annotations__

```
<?xml version="1.0" encoding="UTF-8"?>
<fmiModelDescription fmiVersion="3.0" modelName="Example">
  <ModelVariables>
    <String name="binary-object" valueReference="1" causality="input"/>
        <Annotations>
            <Annotation type="dse.standards.fmi-ls-binary-string-length.vref">2<Annotation>
            <Annotation type="dse.standards.fmi-ls-binary-string-length.vname">binary-length<Annotation>
            <Annotation type="dse.standards.fmi-ls-binary-string-size.vref">3<Annotation>
            <Annotation type="dse.standards.fmi-ls-binary-string-size.vname">binary-size<Annotation>
        <Annotations>
    </String>
    <Integer name="binary-length" valueReference="2" causality="input">
        <Start value="0" />
    </Integer>
    <Integer name="binary-size" valueReference="3" causality="input">
        <Start value="0" />
    </Integer>
  </ModelVariables>
```

> Note: In the above example a causality of `input` is selected even though the data exchange is bidirectional. There is no appropriate causality in the FMI Standards for the usage pattern shown in this example (i.e. using a shared buffer between the Importer and Exporter for binary data).


---
<a name="limitations"></a>

## 4. Known Limitations of this Standard

### 4.1 Compatibility with FMI 2 Standard Only

This layered standard applies on the Modelica FMI 2 Standard _only_.


### 4.2 Indication of causality when sharing a buffer between Importer and Exporter

Causality cannot be defined for this case, where a variable is both input and output. The selection of `input` is seen as the best choice. Implementers of this layered standard can ignore the causality when sharing a buffer as the operation as described in this layered standard is already sufficiently specialized.
