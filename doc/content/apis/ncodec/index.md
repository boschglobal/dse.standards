---
title: Network Codec API Reference
linkTitle: NCodec
---
## Network Codec


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


### Component Diagram

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


### Example


{{< readfile file="examples/ncodec_api.c" code="true" lang="c" >}}




## Typedefs

### NCodecCanMessage

```c
typedef struct NCodecCanMessage {
    uint32_t frame_id;
    uint8_t* buffer;
    size_t len;
    NCodecCanFrameType frame_type;
    uint64_t [2] __reserved__;
    struct {
        uint8_t bus_id;
        uint8_t node_id;
        uint8_t interface_id;
    } sender;
    struct {
        uint64_t send;
        uint64_t arb;
        uint64_t recv;
    } timing;
}
```

### NCodecConfigItem

```c
typedef struct NCodecConfigItem {
    const char* name;
    const char* value;
}
```

### NCodecInstance

```c
typedef struct NCodecInstance {
    const char* mime_type;
    NCodecVTable codec;
    NCodecStreamVTable* stream;
    NCodecTraceVTable trace;
    void* private;
}
```

### NCodecPdu

```c
typedef struct NCodecPdu {
    uint32_t id;
    const uint8_t* payload;
    size_t payload_len;
    uint32_t swc_id;
    uint32_t ecu_id;
    NCodecPduTransportType transport_type;
    struct {
        struct ;
        struct (anonymous struct at dse/ncodec/codec.h:356:9) none;
        NCodecPduCanMessageMetadata can_message;
        NCodecPduIpMessageMetadata ip_message;
    } transport;
}
```

### NCodecPduCanMessageMetadata

```c
typedef struct NCodecPduCanMessageMetadata {
    NCodecPduCanFrameFormat frame_format;
    NCodecPduCanFrameType frame_type;
    uint32_t interface_id;
    uint32_t network_id;
}
```

### NCodecPduDoIpAdapter

```c
typedef struct NCodecPduDoIpAdapter {
    uint8_t protocol_version;
    uint16_t payload_type;
}
```

### NCodecPduIpAddrV4

```c
typedef struct NCodecPduIpAddrV4 {
    uint32_t src_addr;
    uint32_t dst_addr;
}
```

### NCodecPduIpAddrV6

```c
typedef struct NCodecPduIpAddrV6 {
    uint16_t [8] src_addr;
    uint16_t [8] dst_addr;
}
```

### NCodecPduIpMessageMetadata

```c
typedef struct NCodecPduIpMessageMetadata {
    uint64_t eth_dst_mac;
    uint64_t eth_src_mac;
    uint16_t eth_ethertype;
    uint8_t eth_tci_pcp;
    uint8_t eth_tci_dei;
    uint16_t eth_tci_vid;
    NCodecPduIpProtocol ip_protocol;
    NCodecPduIpAddr ip_addr_type;
    struct {
        struct ;
        struct (anonymous struct at dse/ncodec/codec.h:319:9) none;
        NCodecPduIpAddrV4 ip_v4;
        NCodecPduIpAddrV6 ip_v6;
    } ip_addr;
    uint16_t ip_src_port;
    uint16_t ip_dst_port;
    NCodecPduSoAd so_ad_type;
    struct {
        struct ;
        struct (anonymous struct at dse/ncodec/codec.h:329:9) none;
        NCodecPduDoIpAdapter do_ip;
        NCodecPduSomeIpAdapter some_ip;
    } so_ad;
}
```

### NCodecPduSomeIpAdapter

```c
typedef struct NCodecPduSomeIpAdapter {
    uint32_t message_id;
    uint32_t length;
    uint32_t request_id;
    uint8_t protocol_version;
    uint8_t interface_version;
    uint8_t message_type;
    uint8_t return_code;
}
```

### NCodecStreamVTable

```c
typedef struct NCodecStreamVTable {
    NCodecStreamRead read;
    NCodecStreamWrite write;
    NCodecStreamSeek seek;
    NCodecStreamTell tell;
    NCodecStreamEof eof;
    NCodecStreamClose close;
}
```

### NCodecTraceVTable

```c
typedef struct NCodecTraceVTable {
    NCodecTraceWrite write;
    NCodecTraceRead read;
}
```

### NCodecVTable

```c
typedef struct NCodecVTable {
    NCodecConfig config;
    NCodecStat stat;
    NCodecWrite write;
    NCodecRead read;
    NCodecFlush flush;
    NCodecTruncate truncate;
    NCodecClose close;
}
```

## Functions

### ncodec_close

#### Parameters

nc (NCODEC*)
: Network Codec object.



### ncodec_config

Set an additional parameter on a Network Codec. If the parameter is already
specified in the MIMEtype of the codex, the value being set will take
priority over the value originally specified in the MIMEtype.

#### Parameters

nc (NCODEC*)
: Network Codec object.

item (NetworkConfigItem)
: The config item being set.



### ncodec_create

> Implemented in Codec library.

#### Parameters

mime_type (const char*)
: The MIMEtype specifier.

#### Returns

NCodecOpen (pointer)
: Returns a pointer to a codec function which can open the specified MIMEtype.

NULL
: This codec library does not support the specified MIMEtype.



### ncodec_flush

#### Parameters

nc (NCODEC*)
: Network Codec object.

#### Returns

0
: The Network Codec internal buffers were flushed to the connected stream.

-ENOSTR
: The object represented by `nc` does not represent a valid stream.

-ENOSR
: No stream resource has been configured.



### ncodec_load

> Implemented by integrator.

#### Parameters

filename (const char*)
: Name of the Network Codec library to load.

hint (const char*)
: Optional path from where to load the Network Codec.

#### Returns

0
: The Network Codec library way successfully loaded.

-1
: The Network Codec library could not be loaded. Inspect `errno` for more
  details.



### ncodec_open

> Implemented by integrator.

#### Parameters

name (const char*)
: A name associated with the codec instance being opened.

mime_type (const char*)
: The MIMEtype specifier.

#### Returns

NCODEC (pointer)
: Object representing the Network Codec.

NULL
: The Network Codec could not be created. Inspect `errno` for more details.

#### Error Conditions


Available by inspection of `errno`.

ELIBACC
: Static library(ies) not loaded.

EINVAL
: Stream parameter not valid.

ENODATA
: A Network Codec matching the MIMEtype could not be found.

ENOENT
: The `ncodec_create` was not found in any loaded static library.



### ncodec_read

Read messages from a Network Codex until the stream represented by the codec
is fully consumed (i.e. no more messages).

The codec owns the message buffer/memory returned by this function. Messages
should be consumed immediately within a sequence of calls to ncodec_read (e.g.
until ENOMSG), or duplicate the messages for later processing.

#### Parameters

nc (NCODEC*)
: Network Codec object.

msg (NCodecMessage*)
: (out) The message representation to write to the Network Codec. Caller owns
  the message buffer/memory. Message type is defined by the codec
implementation.

#### Returns

<int32_t>
: The number of bytes read from the Network Codec. Will be identical to the
  value returned in `msg.len`.
  Additional messages may remain on the Network Codec, after processing this
  message, repeat calls to `ncodec_read` until -ENOMSG is returned.

-ENOMSG (-42)
: No message is available from the Network Codec.

-ENOSTR (-60)
: The object represented by `nc` does not represent a valid stream.

-ENOSR (-63)
: No stream resource has been configured.

-EINVAL (-22)
: Bad `msg` argument.



### ncodec_seek

#### Parameters

nc (NCODEC*)
: Network Codec object.

pos (size_t)
: Seek position relative to the seek operation.

op (int32_t)
: Seek operation (NCodecStreamSeekOperation).

#### Returns

+ve
: The position in the underlying stream object after the seek operation.

-ENOSTR (-60)
: The object represented by `nc` does not represent a valid stream.

-EINVAL (-22)
: Bad `msg` argument.



### ncodec_stat

#### Parameters

nc (NCODEC*)
: Network Codec object.

index (int32_t*)
: (out) Index of the config item returned by this call. When there are no more
  config items to be returned, this value is set to -1 and an empty
  NetworkConfigItem object is returned.

#### Returns

NetworkConfigItem
: A config item.



### ncodec_tell

#### Parameters

nc (NCODEC*)
: Network Codec object.

#### Returns

+ve
: The current position in the underlying stream object.

-ENOSTR (-60)
: The object represented by `nc` does not represent a valid stream.

-ENOSR (-63)
: No stream resource has been configured.



### ncodec_truncate

#### Parameters

nc (NCODEC*)
: Network Codec object.

#### Returns

0
: The Network Codec internal buffers were truncated.

-ENOSTR
: The object represented by `nc` does not represent a valid stream.

-ENOSR
: No stream resource has been configured.



### ncodec_write

Write the provided message to the Network Codec object.

The caller owns the message buffer/memory and the codec implementation will
encode (i.e. duplicate) the content of that message buffer/memory during this
call.

#### Parameters

nc (NCODEC*)
: Network Codec object.

msg (NCodecMessage*)
: The message representation to write to the Network Codec. Caller owns the
  message buffer/memory. Message type is defined by the codec implementation.

#### Returns

+VE (int32_t)
: The number of bytes written to the Network Codec. Will be identical to the
  value provided in `msg.len`.

-ENOSTR (-60)
: The object represented by `nc` does not represent a valid stream.

-EINVAL (-22)
: Bad `msg` argument.



