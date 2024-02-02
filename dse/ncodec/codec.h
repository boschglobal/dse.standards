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
#endif
#define DLL_PRIVATE
#else  // Linux
#define DLL_PUBLIC  __attribute__((visibility("default")))
#define DLL_PRIVATE __attribute__((visibility("hidden")))
#endif  // _WIN32


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
    NCODEC* nc, uint8_t** data, size_t* len, int pos_op);
typedef size_t (*NCodecStreamWrite)(NCODEC* nc, uint8_t* data, size_t len);
typedef int (*NCodecStreamSeek)(NCODEC* nc, size_t pos, int op);
typedef size_t (*NCodecStreamTell)(NCODEC* nc);
typedef int (*NCodecStreamEof)(NCODEC* nc);
typedef int (*NCodecStreamClose)(NCODEC* nc);

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


typedef int     NCodecLoad(const char* filename, const char* hint);
typedef NCODEC* NCodecOpen(const char* mime_type, NCodecStreamVTable* stream);
typedef NCODEC* NCodecCreate(const char* mime_type);

typedef int (*NCodecConfig)(NCODEC* nc, NCodecConfigItem item);
typedef NCodecConfigItem (*NCodecStat)(NCODEC* nc, int* index);
typedef int (*NCodecWrite)(NCODEC* nc, NCodecMessage* msg);
typedef int (*NCodecRead)(NCODEC* nc, NCodecMessage* msg);
typedef int (*NCodecFlush)(NCODEC* nc);
typedef int (*NCodecTruncate)(NCODEC* nc);
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


typedef struct NCodecInstance {
    const char*         mime_type;
    NCodecVTable        codec;
    NCodecStreamVTable* stream;
} NCodecInstance;


/** NCODEC API */

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
} NCodecCanMessage;


/* Implemented by Codec. */
DLL_PUBLIC NCodecCreate ncodec_create;

/* Implemented by integrator. */
DLL_PUBLIC int     ncodec_load(const char* filename, const char* hint);
DLL_PUBLIC NCODEC* ncodec_open(
    const char* mime_type, NCodecStreamVTable* stream);

/* Provided by codec.c (in this package). */
DLL_PUBLIC void             ncodec_config(NCODEC* nc, NCodecConfigItem item);
DLL_PUBLIC NCodecConfigItem ncodec_stat(NCODEC* nc, int* index);
DLL_PUBLIC int              ncodec_write(NCODEC* nc, NCodecMessage* msg);
DLL_PUBLIC int              ncodec_read(NCODEC* nc, NCodecMessage* msg);
DLL_PUBLIC int              ncodec_flush(NCODEC* nc);
DLL_PUBLIC int              ncodec_truncate(NCODEC* nc);
DLL_PUBLIC void             ncodec_close(NCODEC* nc);


#endif  // DSE_NCODEC_CODEC_H_
