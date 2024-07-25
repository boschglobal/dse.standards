<!--
SPDX-FileCopyrightText: 2024 Robert Bosch GmbH

SPDX-License-Identifier: Apache-2.0
-->

# Dynamic Simulation Environment - FMI Layered Standard Bus Topology


__Contents__
- [Introduction](#introduction)
- [Layered Standard Manifest File](#manifest)
- [FMU with Bus Topology to realise Network](#bus-topology)
- [Example Bus Topology for CAN](#example-network)
- [Known Limitations of this Standard](#limitations)

---

<a name="introduction"></a>

## 1. Introduction

### 1.1 Intent of this Document

FMUs may be interconnected via FMI String or Binary Variables to create a Bus Topology which represents a Network (e.g. CAN Bus). That Network may be further defined by a MIMEtype where the properties of the Network may be configured. The resultant Virtual Bus, or Virtual Network, is then operated with the exchange of FMI Variables - no additional Importer capability is required.

A Virtual Bus (or Network) can be realised in FMI2 simulations using FMI String Variables, and in FMI3 simulations using either FMI String or Binary Variables. When using FMI String Variables the associated [dse-standards-fmi-ls-binary-to-text](../../modelica/fmi-ls-binary-to-text/README.md) Layered Standard is available to configure the binary-to-text encoding of messages being exchanged by the Network.

This layered standard describes the mechanism for defining a bus topology with FMI String or Binary Variables.


### 1.2 Overview of the Approach

The general approach is as follows:

1. The Model Developer implements software functions which are capable of discovering FMI Variables which represent a Bus Topology, and then operating those FMI Variables collectively as a Network Interface to a Virtual Network. This means sending network data on FMI Variables annotated as the TX direction (`causality="output"`), and receiving network data from FMI Variables annotated as the RX direction (`causality="input"`).

2. The Simulation Integrator decides on a Bus Topology and creates/modifies FMU descriptions to include the necessary FMI Variables and annotations for that Bus Topology. Connections between FMU Variables which the Importer should connect are also defined.

3. The FMU Runtime configures its Network Interfaces by parsing its Model Definition for FMI Variables that have Bus Topology annotations. The FMU is then able to exchange Network Messages with other FMUs by using those configured Network Interfaces, while the FMI Importer facilitates the Virtual Network by exchanging FMI String or Binary Variables between FMUs.


### 1.3 Remarks regarding this Approach

There are no specific remarks regarding this approach.



---
<a name="manifest"></a>

## 2. Layered Standard Manifest File

This layered standard defines additional capability flags:


| Attribute   | Description |
| ----------- | ----------- |
| version                       | Version of this layered standard which the FMU implements.


The manifest schema may be found here: [schema/fmi-ls-bus-topology.xsd](schema/fmi-ls-bus-topology.xsd)



---
<a name="bus-topology"></a>

## 3. FMU with Bus Topology to realise Network

A Bus Topology is created by annotating FMI String or Binary Variables. Those annotations represent the Bus (`bus_id`) and the Node (`node_id`) of an FMU, and all FMUs combined represent a Virtual Network. An FMU may be connected to several Virtual Networks.

The Bus topology and resultant Virtual Network is affected with the exchange of FMI Variables by the Importer.

_Figure 1 shows the principle of a Bus Topology which realises a Virtual Network._

![FMU with Bus Topology to realise Network](fmi-ls-bus-topology.png)

__Figure 1: FMUs with Bus Topology to realise Network__


### 3.1 Configuration

A Bus Topology is specified by adding the following annotations to FMI String or Binary Variables. FMI Variables for all connected FMUs need to be defined in each FMU, both in the transmit and receive direction.


__Configuration FMI2__
> Note: annotations in FMI2 are made under the "Tool" grouping `dse.standards.fmi-ls-bus-topology`.

| Annotation | Description |
| ---------- | ----------- |
| `bus_id`     | Indicate the Bus Identifier that this FMI String or Binary Variable represents.
| `node_id`    | Indicate the Node Identifier assigned to this FMU for the Bus represented by this FMI String or Binary Variable.
| `node_count` | (optional) The number of Nodes which are configured for the Bus represented by this FMI String or Binary Variable.


__Configuration FMI3__

| Annotation | Description |
| ---------- | ----------- |
| `dse.standards.fmi-ls-bus-topology.bus_id`     | Indicate the Bus Identifier that this FMI String or Binary Variable represents.
| `dse.standards.fmi-ls-bus-topology.node_id`    | Indicate the Node Identifier assigned to this FMU for the Bus represented by this FMI String or Binary Variable.
| `dse.standards.fmi-ls-bus-topology.node_count` | (optional) The number of Nodes which are configured for the Bus represented by this FMI String or Binary Variable.

### 3.2 Examples

The following example shows the configuration of a basic network connection (TX/RX) using FMI String Variables and an ascii85 encoding.


__Example Configuration FMI2__

```xml
<?xml version="1.0" encoding="UTF-8"?>
<fmiModelDescription fmiVersion="2.0" modelName="VirtualECU">
  <ModelVariables>
    <String name="network_1_1_rx" valueReference="1" causality="input"/>
      <Annotations>
        <Tool name="dse.standards.fmi-ls-bus-topology">
          <Annotation name="bus_id">1<Annotation>
          <Annotation name="node_id">1<Annotation>
        </Tool>
      <Annotations>
    </String>
    <String name="network_1_1_tx" valueReference="2" causality="output"/>
      <Annotations>
        <Tool name="dse.standards.fmi-ls-bus-topology">
          <Annotation name="bus_id">1<Annotation>
          <Annotation name="node_id">1<Annotation>
        </Tool>
      <Annotations>
    </String>
    <String name="network_1_2_rx" valueReference="3" causality="input"/>
      <Annotations>
        <Tool name="dse.standards.fmi-ls-bus-topology">
          <Annotation name="bus_id">1<Annotation>
          <Annotation name="node_id">2<Annotation>
        </Tool>
      <Annotations>
    </String>
    <String name="network_1_2_tx" valueReference="4" causality="output"/>
      <Annotations>
        <Tool name="dse.standards.fmi-ls-bus-topology">
          <Annotation name="bus_id">1<Annotation>
          <Annotation name="node_id">2<Annotation>
        </Tool>
      <Annotations>
    </String>
    <String name="network_1_3_rx" valueReference="5" causality="input"/>
      <Annotations>
        <Tool name="dse.standards.fmi-ls-bus-topology">
          <Annotation name="bus_id">1<Annotation>
          <Annotation name="node_id">3<Annotation>
        </Tool>
      <Annotations>
    </String>
    <String name="network_1_3_tx" valueReference="6" causality="output"/>
      <Annotations>
        <Tool name="dse.standards.fmi-ls-bus-topology">
          <Annotation name="bus_id">1<Annotation>
          <Annotation name="node_id">3<Annotation>
        </Tool>
      <Annotations>
    </String>
  </ModelVariables>
```


__Example Configuration FMI3__

```xml
<?xml version="1.0" encoding="UTF-8"?>
<fmiModelDescription fmiVersion="3.0" modelName="VirtualECU">
  <ModelVariables>
    <String name="network_1_1_rx" valueReference="1" causality="input">
        <Annotations>
            <Annotation type="dse.standards.fmi-ls-bus-topology.bus_id">1<Annotation>
            <Annotation type="dse.standards.fmi-ls-bus-topology.node_id">1<Annotation>
        <Annotations>
    </String>
    <String name="network_1_1_tx" valueReference="2" causality="output">
        <Annotations>
            <Annotation type="dse.standards.fmi-ls-bus-topology.bus_id">1<Annotation>
            <Annotation type="dse.standards.fmi-ls-bus-topology.node_id">1<Annotation>
        <Annotations>
    </String>
    <String name="network_1_1_rx" valueReference="3" causality="input">
        <Annotations>
            <Annotation type="dse.standards.fmi-ls-bus-topology.bus_id">1<Annotation>
            <Annotation type="dse.standards.fmi-ls-bus-topology.node_id">2<Annotation>
        <Annotations>
    </String>
    <String name="network_1_1_tx" valueReference="4" causality="output">
        <Annotations>
            <Annotation type="dse.standards.fmi-ls-bus-topology.bus_id">1<Annotation>
            <Annotation type="dse.standards.fmi-ls-bus-topology.node_id">2<Annotation>
        <Annotations>
    </String>
    <String name="network_1_1_rx" valueReference="5" causality="input">
        <Annotations>
            <Annotation type="dse.standards.fmi-ls-bus-topology.bus_id">1<Annotation>
            <Annotation type="dse.standards.fmi-ls-bus-topology.node_id">3<Annotation>
        <Annotations>
    </String>
    <String name="network_1_1_tx" valueReference="6" causality="output">
        <Annotations>
            <Annotation type="dse.standards.fmi-ls-bus-topology.bus_id">1<Annotation>
            <Annotation type="dse.standards.fmi-ls-bus-topology.node_id">3<Annotation>
        <Annotations>
    </String>
  </ModelVariables>
```



---
<a name="example-network"></a>

## 4. Example Bus Topology for CAN

### 4.1 CAN Network with Network Codec based Virtual Network

_Figure 2 demonstrates a Bus Topology that represents a Network Codec based Virtual Network._

![Network Codec based Virtual Network](fmi-ls-bus-topology-ncodec-vbus.png)

__Figure 2: Network Codec based Virtual Network__


### 4.2 CAN Network with FMU based Virtual Network

_Figure 3 demonstrates a Bus Topology that represents an FMU based Virtual Network._

![FMU based Virtual Network](fmi-ls-bus-topology-fmu-vbus.png)

__Figure 3: FMU based Virtual Network__



---
<a name="limitations"></a>

## 4. Known Limitations of this Standard

There are no limitations with the application of this Layered Standard to either FMI2 and/or FMI3 simulation systems (or Importers).
