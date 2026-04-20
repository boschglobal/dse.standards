<!--
SPDX-FileCopyrightText: 2026 Robert Bosch GmbH

SPDX-License-Identifier: Apache-2.0
-->

# Dynamic Simulation Environment - FMI Layered Standard PDU Net

__Contents__
- [Introduction](#introduction)
- [Layered Standard Manifest File](#layered-standard-manifest-file)
- [FMU with PDU Net](#fmu-with-pdu-net)
  - [Variation: PDU Net with Embedded SimBus](#variation-pdu-net-with-embedded-simbus)
  - [Variation: PDU Net with Embedded Simulation](#variation-pdu-net-with-embedded-simulation)
  - [Variation: PDU Net with Bus Topology](#variation-pdu-net-with-bus-topology)
- [Reference Material](#reference-material)
  - [Simulation Bus Reference Implementation](#simulation-bus-reference-implementation)
  - [Example CAN Network](#example-can-network)
  - [Example FlexRay Network](#example-flexray-network)
- [Known Limitations of this Standard](#known-limitations-of-this-standard)

---

## Introduction

PDU Net is a stream based network implementation which uses a Protocol Data Unit (PDU) as a container for data exchange.
The PDU Net is facilitated by the [PDU Stream schema][automotive-bus-schmea-pdu-stream] as implemented in the [Network Codec][dse-ncodec] (NCodec).



This layered standard integrates several technologies which are available from the following repos:

* FMI related integrations are provided by the [DSE FMI repo][dse-fmi]
* PDU Net is provided by the [DSE ModelC repo][dse-modelc]
* NCodec is provided by the [DSE NCodec repo][dse-ncodec]
* The schemas used by PDU Net are available in the [Automotive Bus Schema repo][automotive-bus-schmea-pdu-stream]


### Intent of this Document

FMUs may be interconnected via FMI string or binary variables to form a PDU-based network capable of representing network types such as CAN and FlexRay, as well as other data exchange mechanisms, including composite data records (C structs), memory blocks, and other PDU-based protocols.
Each node of a PDU Network is defined by a MIMEtype where the properties of the Network are configured.
The resultant PDU Network represents a Virtual Network which is operated via the normal exchange of FMI Variables - no additional Importer capability is required.

A PDU Network can be realised in FMI2 simulations using FMI String Variables, and in FMI3 simulations using either FMI String or Binary Variables. When using FMI String Variables the associated [dse-standards-fmi-ls-binary-to-text](../../modelica/fmi-ls-binary-to-text/README.md) Layered Standard is available to configure the binary-to-text encoding of messages being exchanged by the Importer.


### Overview of the Approach

The general approach is as follows:


### Remarks regarding this Approach

There are no specific remarks regarding this approach.



## Layered Standard Manifest File

## FMU with PDU Net

### Variation: PDU Net with Embedded SimBus

_Figure 1 demonstrates a PDU Network using NCodec objects which exchange data using a Simulation Bus._

![PDU Net with Embedded SimBus][pdunet-embedded-simbus]

__Figure 1: PDU Net with Embedded SimBus__


### Variation: PDU Net with Embedded Simulation

_Figure 2 demonstrates a PDU Network which bridges FMUs to a Simulation that is embedded in another FMU._

![PDU Net with Embedded Simulation][pdunet-embedded-simulation]

__Figure 2: PDU Net with Embedded Simulation__


### Variation: PDU Net with Bus Topology

_Figure 3 demonstrates a PDU Network using NCodec objects arranged in a Bus Topology._

![PDU Net with Bus Topology][pdunet-bus-topo]

__Figure 3: PDU Net with Bus Topology__


## Reference Material

### Simulation Bus Reference Implementation

### Example CAN Network

### Example FlexRay Network



## Known Limitations of this Standard

There are no limitations with the application of this Layered Standard to either FMI2 and/or FMI3 simulations.


<!-- links -->
[pdunet-embedded-simulation]: fmi-ls-pdunet-embedded-simulation.png
[pdunet-bus-topo]: fmi-ls-pdunet-bus-topology.png
[pdunet-embedded-simbus]: fmi-ls-pdunet-embedded-netbus.png
[dse-modelc]: https://github.com/boschglobal/dse.modelc/blob/main/dse/modelc/pdunet.h
[dse-ncodec]: https://github.com/boschglobal/dse.ncodec
[automotive-bus-schmea-pdu-stream]: https://github.com/boschglobal/automotive-bus-schema/blob/main/schemas/stream/pdu.fbs
[dse-fmi]: https://github.com/boschglobal/dse.fmi