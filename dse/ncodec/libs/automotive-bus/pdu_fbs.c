// Copyright 2025 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <dse/ncodec/codec.h>
#include <automotive-bus/codec.h>
#include <automotive_bus_schema/stream/pdu_builder.h>


#undef ns
#define ns(x) FLATBUFFERS_WRAP_NAMESPACE(AutomotiveBus_Stream_Pdu, x)


static void initialize_stream(ABCodecInstance* nc)
{
    if (nc->fbs_stream_initalized) return;

    flatcc_builder_t* B = &nc->fbs_builder;
    flatcc_builder_reset(B);
    ns(Stream_start_as_root_with_size(B));
    ns(Stream_pdus_start(B));
    nc->fbs_stream_initalized = true;
}


static void reset_stream(ABCodecInstance* nc)
{
    if (nc->fbs_stream_initalized == false) return;

    flatcc_builder_t* B = &nc->fbs_builder;
    flatcc_builder_reset(B);
    nc->fbs_stream_initalized = false;
}


static void finalize_stream(
    ABCodecInstance* nc, uint8_t** buffer, size_t* length)
{
    if (nc->fbs_stream_initalized == false) {
        *buffer = NULL;
        *length = 0;
        return;
    }

    flatcc_builder_t* B = &nc->fbs_builder;
    ns(Stream_pdus_end(B));
    ns(Stream_end_as_root(B));
    *buffer = flatcc_builder_finalize_buffer(B, length);
    reset_stream(nc);
}


static uint32_t _emit_can_message_metadata(flatcc_builder_t* B, NCodecPdu* _pdu)
{
    NCodecPduCanMessageMetadata* can = &_pdu->transport.can_message;
    ns(CanMessageMetadata_start(B));
    ns(CanMessageMetadata_message_format_add(B, can->frame_format));
    ns(CanMessageMetadata_frame_type_add(B, can->frame_type));
    ns(CanMessageMetadata_interface_id_add(B, can->interface_id));
    ns(CanMessageMetadata_network_id_add(B, can->network_id));
    return ns(CanMessageMetadata_end(B));
}

static uint32_t _emit_ip_addr_v4(flatcc_builder_t* B, NCodecPdu* _pdu)
{
    NCodecPduIpAddrV4* addr = &_pdu->transport.ip_message.ip_addr.ip_v4;
    ns(IpV4_start(B));
    ns(IpV4_src_addr_add(B, addr->src_addr));
    ns(IpV4_dst_addr_add(B, addr->dst_addr));
    return ns(IpV4_end(B));
}

static uint32_t _emit_ip_addr_v6(flatcc_builder_t* B, NCodecPdu* _pdu)
{
    NCodecPduIpAddrV6* addr = &_pdu->transport.ip_message.ip_addr.ip_v6;
    ns(IpV6_start(B));
    ns(IpV6_src_addr_create(B, addr->src_addr[0], addr->src_addr[1],
        addr->src_addr[2], addr->src_addr[3], addr->src_addr[4],
        addr->src_addr[5], addr->src_addr[6], addr->src_addr[7]));
    ns(IpV6_dst_addr_create(B, addr->dst_addr[0], addr->dst_addr[1],
        addr->dst_addr[2], addr->dst_addr[3], addr->dst_addr[4],
        addr->dst_addr[5], addr->dst_addr[6], addr->dst_addr[7]));
    return ns(IpV6_end(B));
}

static uint32_t _emit_do_ip(flatcc_builder_t* B, NCodecPdu* _pdu)
{
    NCodecPduDoIpAdapter* a = &_pdu->transport.ip_message.so_ad.do_ip;
    ns(DoIpMetadata_start(B));
    ns(DoIpMetadata_protocol_version_add(B, a->protocol_version));
    ns(DoIpMetadata_payload_type_add(B, a->payload_type));
    return ns(DoIpMetadata_end(B));
}

static uint32_t _emit_some_ip(flatcc_builder_t* B, NCodecPdu* _pdu)
{
    NCodecPduSomeIpAdapter* a = &_pdu->transport.ip_message.so_ad.some_ip;
    ns(SomeIpMetadata_start(B));
    ns(SomeIpMetadata_message_id_add(B, a->message_id));
    ns(SomeIpMetadata_length_add(B, a->length));
    ns(SomeIpMetadata_request_id_add(B, a->request_id));
    ns(SomeIpMetadata_protocol_version_add(B, a->protocol_version));
    ns(SomeIpMetadata_interface_version_add(B, a->interface_version));
    ns(SomeIpMetadata_message_type_add(B, a->message_type));
    ns(SomeIpMetadata_return_code_add(B, a->return_code));
    return ns(SomeIpMetadata_end(B));
}

static uint32_t _emit_ip_message_metadata(flatcc_builder_t* B, NCodecPdu* _pdu)
{
    NCodecPduIpMessageMetadata* ip = &_pdu->transport.ip_message;
    ns(IpV4_ref_t) ip_addr_v4 = 0;
    ns(IpV6_ref_t) ip_addr_v6 = 0;
    ns(DoIpMetadata_ref_t) do_ip_metadata = 0;
    ns(SomeIpMetadata_ref_t) some_ip_metadata = 0;

    switch (ip->ip_addr_type) {
    case NCodecPduIpAddrIPv4:
        ip_addr_v4 = _emit_ip_addr_v4(B, _pdu);
        break;
    case NCodecPduIpAddrIPv6:
        ip_addr_v6 = _emit_ip_addr_v6(B, _pdu);
        break;
    default:
        break;
    }

    switch (ip->so_ad_type) {
    case NCodecPduSoAdDoIP:
        do_ip_metadata = _emit_do_ip(B, _pdu);
        break;
    case NCodecPduSoAdSomeIP:
        some_ip_metadata = _emit_some_ip(B, _pdu);
        break;
    default:
        break;
    }

    ns(IpMessageMetadata_start(B));
    ns(IpMessageMetadata_eth_dst_mac_add(B, ip->eth_dst_mac));
    ns(IpMessageMetadata_eth_src_mac_add(B, ip->eth_src_mac));
    ns(IpMessageMetadata_eth_ethertype_add(B, ip->eth_ethertype));
    ns(IpMessageMetadata_eth_tci_pcp_add(B, ip->eth_tci_pcp));
    ns(IpMessageMetadata_eth_tci_dei_add(B, ip->eth_tci_dei));
    ns(IpMessageMetadata_eth_tci_vid_add(B, ip->eth_tci_vid));
    if (ip_addr_v4) {
        ns(IpMessageMetadata_ip_addr_v4_add(B, ip_addr_v4));
    } else if (ip_addr_v6) {
        ns(IpMessageMetadata_ip_addr_v6_add(B, ip_addr_v6));
    }
    ns(IpMessageMetadata_ip_protocol_add(B, ip->ip_protocol));
    ns(IpMessageMetadata_ip_src_port_add(B, ip->ip_src_port));
    ns(IpMessageMetadata_ip_dst_port_add(B, ip->ip_dst_port));
    if (do_ip_metadata) {
        ns(IpMessageMetadata_adapter_do_ip_add(B, do_ip_metadata));
    } else if (some_ip_metadata) {
        ns(IpMessageMetadata_adapter_some_ip_add(B, some_ip_metadata));
    }
    return ns(IpMessageMetadata_end(B));
}


int32_t pdu_write(NCODEC* nc, NCodecPdu* pdu)
{
    ABCodecInstance* _nc = (ABCodecInstance*)nc;
    NCodecPdu*       _pdu = (NCodecPdu*)pdu;
    if (_nc == NULL) return -ENOSTR;
    if (_pdu == NULL) return -EINVAL;
    if (_nc->c.stream == NULL) return -ENOSR;

    uint32_t swc_id = _pdu->swc_id ? _pdu->swc_id : _nc->swc_id;
    uint32_t ecu_id = _pdu->ecu_id ? _pdu->ecu_id : _nc->ecu_id;

    flatcc_builder_t* B = &_nc->fbs_builder;
    initialize_stream(_nc);
    ns(CanMessageMetadata_ref_t) can_message_metadata = 0;
    ns(IpMessageMetadata_ref_t) ip_message_metadata = 0;

    /* Encode the PDU. */
    // Transport Table
    switch (_pdu->transport_type) {
    case NCodecPduTransportTypeCan: {
        can_message_metadata = _emit_can_message_metadata(B, _pdu);
    } break;
    case NCodecPduTransportTypeIp: {
        ip_message_metadata = _emit_ip_message_metadata(B, _pdu);
    } break;
    default:
        break;
    }

    // PDU Table
    ns(Stream_pdus_push_start(B));
    ns(Pdu_id_add(B, _pdu->id));
    ns(Pdu_payload_add(
        B, flatbuffers_uint8_vec_create(B, _pdu->payload, _pdu->payload_len)));
    ns(Pdu_swc_id_add(B, swc_id));
    ns(Pdu_ecu_id_add(B, ecu_id));
    if (can_message_metadata) {
        ns(Pdu_transport_Can_add(B, can_message_metadata));
    } else if (ip_message_metadata) {
        ns(Pdu_transport_Ip_add(B, ip_message_metadata));
    }
    ns(Stream_pdus_push_end(B));

    return _pdu->payload_len;
}


static void _decode_can_message_metadata(ns(Pdu_table_t) pdu, NCodecPdu* _pdu)
{
    NCodecPduCanMessageMetadata* can = &_pdu->transport.can_message;
    _pdu->transport_type = NCodecPduTransportTypeCan;
    ns(CanMessageMetadata_table_t) can_msg =
        (ns(CanMessageMetadata_table_t))ns(Pdu_transport(pdu));
    can->frame_format = ns(CanMessageMetadata_message_format(can_msg));
    can->frame_type = ns(CanMessageMetadata_frame_type(can_msg));
    can->interface_id = ns(CanMessageMetadata_interface_id(can_msg));
    can->network_id = ns(CanMessageMetadata_network_id(can_msg));
}

static void _decode_ip_addr_v4(
    ns(IpMessageMetadata_table_t) ip_msg, NCodecPdu* _pdu)
{
    NCodecPduIpAddrV4* addr = &_pdu->transport.ip_message.ip_addr.ip_v4;
    _pdu->transport.ip_message.ip_addr_type = NCodecPduIpAddrIPv4;
    ns(IpV4_table_t) ip_v4 =
        (ns(IpV4_table_t))ns(IpMessageMetadata_ip_addr(ip_msg));
    addr->src_addr = ns(IpV4_src_addr(ip_v4));
    addr->dst_addr = ns(IpV4_dst_addr(ip_v4));
}

static void _decode_ip_addr_v6(
    ns(IpMessageMetadata_table_t) ip_msg, NCodecPdu* _pdu)
{
    NCodecPduIpAddrV6* addr = &_pdu->transport.ip_message.ip_addr.ip_v6;
    _pdu->transport.ip_message.ip_addr_type = NCodecPduIpAddrIPv6;
    ns(IpV6_table_t) ip_v6 =
        (ns(IpV6_table_t))ns(IpMessageMetadata_ip_addr(ip_msg));
    addr->src_addr[0] = ns(IpV6_src_addr(ip_v6))->v0;
    addr->src_addr[1] = ns(IpV6_src_addr(ip_v6))->v1;
    addr->src_addr[2] = ns(IpV6_src_addr(ip_v6))->v2;
    addr->src_addr[3] = ns(IpV6_src_addr(ip_v6))->v3;
    addr->src_addr[4] = ns(IpV6_src_addr(ip_v6))->v4;
    addr->src_addr[5] = ns(IpV6_src_addr(ip_v6))->v5;
    addr->src_addr[6] = ns(IpV6_src_addr(ip_v6))->v6;
    addr->src_addr[7] = ns(IpV6_src_addr(ip_v6))->v7;
    addr->dst_addr[0] = ns(IpV6_dst_addr(ip_v6))->v0;
    addr->dst_addr[1] = ns(IpV6_dst_addr(ip_v6))->v1;
    addr->dst_addr[2] = ns(IpV6_dst_addr(ip_v6))->v2;
    addr->dst_addr[3] = ns(IpV6_dst_addr(ip_v6))->v3;
    addr->dst_addr[4] = ns(IpV6_dst_addr(ip_v6))->v4;
    addr->dst_addr[5] = ns(IpV6_dst_addr(ip_v6))->v5;
    addr->dst_addr[6] = ns(IpV6_dst_addr(ip_v6))->v6;
    addr->dst_addr[7] = ns(IpV6_dst_addr(ip_v6))->v7;
}

static void _decode_do_ip(ns(IpMessageMetadata_table_t) ip_msg, NCodecPdu* _pdu)
{
    NCodecPduDoIpAdapter* a = &_pdu->transport.ip_message.so_ad.do_ip;
    _pdu->transport.ip_message.so_ad_type = NCodecPduSoAdDoIP;
    ns(DoIpMetadata_table_t) do_ip =
        (ns(DoIpMetadata_table_t))ns(IpMessageMetadata_adapter(ip_msg));
    a->protocol_version = ns(DoIpMetadata_protocol_version(do_ip));
    a->payload_type = ns(DoIpMetadata_payload_type(do_ip));
}

static void _decode_some_ip(
    ns(IpMessageMetadata_table_t) ip_msg, NCodecPdu* _pdu)
{
    NCodecPduSomeIpAdapter* a = &_pdu->transport.ip_message.so_ad.some_ip;
    _pdu->transport.ip_message.so_ad_type = NCodecPduSoAdSomeIP;
    ns(SomeIpMetadata_table_t) some_ip =
        (ns(SomeIpMetadata_table_t))ns(IpMessageMetadata_adapter(ip_msg));
    a->message_id = ns(SomeIpMetadata_message_id(some_ip));
    a->length = ns(SomeIpMetadata_length(some_ip));
    a->request_id = ns(SomeIpMetadata_request_id(some_ip));
    a->protocol_version = ns(SomeIpMetadata_protocol_version(some_ip));
    a->interface_version = ns(SomeIpMetadata_interface_version(some_ip));
    a->message_type = ns(SomeIpMetadata_message_type(some_ip));
    a->return_code = ns(SomeIpMetadata_return_code(some_ip));
}

static void _decode_ip_message_metadata(ns(Pdu_table_t) pdu, NCodecPdu* _pdu)
{
    NCodecPduIpMessageMetadata* ip = &_pdu->transport.ip_message;
    _pdu->transport_type = NCodecPduTransportTypeIp;
    ns(IpMessageMetadata_table_t) ip_msg =
        (ns(IpMessageMetadata_table_t))ns(Pdu_transport(pdu));
    ip->eth_dst_mac = ns(IpMessageMetadata_eth_dst_mac(ip_msg));
    ip->eth_src_mac = ns(IpMessageMetadata_eth_src_mac(ip_msg));
    ip->eth_ethertype = ns(IpMessageMetadata_eth_ethertype(ip_msg));
    ip->eth_tci_pcp = ns(IpMessageMetadata_eth_tci_pcp(ip_msg));
    ip->eth_tci_dei = ns(IpMessageMetadata_eth_tci_dei(ip_msg));
    ip->eth_tci_vid = ns(IpMessageMetadata_eth_tci_vid(ip_msg));

    ns(IpAddr_union_type_t) ip_addr_type =
        ns(IpMessageMetadata_ip_addr_type(ip_msg));
    if (ip_addr_type == ns(IpAddr_v4)) {
        _decode_ip_addr_v4(ip_msg, _pdu);


    } else if (ip_addr_type == ns(IpAddr_v6)) {
        _decode_ip_addr_v6(ip_msg, _pdu);
    }
    ip->ip_protocol = ns(IpMessageMetadata_ip_protocol(ip_msg));
    ip->ip_src_port = ns(IpMessageMetadata_ip_src_port(ip_msg));
    ip->ip_dst_port = ns(IpMessageMetadata_ip_dst_port(ip_msg));

    ns(SocketAdapter_union_type_t) adapter_type =
        ns(IpMessageMetadata_adapter_type(ip_msg));
    if (adapter_type == ns(SocketAdapter_do_ip)) {
        _decode_do_ip(ip_msg, _pdu);


    } else if (adapter_type == ns(SocketAdapter_some_ip)) {
        _decode_some_ip(ip_msg, _pdu);
    }
}


static void get_stream_from_buffer(NCODEC* nc)
{
    ABCodecInstance* _nc = (ABCodecInstance*)nc;

    /* Reset the message (and frame) parsing state. */
    _nc->msg_ptr = NULL;
    _nc->msg_len = 0;
    _nc->vector = NULL;
    _nc->vector_idx = 0;
    _nc->vector_len = 0;

    /* Next message? */
    uint8_t* buffer;
    size_t   length;
    _nc->c.stream->read(nc, &buffer, &length, NCODEC_POS_NC);

    uint8_t*       msg_ptr = buffer;
    uint8_t* const buffer_ptr = buffer;
    while ((size_t)(msg_ptr - buffer_ptr) < length) {
        /* Messages start with a size prefix. */
        size_t msg_len = 0;
        msg_ptr = flatbuffers_read_size_prefix(msg_ptr, &msg_len);
        if (msg_len == 0) break;
        /* Advance the stream pos (+4 for size prefix). */
        _nc->c.stream->seek(nc, msg_len + 4, NCODEC_SEEK_CUR);
        /* Set the parsing state. */
        if (flatbuffers_has_identifier(msg_ptr, flatbuffers_identifier)) {
            _nc->msg_ptr = msg_ptr;
            _nc->msg_len = msg_len;
            return;
        }
        /* Next message in the stream. */
        msg_ptr += msg_len;
    }

    /* No message in stream. */
    _nc->c.stream->seek(nc, 0, NCODEC_SEEK_END);
}

static void get_vector_from_stream(NCODEC* nc)
{
    ABCodecInstance* _nc = (ABCodecInstance*)nc;

    /* Reset the PDU parsing state. */
    _nc->vector = NULL;
    _nc->vector_idx = 0;
    _nc->vector_len = 0;

    /* Guard conditions. */
    if (_nc->msg_ptr == NULL) return;

    /* Decode the vector of PDUs. */
    ns(Stream_table_t) stream = ns(Stream_as_root(_nc->msg_ptr));
    _nc->vector = ns(Stream_pdus(stream));
    _nc->vector_len = ns(Pdu_vec_len(_nc->vector));
}

int32_t pdu_read(NCODEC* nc, NCodecPdu* pdu)
{
    ABCodecInstance* _nc = (ABCodecInstance*)nc;
    NCodecPdu*       _pdu = (NCodecPdu*)pdu;
    if (_nc == NULL) return -ENOSTR;
    if (_pdu == NULL) return -EINVAL;
    if (_nc->c.stream == NULL) return -ENOSR;

    /* Reset the message, in case caller ignores the return value. */
    _pdu->payload_len = 0;
    _pdu->payload = NULL;

    /* Process the stream/frames. */
    if (_nc->msg_ptr == NULL) get_stream_from_buffer(nc);
    if (_nc->vector == NULL) get_vector_from_stream(nc);
    while (_nc->msg_ptr && _nc->vector) {
        for (uint32_t _vi = _nc->vector_idx; _vi < _nc->vector_len; _vi++) {
            ns(Pdu_table_t) pdu = ns(Pdu_vec_at(_nc->vector, _vi));

            /* Filter: sender==receiver. */
            if ((_nc->swc_id) && (_nc->swc_id == ns(Pdu_swc_id(pdu)))) continue;

            /* Return the message. */
            _pdu->id = ns(Pdu_id(pdu));
            flatbuffers_uint8_vec_t payload = ns(Pdu_payload(pdu));
            _pdu->payload = (uint8_t*)payload;
            _pdu->payload_len = flatbuffers_uint8_vec_len(payload);
            _pdu->swc_id = ns(Pdu_swc_id(pdu));
            _pdu->ecu_id = ns(Pdu_ecu_id(pdu));

            if (ns(Pdu_transport_is_present(pdu))) {
                ns(TransportMetadata_union_type_t) transport_type =
                    ns(Pdu_transport_type(pdu));
                if (transport_type == ns(TransportMetadata_Can)) {
                    _decode_can_message_metadata(pdu, _pdu);
                } else if (transport_type == ns(TransportMetadata_Ip)) {
                    _decode_ip_message_metadata(pdu, _pdu);
                }
            }

            /* ... but don't forget to save the vector index either. */
            _nc->vector_idx = _vi + 1;
            return _pdu->payload_len;
        }

        /* Next msg/vector? */
        get_stream_from_buffer(nc);
        if (_nc->msg_ptr) get_vector_from_stream(nc);
    }
    /* No messages in stream. */
    _nc->c.stream->seek(nc, 0, NCODEC_SEEK_END);
    return -ENOMSG;
}


int32_t pdu_flush(NCODEC* nc)
{
    ABCodecInstance* _nc = (ABCodecInstance*)nc;
    if (_nc == NULL) return -ENOSTR;
    if (_nc->c.stream == NULL) return -ENOSR;

    uint8_t* buffer = NULL;
    size_t   length = 0;

    finalize_stream(_nc, &buffer, &length);
    if (buffer) {
        _nc->c.stream->write(nc, buffer, length);
        free(buffer);
    }
    return length;
}


int32_t pdu_truncate(NCODEC* nc)
{
    ABCodecInstance* _nc = (ABCodecInstance*)nc;
    if (_nc == NULL) return -ENOSTR;
    if (_nc->c.stream == NULL) return -ENOSR;

    reset_stream(_nc);
    _nc->c.stream->seek(nc, 0, NCODEC_SEEK_RESET);

    return 0;
}
