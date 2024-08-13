// Copyright 2024 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <fmi2Functions.h>
#include <fmi2FunctionTypes.h>
#include <fmi2TypesPlatform.h>
#include <dse/ncodec/codec.h>
#include <bus_topology.h>


#define UNUSED(x) ((void)x)


static void _log(const char* format, ...)
{
    printf("FMU: ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
    fflush(stdout);
}

char* _path_cat(const char* a, const char* b)
{
    if (a == NULL && b == NULL) return NULL;

    /* Caller will free. */
    int len = 2;  // '/' + NULL.
    len += (a) ? strlen(a) : 0;
    len += (b) ? strlen(b) : 0;
    char* path = calloc(len, sizeof(char));

    if (a && b) {
        snprintf(path, len, "%s/%s", a, b);
    } else {
        strncpy(path, a ? a : b, len - 1);
    }

    return path;
}

static char* __get_ncodec_bus_id(NCODEC* nc)
{
    int   index = 0;
    char* bus_id = NULL;
    while (index >= 0) {
        NCodecConfigItem ci = ncodec_stat(nc, &index);
        if (strcmp(ci.name, "bus_id") == 0) {
            if (ci.value) bus_id = strdup(ci.value);
            return bus_id;
        }
        index++;
    }
    return bus_id;
}

/* FMI2 FMU Instance Data */
typedef struct Fmu2InstanceData {
    /* FMI Instance Data. */
    struct {
        char*    name;
        fmi2Type type;
        char*    resource_location;
        char*    guid;
        bool     log_enabled;
        char*    model_xml_path;

        /* FMI Callbacks. */
        const fmi2CallbackFunctions callbacks;

        /* Storage for memory to be explicitly released. */
        char* save_resource_location;
    } instance;

    /* Bus Topology. */
    BusTopology* bus_topology;
    NCODEC*      bus_ncodec;
} Fmu2InstanceData;

fmi2Component fmi2Instantiate(fmi2String instance_name, fmi2Type fmu_type,
    fmi2String fmu_guid, fmi2String fmu_resource_location,
    const fmi2CallbackFunctions* functions, fmi2Boolean visible,
    fmi2Boolean logging_on)
{
    UNUSED(visible);

    /* Create the FMU Model Instance Data. */
    _log("Create the FMU Model Instance Data");
    Fmu2InstanceData* fmu = calloc(1, sizeof(Fmu2InstanceData));
    fmu->instance.name = strdup(instance_name);
    fmu->instance.type = fmu_type;
    fmu->instance.resource_location = strdup(fmu_resource_location);
    fmu->instance.guid = strdup(fmu_guid);
    fmu->instance.log_enabled = logging_on;
    if (functions) {
        memcpy((void*)&fmu->instance.callbacks, functions,
            sizeof(fmi2CallbackFunctions));
    }

    /**
     *  Calculate the offset needed to trim/correct the resource location.
     *  The resource location may take the forms:
     *
     *      file:///tmp/MyFMU/resources
     *      file:/tmp/MyFMU/resources
     *      /tmp/MyFMU/resources
     */
    fmu->instance.save_resource_location = fmu->instance.resource_location;
    int resource_path_offset = 0;
    if (strstr(fmu_resource_location, FILE_URI_SCHEME)) {
        resource_path_offset = strlen(FILE_URI_SCHEME);
    } else if (strstr(fmu_resource_location, FILE_URI_SHORT_SCHEME)) {
        resource_path_offset = strlen(FILE_URI_SHORT_SCHEME);
    }
    fmu->instance.resource_location += resource_path_offset;
    _log("Resource location: %s", fmu->instance.resource_location);
    fmu->instance.model_xml_path =
        _path_cat(fmu->instance.resource_location, "../modelDescription.yaml");
    _log("Model Description Path: %s", fmu->instance.model_xml_path);

    /* Return the created instance object. */
    return (fmi2Component)fmu;
}

// TODO Relocate this to a FMI String Variable (parameter).
#define MIMETYPE                                                               \
    "application/x-automotive-bus; "                                           \
    "interface=stream;type=frame;bus=can;schema=fbs;"                          \
    "bus_id=1;node_id=2;interface_id=3"

fmi2Status fmi2ExitInitializationMode(fmi2Component c)
{
    assert(c);
    Fmu2InstanceData* fmu = (Fmu2InstanceData*)c;

    /* Setup the Bus Topology. */
    _log("Create BusTopology object");
    fmu->bus_topology = bus_topology_create(fmu->instance.model_xml_path);
    fmu->bus_ncodec = ncodec_open(MIMETYPE, stream_create());
    char* bus_id = __get_ncodec_bus_id(fmu->bus_ncodec);
    if (bus_id) {
        _log("Configure Bus/Network : %s", bus_id);
        bus_topology_add(fmu->bus_topology, bus_id, fmu->bus_ncodec);
        free(bus_id);
    }

    return fmi2OK;
}

fmi2Status fmi2GetString(fmi2Component c, const fmi2ValueReference vr[],
    size_t nvr, fmi2String value[])
{
    assert(c);
    Fmu2InstanceData* fmu = (Fmu2InstanceData*)c;
    BusTopology*      bt = fmu->bus_topology;
    assert(bt);

    /* Get is Bus TX (ncodec/stream -> FMI String). */
    for (size_t i = 0; i < nvr; i++) {
        value[i] = NULL;
        size_t len = 0;
        bus_topology_tx(bt, vr[i], (uint8_t**)&value[i], &len);
    }
    return fmi2OK;
}

fmi2Status fmi2SetString(fmi2Component c, const fmi2ValueReference vr[],
    size_t nvr, const fmi2String value[])
{
    assert(c);
    Fmu2InstanceData* fmu = (Fmu2InstanceData*)c;
    BusTopology*      bt = fmu->bus_topology;
    assert(bt);

    /* Set is Bus RX (FMI String -> ncodec/stream). */
    bus_topology_reset(bt);
    for (size_t i = 0; i < nvr; i++) {
        if (value[i] == NULL) continue;
        bus_topology_rx(bt, vr[i], (uint8_t*)value[i], strlen(value[i]));
    }
    return fmi2OK;
}

// TODO Relocate this to a FMI String Variable (parameter).
#define GREETING "Hello World!"

fmi2Status fmi2DoStep(fmi2Component c, fmi2Real currentCommunicationPoint,
    fmi2Real    communicationStepSize,
    fmi2Boolean noSetFMUStatePriorToCurrentPoint)
{
    assert(c);
    UNUSED(currentCommunicationPoint);
    UNUSED(communicationStepSize);
    UNUSED(noSetFMUStatePriorToCurrentPoint);
    Fmu2InstanceData* fmu = (Fmu2InstanceData*)c;
    BusTopology*      bt = fmu->bus_topology;
    NCODEC*           ncodec = fmu->bus_ncodec;
    assert(bt);
    assert(ncodec);

    /* Read. */
    NCodecCanMessage msg = {};
    while (1) {
        int len = ncodec_read(ncodec, &msg);
        if (len < 0) break; /* No more messages. */
        _log("Network RX [%04x]: %s", msg.frame_id, msg.buffer);
    }
    ncodec_truncate(ncodec);

    /* Write. */
    ncodec_write(ncodec, &(struct NCodecCanMessage){ .frame_id = 42,
                             .buffer = (uint8_t*)GREETING,
                             .len = strlen(GREETING) });
    ncodec_flush(ncodec);

    return fmi2OK;
}

void fmi2FreeInstance(fmi2Component c)
{
    assert(c);
    Fmu2InstanceData* fmu = (Fmu2InstanceData*)c;

    bus_topology_destroy(fmu->bus_topology);
    free(fmu->instance.name);
    free(fmu->instance.guid);
    free(fmu->instance.save_resource_location);
    free(fmu->instance.model_xml_path);
    free(c);
}


/*
Unused parts of FMI interface
=============================

These functions are required to satisfy FMI packaging restrictions (i.e. these
functions need to exist in an FMU for some reason ...).
*/

const char* fmi2GetTypesPlatform(void)
{
    return fmi2TypesPlatform;
}

const char* fmi2GetVersion(void)
{
    return fmi2Version;
}

fmi2Status fmi2SetDebugLogging(fmi2Component c, fmi2Boolean loggingOn,
    size_t nCategories, const fmi2String categories[])
{
    assert(c);
    UNUSED(loggingOn);
    UNUSED(nCategories);
    UNUSED(categories);
    return fmi2OK;
}

fmi2Status fmi2SetupExperiment(fmi2Component c, fmi2Boolean toleranceDefined,
    fmi2Real tolerance, fmi2Real startTime, fmi2Boolean stopTimeDefined,
    fmi2Real stopTime)
{
    assert(c);
    UNUSED(toleranceDefined);
    UNUSED(tolerance);
    UNUSED(startTime);
    UNUSED(stopTimeDefined);
    UNUSED(stopTime);
    return fmi2OK;
}

fmi2Status fmi2EnterInitializationMode(fmi2Component c)
{
    assert(c);
    return fmi2OK;
}

fmi2Status fmi2GetReal(fmi2Component c, const fmi2ValueReference vr[],
    size_t nvr, fmi2Real value[])
{
    assert(c);
    UNUSED(vr);
    UNUSED(nvr);
    UNUSED(value);
    return fmi2OK;
}

fmi2Status fmi2GetInteger(fmi2Component c, const fmi2ValueReference vr[],
    size_t nvr, fmi2Integer value[])
{
    assert(c);
    UNUSED(vr);
    UNUSED(nvr);
    UNUSED(value);
    return fmi2OK;
}

fmi2Status fmi2GetBoolean(fmi2Component c, const fmi2ValueReference vr[],
    size_t nvr, fmi2Boolean value[])
{
    assert(c);
    UNUSED(vr);
    UNUSED(nvr);
    UNUSED(value);
    return fmi2OK;
}


fmi2Status fmi2SetReal(fmi2Component c, const fmi2ValueReference vr[],
    size_t nvr, const fmi2Real value[])
{
    assert(c);
    UNUSED(vr);
    UNUSED(nvr);
    UNUSED(value);
    return fmi2OK;
}

fmi2Status fmi2SetInteger(fmi2Component c, const fmi2ValueReference vr[],
    size_t nvr, const fmi2Integer value[])
{
    assert(c);
    UNUSED(vr);
    UNUSED(nvr);
    UNUSED(value);
    return fmi2OK;
}

fmi2Status fmi2SetBoolean(fmi2Component c, const fmi2ValueReference vr[],
    size_t nvr, const fmi2Boolean value[])
{
    assert(c);
    UNUSED(vr);
    UNUSED(nvr);
    UNUSED(value);
    return fmi2OK;
}

fmi2Status fmi2GetStatus(
    fmi2Component c, const fmi2StatusKind s, fmi2Status* value)
{
    assert(c);
    UNUSED(s);
    UNUSED(value);
    return fmi2OK;
}

fmi2Status fmi2GetRealStatus(
    fmi2Component c, const fmi2StatusKind s, fmi2Real* value)
{
    assert(c);
    UNUSED(s);
    UNUSED(value);
    return fmi2OK;
}

fmi2Status fmi2GetIntegerStatus(
    fmi2Component c, const fmi2StatusKind s, fmi2Integer* value)
{
    assert(c);
    UNUSED(s);
    UNUSED(value);
    return fmi2OK;
}

fmi2Status fmi2GetBooleanStatus(
    fmi2Component c, const fmi2StatusKind s, fmi2Boolean* value)
{
    assert(c);
    UNUSED(s);
    UNUSED(value);
    return fmi2OK;
}

fmi2Status fmi2GetStringStatus(
    fmi2Component c, const fmi2StatusKind s, fmi2String* value)
{
    assert(c);
    UNUSED(s);
    UNUSED(value);
    return fmi2OK;
}

fmi2Status fmi2SetRealInputDerivatives(fmi2Component c,
    const fmi2ValueReference vr[], size_t nvr, const fmi2Integer order[],
    const fmi2Real value[])
{
    assert(c);
    UNUSED(vr);
    UNUSED(nvr);
    UNUSED(order);
    UNUSED(value);
    return fmi2OK;
}

fmi2Status fmi2GetRealOutputDerivatives(fmi2Component c,
    const fmi2ValueReference vr[], size_t nvr, const fmi2Integer order[],
    fmi2Real value[])
{
    assert(c);
    UNUSED(vr);
    UNUSED(nvr);
    UNUSED(order);
    UNUSED(value);
    return fmi2OK;
}

fmi2Status fmi2CancelStep(fmi2Component c)
{
    assert(c);
    return fmi2OK;
}

fmi2Status fmi2GetFMUstate(fmi2Component c, fmi2FMUstate* FMUstate)
{
    assert(c);
    UNUSED(FMUstate);
    return fmi2OK;
}

fmi2Status fmi2SetFMUstate(fmi2Component c, fmi2FMUstate FMUstate)
{
    assert(c);
    UNUSED(FMUstate);
    return fmi2OK;
}

fmi2Status fmi2FreeFMUstate(fmi2Component c, fmi2FMUstate* FMUstate)
{
    assert(c);
    UNUSED(FMUstate);
    return fmi2OK;
}

fmi2Status fmi2SerializedFMUstateSize(
    fmi2Component c, fmi2FMUstate FMUstate, size_t* size)
{
    assert(c);
    UNUSED(FMUstate);
    UNUSED(size);
    return fmi2OK;
}

fmi2Status fmi2SerializeFMUstate(fmi2Component c, fmi2FMUstate FMUstate,
    fmi2Byte serializedState[], size_t size)
{
    assert(c);
    UNUSED(FMUstate);
    UNUSED(serializedState);
    UNUSED(size);
    return fmi2OK;
}

fmi2Status fmi2DeSerializeFMUstate(fmi2Component c,
    const fmi2Byte serializedState[], size_t size, fmi2FMUstate* FMUstate)
{
    assert(c);
    UNUSED(serializedState);
    UNUSED(size);
    UNUSED(FMUstate);
    return fmi2OK;
}

fmi2Status fmi2GetDirectionalDerivative(fmi2Component c,
    const fmi2ValueReference vUnknown_ref[], size_t nUnknown,
    const fmi2ValueReference vKnown_ref[], size_t nKnown,
    const fmi2Real dvKnown[], fmi2Real dvUnknown[])
{
    assert(c);
    UNUSED(vUnknown_ref);
    UNUSED(nUnknown);
    UNUSED(vKnown_ref);
    UNUSED(nKnown);
    UNUSED(dvKnown);
    UNUSED(dvUnknown);
    return fmi2OK;
}

fmi2Status fmi2Reset(fmi2Component c)
{
    assert(c);
    return fmi2OK;
}

fmi2Status fmi2Terminate(fmi2Component c)
{
    assert(c);
    return fmi2OK;
}
