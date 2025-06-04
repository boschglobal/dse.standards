// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#ifndef DSE_NCODEC_CODEC_H_
#define DSE_NCODEC_CODEC_H_

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>


/* DLL Interface visibility. */
#if defined _WIN32 || defined __CYGWIN__
#ifdef DLL_BUILD
#define DLL_PUBLIC __declspec(dllexport)
#else
#define DLL_PUBLIC __declspec(dllimport)
#endif /* DLL_BUILD */
#define DLL_PRIVATE
#else /* Linux */
#define DLL_PUBLIC  __attribute__((visibility("default")))
#define DLL_PRIVATE __attribute__((visibility("hidden")))
#endif /* _WIN32 || defined __CYGWIN__ */


/**
Network Codec
=============

A Network Codec has two interfaces: a Codec Interface which is used to
encode/decode message from a Model/Device (connected to a Network), and a
Stream Interface which is used to exchange the encoded messages with other
Model/Devices connected to the same Network.

The Network Codec API (codec.h & codec.c) provides the framework
for implementing both the Codec Interface and the Stream Interface.
A typical realisation of this scheme would be:

* Stream Implementation - provided by the Model Environment.
* Network Codec Implementation - provided by a Codec vendor.
* Model - using the Network Codec API to configure and use a Network, with
  the assistance of both the Network Codec and Stream implementations.


Component Diagram
-----------------
<div hidden>

```
@startuml ncodec-component

title Network Codec

package "Model Environment" {
    interface "Signal Interface" as Sif
    component "Model" as foo {
                component "Model" as Model
                interface "CodecVTable" as Cvt
                component "Codec" as Codec
        interface "StreamVTable" as Svt
    }
    component "Stream" as Stream
    interface "Binary Interface" as Bif
    component "Codec Lib" as lib
}

Sif )-down- Model
Model -right-( Cvt
Cvt -right- Codec
Codec -right-( Svt
Svt -right- Stream
Stream -down-( Bif

lib .up.> Codec

Model --> lib :ncodec_open()

center footer Dynamic Simulation Environment

@enduml
```

</div>

![](ncodec-component.png)


Example
-------

{{< readfile file="examples/ncodec_api.c" code="true" lang="c" >}}

*/

typedef struct {
} NCODEC;


/* Stream Interface */

#define NCODEC_EOF true

typedef enum NCodecStreamSeekOperation {
    NCODEC_SEEK_SET = 0,
    NCODEC_SEEK_CUR,
    NCODEC_SEEK_END,
    NCODEC_SEEK_RESET,
} NCodecStreamSeekOperation;

typedef enum NCodecStreamPosOperation {
    NCODEC_POS_UPDATE = 0,
    NCODEC_POS_NC,
} NCodecStreamPosOperation;

typedef size_t (*NCodecStreamRead)(
    NCODEC* nc, uint8_t** data, size_t* len, int32_t pos_op);
typedef size_t (*NCodecStreamWrite)(NCODEC* nc, uint8_t* data, size_t len);
typedef int64_t (*NCodecStreamSeek)(NCODEC* nc, size_t pos, int32_t op);
typedef int64_t (*NCodecStreamTell)(NCODEC* nc);
typedef int32_t (*NCodecStreamEof)(NCODEC* nc);
typedef int32_t (*NCodecStreamClose)(NCODEC* nc);

typedef struct NCodecStreamVTable {
    NCodecStreamRead  read;
    NCodecStreamWrite write;
    NCodecStreamSeek  seek;
    NCodecStreamTell  tell;
    NCodecStreamEof   eof;
    NCodecStreamClose close;
} NCodecStreamVTable;


/** CODEC Interface */

typedef struct NCodecConfigItem {
    const char* name;
    const char* value;
} NCodecConfigItem;

typedef void NCodecMessage; /* Generic message container. */


typedef int32_t NCodecLoad(const char* filename, const char* hint);
typedef NCODEC* NCodecOpen(const char* mime_type, NCodecStreamVTable* stream);
typedef NCODEC* NCodecCreate(const char* mime_type);

typedef int32_t (*NCodecConfig)(NCODEC* nc, NCodecConfigItem item);
typedef NCodecConfigItem (*NCodecStat)(NCODEC* nc, int32_t* index);
typedef int32_t (*NCodecWrite)(NCODEC* nc, NCodecMessage* msg);
typedef int32_t (*NCodecRead)(NCODEC* nc, NCodecMessage* msg);
typedef int32_t (*NCodecFlush)(NCODEC* nc);
typedef int32_t (*NCodecTruncate)(NCODEC* nc);
typedef void (*NCodecClose)(NCODEC* nc);

typedef struct NCodecVTable {
    NCodecConfig   config;
    NCodecStat     stat;
    NCodecWrite    write;
    NCodecRead     read;
    NCodecFlush    flush;
    NCodecTruncate truncate;
    NCodecClose    close;
} NCodecVTable;

typedef void (*NCodecTraceWrite)(NCODEC* nc, NCodecMessage* msg);
typedef void (*NCodecTraceRead)(NCODEC* nc, NCodecMessage* msg);

typedef struct NCodecTraceVTable {
    NCodecTraceWrite write;
    NCodecTraceRead  read;
} NCodecTraceVTable;


typedef struct NCodecInstance {
    const char*         mime_type;
    NCodecVTable        codec;
    NCodecStreamVTable* stream;
    /* Trace interface (optional). */
    NCodecTraceVTable   trace;
    /* Private reference data from API user (optional). */
    void* private;
} NCodecInstance;


/** NCODEC API - CAN Frame/Stream
    =============================

    Types relating to the implementaiton of the Stream/Frame interface of
    the NCodec API for transmission of CAN Messages.

    The root type is `NCodecCanMessage` which may be substitued for the
    `NCodecMessage` type when calling NCodec API methods (e.g.
   `ncodec_write()`).
*/

typedef enum NCodecCanFrameType {
    CAN_BASE_FRAME = 0,
    CAN_EXTENDED_FRAME = 1,
    CAN_FD_BASE_FRAME = 2,
    CAN_FD_EXTENDED_FRAME = 3,
} NCodecCanFrameType;

typedef struct NCodecCanMessage {
    uint32_t           frame_id;
    uint8_t*           buffer;
    size_t             len;
    NCodecCanFrameType frame_type;

    /* Reserved. */
    uint64_t __reserved__[2];

    /* Sender metadata (optional). */
    struct {
        /* RX node identification. */
        uint8_t bus_id;
        uint8_t node_id;
        uint8_t interface_id;
    } sender;

    /* Timing metadata (optional), values in nSec. */
    struct {
        uint64_t send; /* When the message is delivered to the Codec. */
        uint64_t arb;  /* When the message is sent by the Codec. */
        uint64_t recv; /* When the message is received from the Codec. */
    } timing;
} NCodecCanMessage;


/** NCODEC API - PDU/Stream
    =======================

    Types relating to the implementaiton of the Stream/PDU interface of
    the NCodec API.

    The root type is `NCodecPdu` which may be substitued for the `NCodecMessage`
    type when calling NCodec API methods (e.g. `ncodec_write()`).
*/

typedef enum NCodecPduCanFrameFormat {
    NCodecPduCanFrameFormatBase = 0,
    NCodecPduCanFrameFormatExtended = 1,
    NCodecPduCanFrameFormatFdBase = 2,
    NCodecPduCanFrameFormatFdExtended = 3,
} NCodecPduCanFrameFormat;

typedef enum NCodecPduCanFrameType {
    NCodecPduCanFrameTypeData = 0,
    NCodecPduCanFrameTypeRemote = 1,
    NCodecPduCanFrameTypeError = 2,
    NCodecPduCanFrameTypeOverload = 3,
} NCodecPduCanFrameType;

typedef struct NCodecPduCanMessageMetadata {
    NCodecPduCanFrameFormat frame_format;
    NCodecPduCanFrameType   frame_type;
    uint32_t                interface_id;
    uint32_t                network_id;
} NCodecPduCanMessageMetadata;


typedef enum {
    NCodecPduIpProtocolNone = 0,
    NCodecPduIpProtocolTcp = 6,
    NCodecPduIpProtocolUdp = 17,
} NCodecPduIpProtocol;

typedef enum {
    NCodecPduIpAddrNone = 0,
    NCodecPduIpAddrIPv4 = 1,
    NCodecPduIpAddrIPv6 = 2,
} NCodecPduIpAddr;

typedef enum {
    NCodecPduSoAdNone = 0,
    NCodecPduSoAdDoIP = 1,
    NCodecPduSoAdSomeIP = 2,
} NCodecPduSoAd;

typedef struct NCodecPduIpAddrV4 {
    uint32_t src_addr;
    uint32_t dst_addr;
} NCodecPduIpAddrV4;

typedef struct NCodecPduIpAddrV6 {
    uint16_t src_addr[8];
    uint16_t dst_addr[8];
} NCodecPduIpAddrV6;


typedef struct NCodecPduDoIpAdapter {
    uint8_t  protocol_version;
    uint16_t payload_type;
} NCodecPduDoIpAdapter;

typedef struct NCodecPduSomeIpAdapter {
    uint32_t message_id;
    uint32_t length;
    uint32_t request_id;
    uint8_t  protocol_version;
    uint8_t  interface_version;
    uint8_t  message_type;
    uint8_t  return_code;
} NCodecPduSomeIpAdapter;


typedef struct NCodecPduIpMessageMetadata {
    uint64_t eth_dst_mac;
    uint64_t eth_src_mac;
    uint16_t eth_ethertype;
    uint8_t  eth_tci_pcp;
    uint8_t  eth_tci_dei;
    uint16_t eth_tci_vid;

    NCodecPduIpProtocol ip_protocol;
    NCodecPduIpAddr     ip_addr_type;
    union {
        struct {
        } none;
        NCodecPduIpAddrV4 ip_v4;
        NCodecPduIpAddrV6 ip_v6;
    } ip_addr;
    uint16_t ip_src_port;
    uint16_t ip_dst_port;

    NCodecPduSoAd so_ad_type;
    union {
        struct {
        } none;
        NCodecPduDoIpAdapter   do_ip;
        NCodecPduSomeIpAdapter some_ip;
    } so_ad;
} NCodecPduIpMessageMetadata;


typedef struct NCodecPduStructMetadata {
    const char* type_name;
    const char* var_name;
    const char* encoding;
    uint16_t    attribute_aligned;
    bool        attribute_packed;
    const char* platform_arch;
    const char* platform_os;
    const char* platform_abi;
} NCodecPduStructMetadata;


typedef enum {
    NCodecPduTransportTypeNone = 0,
    NCodecPduTransportTypeCan = 1,
    NCodecPduTransportTypeIp = 2,
    NCodecPduTransportTypeStruct = 3,
} NCodecPduTransportType;

typedef struct NCodecPdu {
    uint32_t       id;
    const uint8_t* payload;
    size_t         payload_len;

    /* Sender identifying properties (optional), default values are taken
       from the stream MIME Type parameters. */
    uint32_t swc_id;
    uint32_t ecu_id;

    /* Transport Metadata. */
    NCodecPduTransportType transport_type;
    union {
        struct {
        } none;
        NCodecPduCanMessageMetadata can_message;
        NCodecPduIpMessageMetadata  ip_message;
        NCodecPduStructMetadata     struct_object;
    } transport;
} NCodecPdu;


/* Implemented by Codec. */
DLL_PUBLIC NCodecCreate ncodec_create;

/* Implemented by integrator. */
DLL_PUBLIC int32_t ncodec_load(const char* filename, const char* hint);
DLL_PUBLIC NCODEC* ncodec_open(
    const char* mime_type, NCodecStreamVTable* stream);

/* Provided by codec.c (in this package). */
DLL_PUBLIC void             ncodec_config(NCODEC* nc, NCodecConfigItem item);
DLL_PUBLIC NCodecConfigItem ncodec_stat(NCODEC* nc, int32_t* index);
DLL_PUBLIC int32_t          ncodec_write(NCODEC* nc, NCodecMessage* msg);
DLL_PUBLIC int32_t          ncodec_read(NCODEC* nc, NCodecMessage* msg);
DLL_PUBLIC int32_t          ncodec_flush(NCODEC* nc);
DLL_PUBLIC int32_t          ncodec_truncate(NCODEC* nc);
DLL_PUBLIC void             ncodec_close(NCODEC* nc);
DLL_PUBLIC int64_t          ncodec_seek(NCODEC* nc, size_t pos, int32_t op);
DLL_PUBLIC int64_t          ncodec_tell(NCODEC* nc);

#endif  // DSE_NCODEC_CODEC_H_
