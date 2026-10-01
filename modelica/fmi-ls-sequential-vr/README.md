<!--
SPDX-FileCopyrightText: 2026 Robert Bosch GmbH

SPDX-License-Identifier: Apache-2.0
-->

# Dynamic Simulation Environment - FMI Layered Standard Sequential VR for Get/Set Speed Boost


__Contents__
- [Introduction](#introduction)
- [FMI Layered Standard Sequential VR](#fmi-layered-standard-sequential-vr)
- [Layered Standard Manifest](#layered-standard-manifest)
- [License](#license)

## Introduction

An FMU with a large number of variables may use this Layered Standard to implement a Speed Boost
mechanism which may be used when marshalling data via its Get/Set API methods. The FMU aligns its
variable space in such a way that `memcpy()` can be used to copy variables directly from the
values parameter of the Get/Set FMI API into the FMUs variable space. On modern CPU architectures this
can result in a significant speed boost.


### Speed Boost with Sequential Value References.

> [!NOTE]
> Speedup factors are highly dependent on the host CPU architecture and compiler.
> Depending on your CPU's hardware vectorization capabilities, the speedup factor can be *even* higher.

| Scenario | Pseudo Code | Speedup Factor |
| :--- | :--- | :---: |
| Variables as array of pointers | `*(scalar[vr[i]]) = values[i]` | 0.3x |
| Variables as array of scalars | `scalar[vr[i]] = values[i]` | 1.0x |
| Variables as array, Sorted & Continuous | <pre>if (vr[0] == 0 &amp;&amp; vr[nvr - 1] == nvr - 1) {<br>&nbsp;&nbsp;&nbsp;&nbsp;#pragma gcc ivdep<br>&nbsp;&nbsp;&nbsp;&nbsp;for (size_t i = 0; i &lt; nvr; i++) {<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;scalar[i] = values[i];<br>&nbsp;&nbsp;&nbsp;&nbsp;}<br>&nbsp;&nbsp;&nbsp;&nbsp;return 0;<br>}</pre> | 8.0x |
| Direct Copy via `memcpy`. | `memcpy(&scalar[vr[0]-offset], values, nvr*sizeof(scalar));` | 16.0x |


### Repository File Layout

Project Structure

```text
dse.standards/
└── modelica/
    └── fmi-ls-sequential-vr/
        ├── README.md
        └── schema/
            ├── extra/
            │   └── com.github.boschglobal.dse.fmi-ls-sequential-vr/
            │       └── fmi-ls-manifest.xml
            ├── fmiLayeredStandardSequentialVrManifest.xsd
            ├── modelDescription_fmi2.xml
            └── modelDescription_fmi3.xml
```


---


## FMI Layered Standard Sequential VR

The FMU arranges its variables into arrays of the underlying variable type, and groups/orders those variables (within the array or in a map of arrays) according to their causality. Those variables are then
represented in the Model Description by an increasing sequence of Variable Reference values. When this Layered Standard is activated by the Importer, then the FMU is able to switch to the Get/Set Speed Boost algorithm and realise the speedup.


### Example FMU Model Description

```xml
<?xml version="1.0" encoding="UTF-8"?>
<fmiModelDescription
    fmiVersion="3.0"
    modelName="sequential_vr"
    version="1.0">

    <ModelVariables>
        <Float64 name="v1" valueReference="1" causality="input" />
        <Float64 name="v2" valueReference="2" causality="input" />
        <Float64 name="v3" valueReference="3" causality="input" />
        <Float64 name="v4" valueReference="4" causality="input" />
        ...
        <Float64 name="v499" valueReference="499" causality="input" />
        <Float64 name="v500" valueReference="500" causality="input" />
    </ModelVariables>

</fmiModelDescription>

```


### Example Set Implementation


```c
#define LS_SEQ_VR_ACTIVATE  100500001
#define LS_SEQ_VR_DISABLED  0
#define LS_SEQ_VR_ENABLED   1
#define INPUT_COUNT         500
#define INPUT_VR_OFFSET     1

static int ls_sequential_vr_enabled;
static fmi3Float64 input[INPUT_COUNT];

fmi3Status fmi3SetInt32(fmi3Instance instance,
    const fmi3ValueReference valueReferences[], size_t nValueReferences,
    const fmi3Int32 values[], size_t nValues)
{
    if (!m || nValueReferences == 0 || nValues == 0) return fmi3Error;

    for (size_t i = 0; i < nValueReferences; i++) {
        if (valueReferences[i] == LS_SEQ_VR_ACTIVATE) {
            ls_sequential_vr_enabled = values[i];
        }
    }
    return fmi3OK;
}

fmi3Status fmi3SetFloat64(fmi3Instance instance,
    const fmi3ValueReference valueReferences[], size_t nValueReferences,
    const fmi3Float64 values[], size_t nValues)
{
    if (nValues == 0 || nValueReferences == 0) return fmi3OK;

    // Sequential VR Speed Boost.
    if (ls_sequential_vr_enabled == LS_SEQ_VR_ENABLED && nValues <= INPUT_COUNT) {
        size_t offset = valueReferences[0] - INPUT_VR_OFFSET;
        memcpy(&input[offset], values, nValues*sizeof(fmi3Float64));
        return fmi3OK;
    }

    // Fallback to valueReferences traversal/lookup.
    for (size_t i = 0; i < nValueReferences; i++) {
        size_t idx = valueReferences[i] - INPUT_VR_OFFSET;
        if (idx < INPUT_COUNT) {
            input[idx] = values[i];
        }
    }
    return fmi3OK;
}

```


---


## Layered Standard Manifest

```xml
<?xml version="1.0" encoding="UTF-8"?>
<fmiLayeredStandardManifest
    xmlns:fmi-ls="http://fmi-standard.org/fmi-ls-manifest"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    fmi-ls:fmi-ls-name="com.github.boschglobal.dse.fmi-ls-sequential-vr"
    fmi-ls:fmi-ls-version="1.0.0"
    fmi-ls:fmi-ls-description="Bosch DSE Sequential VR feature activation configuration"
    xsi:noNamespaceSchemaLocation="https://github.com/boschglobal/dse.standards/modelica/fmi-ls-sequential-vr/schema/fmiLayeredStandardSequentialVrManifest.xsd">

    <SequentialVr>
        <ActivationParameter name="enableSequentialVR" type="Integer" value="1"/>
    </SequentialVr>

</fmiLayeredStandardManifest>
```


---


## License

This project is open-sourced under the Apache-2.0 license.
See the [LICENSE](../../LICENSE) and [NOTICE](../../NOTICE) files for details.
