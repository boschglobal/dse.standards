// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include "codec.h"  // NOLINT


/**
ncodec_load
===========

> Implemented by integrator.

Parameters
----------
filename (const char*)
: Name of the Network Codec library to load.

hint (const char*)
: Optional path from where to load the Network Codec.

Returns
-------
0
: The Network Codec library way successfully loaded.

-1
: The Network Codec library could not be loaded. Inspect `errno` for more
  details.
*/
extern int ncodec_load(const char* filename, const char* hint);


/**
ncodec_open
===========

> Implemented by integrator.

Parameters
----------
name (const char*)
: A name associated with the codec instance being opened.

mime_type (const char*)
: The MIMEtype specifier.

Returns
-------
NCODEC (pointer)
: Object representing the Network Codec.

NULL
: The Network Codec could not be created. Inspect `errno` for more details.

Error Conditions
----------------

Available by inspection of `errno`.

ELIBACC
: Static library(ies) not loaded.

EINVAL
: Stream parameter not valid.

ENODATA
: A Network Codec matching the MIMEtype could not be found.

ENOENT
: The `ncodec_create` was not found in any loaded static library.
*/
extern NCODEC* ncodec_open(const char* mime_type, NCodecStreamVTable* stream);


/**
ncodec_create
=============

> Implemented in Codec library.

Parameters
----------
mime_type (const char*)
: The MIMEtype specifier.

Returns
-------
NCodecOpen (pointer)
: Returns a pointer to a codec function which can open the specified MIMEtype.

NULL
: This codec library does not support the specified MIMEtype.
*/
extern NCODEC* ncodec_create(const char* mime_type);


/**
ncodec_config
=============

Set an additional parameter on a Network Codec. If the parameter is already
specified in the MIMEtype of the codex, the value being set will take
priority over the value originally specified in the MIMEtype.

Parameters
----------
nc (NCODEC*)
: Network Codec object.

item (NetworkConfigItem)
: The config item being set.
*/
inline void ncodec_config(NCODEC* nc, NCodecConfigItem item)
{
    NCodecInstance* _nc = (NCodecInstance*)nc;
    if (_nc && _nc->codec.config) {
        _nc->codec.config(nc, item);
    }
}


/**
ncodec_stat
===========

Parameters
----------
nc (NCODEC*)
: Network Codec object.

index (int*)
: (out) Index of the config item returned by this call. When there are no more
  config items to be returned, this value is set to -1 and an empty
  NetworkConfigItem object is returned.

Returns
-------
NetworkConfigItem
: A config item.
*/
inline NCodecConfigItem ncodec_stat(NCODEC* nc, int* index)
{
    NCodecInstance* _nc = (NCodecInstance*)nc;
    if (_nc && _nc->codec.stat) {
        return _nc->codec.stat(nc, index);
    } else {
        NCodecConfigItem _ = {};
        return _;
    }
}


/**
ncodec_write
============

Write the provided message to the Network Codec object.

The caller owns the message buffer/memory and the codec implementation will
encode (i.e. duplicate) the content of that message buffer/memory during this
call.

Parameters
----------
nc (NCODEC*)
: Network Codec object.

msg (NCodecMessage*)
: The message representation to write to the Network Codec. Caller owns the
  message buffer/memory.

Returns
-------
+VE (int)
: The number of bytes written to the Network Codec. Will be identical to the
  value provided in `msg.len`.

-ENOSTR
: The object represented by `nc` does not represent a valid stream.

-EINVAL
: Bad `msg` argument.
*/
inline int ncodec_write(NCODEC* nc, NCodecMessage* msg)
{
    NCodecInstance* _nc = (NCodecInstance*)nc;
    if (_nc && _nc->codec.write) {
        return _nc->codec.write(nc, msg);
    } else {
        return -ENOSTR;
    }
}


/**
ncodec_read
===========

Read messages from a Network Codex until the stream represented by the codec
is fully consumed (i.e. no more messages).

The codec owns the message buffer/memory returned by this function. Messages
should be consumed immediately within a sequence of calls to ncodec_read (e.g.
until ENOMSG), or duplicate the messages for later processing.

Parameters
----------
nc (NCODEC*)
: Network Codec object.

msg (NCodecMessage*)
: (out) The message representation to write to the Network Codec. Caller owns
  the message buffer/memory.

Returns
-------
<int>
: The number of bytes read from the Network Codec. Will be identical to the
  value returned in `msg.len`.
  Additional messages may remain on the Network Codec, after processing this
  message, repeat calls to `ncodec_read` until -ENOMSG is returned.

-ENOMSG
: No message is available from the Network Codec.

-ENOSTR
: The object represented by `nc` does not represent a valid stream.

-ENOSR
: No stream resource has been configured.

-EINVAL
: Bad `msg` argument.
*/
inline int ncodec_read(NCODEC* nc, NCodecMessage* msg)
{
    NCodecInstance* _nc = (NCodecInstance*)nc;
    if (_nc && _nc->codec.read) {
        return _nc->codec.read(nc, msg);
    } else {
        msg = NULL;
        return -ENOMSG;
    }
}


/**
ncodec_flush
============

Parameters
----------
nc (NCODEC*)
: Network Codec object.

Returns
-------
0
: The Network Codec internal buffers were flushed to the connected stream.

-ENOSTR
: The object represented by `nc` does not represent a valid stream.

-ENOSR
: No stream resource has been configured.
*/
inline int ncodec_flush(NCODEC* nc)
{
    NCodecInstance* _nc = (NCodecInstance*)nc;
    if (_nc && _nc->codec.flush) {
        return _nc->codec.flush(nc);
    } else {
        return -ENOSTR;
    }
}


/**
ncodec_truncate
===============

Parameters
----------
nc (NCODEC*)
: Network Codec object.

Returns
-------
0
: The Network Codec internal buffers were truncated.

-ENOSTR
: The object represented by `nc` does not represent a valid stream.

-ENOSR
: No stream resource has been configured.
*/
inline int ncodec_truncate(NCODEC* nc)
{
    NCodecInstance* _nc = (NCodecInstance*)nc;
    if (_nc && _nc->codec.truncate) {
        return _nc->codec.truncate(nc);
    } else {
        return -ENOSTR;
    }
}


/**
ncodec_close
============

Parameters
----------
nc (NCODEC*)
: Network Codec object.
*/
inline void ncodec_close(NCODEC* nc)
{
    NCodecInstance* _nc = (NCodecInstance*)nc;
    if (_nc && _nc->codec.close) {
        _nc->codec.close(nc);
    }
}
