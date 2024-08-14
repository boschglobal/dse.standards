// Copyright 2024 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <libxml/xpath.h>
#include <dse/clib/collections/hashmap.h>


extern char*    ascii85_encode(const uint8_t* data, size_t len);
extern uint8_t* ascii85_decode(const char* source, size_t* len);


/**
parse_tool_anno
===============

Parse a specific tool annotation by `Tool name` and `name`.

Parameters
----------
node (xmlNode*)
: Node (ScalarVariable) from where to search for the tool annotation.

tool (const char*)
: The name of the tool annotations container.

name (const char*)
: The name of the annotation to return.

Returns
-------
xmlChar*
: Annotation text (consolidated `XmlElement` text).

NULL
: The annotation was not found.
*/
static xmlChar* parse_tool_anno(
    xmlNode* node, const char* tool, const char* name)
{
    xmlChar* result = NULL;

    /* Build the XPath query. */
    size_t query_len =
        snprintf(NULL, 0, "Annotations/Tool[@name='%s']/Annotation", tool) + 1;
    char* query = calloc(query_len, sizeof(char));
    snprintf(query, query_len, "Annotations/Tool[@name='%s']/Annotation", tool);
    xmlXPathContext* ctx = xmlXPathNewContext(node->doc);
    ctx->node = node;
    xmlXPathObject* obj = xmlXPathEvalExpression((xmlChar*)query, ctx);

    /* Locate the annotation value (if present). */
    if (obj->type == XPATH_NODESET && obj->nodesetval) {
        /* Annotation */
        for (int i = 0; i < obj->nodesetval->nodeNr; i++) {
            xmlNode* a_node = obj->nodesetval->nodeTab[i];
            xmlChar* a_name = xmlGetProp(a_node, (xmlChar*)"name");
            xmlChar* a_value =
                xmlNodeListGetString(a_node->doc, a_node->xmlChildrenNode, 1);
            if (strcmp((char*)a_name, name) == 0) {
                result = a_value;
                xmlFree(a_name);
                break;
            } else {
                xmlFree(a_name);
                xmlFree(a_value);
            }
        }
    }

    /* Cleanup and return the result. */
    free(query);
    xmlXPathFreeObject(obj);
    xmlXPathFreeContext(ctx);
    return result;
}


/**
parse_bus_topology
==================

Parse the Bus Topology from the FMU `modelDescription.xml` file and persist
the topology in the provided `HashMap` objects.

Parameters
----------
model_description_path (const char*)
: Path of `modelDescription.xml` file to be parsed.

bus_id (const char*)
: The Bus Identifier for the bus being parsed.

bus_object (void*)
: This object will be indexed in the `rx`/`tx` maps.

rx (HashMap*)
: Map {`vr`:`bus_object`}.

tx (HashMap*)
: Map {`vr`:`bus_object`}.
*/
void parse_bus_topology(const char* model_description_path, const char* bus_id,
    void* bus_object, HashMap* rx, HashMap* tx)
{
    xmlInitParser();
    xmlDoc* doc = xmlParseFile(model_description_path);

    /* Search all Scalar Variables for bus annotations. */
    xmlXPathContext* ctx = xmlXPathNewContext(doc);
    xmlXPathObject*  obj = xmlXPathEvalExpression(
         (xmlChar*)"/fmiModelDescription/ModelVariables/ScalarVariable", ctx);
    for (int i = 0; i < obj->nodesetval->nodeNr; i++) {
        /* ScalarVariable */
        xmlChar* a_bus_id = NULL;
        xmlNode* node = obj->nodesetval->nodeTab[i];
        xmlChar* name = xmlGetProp(node, (xmlChar*)"name");
        xmlChar* vr = xmlGetProp(node, (xmlChar*)"valueReference");
        xmlChar* causality = xmlGetProp(node, (xmlChar*)"causality");
        if (name == NULL || vr == NULL || causality == NULL) goto next;

        /* Look for the bus_id annotation. */
        a_bus_id = parse_tool_anno(
            node, "dse.standards.fmi-ls-bus-topology", "bus_id");
        if (a_bus_id == NULL) goto next;
        if (strcmp(bus_id, (char*)a_bus_id) == 0) {
            if (strcmp((char*)causality, "input") == 0) {
                hashmap_set(rx, (char*)vr, bus_object);
            } else if (strcmp((char*)causality, "output") == 0) {
                hashmap_set(tx, (char*)vr, bus_object);
            }
        }
    next:
        /* Cleanup. */
        xmlFree(a_bus_id);
        xmlFree(name);
        xmlFree(vr);
        xmlFree(causality);
    }

    /* Cleanup. */
    xmlXPathFreeObject(obj);
    xmlXPathFreeContext(ctx);
    xmlFreeDoc(doc);
}


/**
parse_binary_to_text
====================

Parse all Binary-to-Text configurations from the FMU `modelDescription.xml`
file and persist the configuration to the provided `HashMap` objects.

Parameters
----------
model_description_path (const char*)
: Path of `modelDescription.xml` file to be parsed.

encode_func (HashMap*)
: Map {`vr`:`EncodeFunc`}.

decode_func (HashMap*)
: Map {`vr`:`DecodeFunc`}.
*/
void parse_binary_to_text(const char* model_description_path,
    HashMap* encode_func, HashMap* decode_func)
{
    xmlInitParser();
    xmlDoc* doc = xmlParseFile(model_description_path);

    /* Search all Scalar Variables for bus annotations. */
    xmlXPathContext* ctx = xmlXPathNewContext(doc);
    xmlXPathObject*  obj = xmlXPathEvalExpression(
         (xmlChar*)"/fmiModelDescription/ModelVariables/ScalarVariable", ctx);
    for (int i = 0; i < obj->nodesetval->nodeNr; i++) {
        /* ScalarVariable */
        xmlChar* encoding = NULL;
        xmlNode* node = obj->nodesetval->nodeTab[i];
        xmlChar* name = xmlGetProp(node, (xmlChar*)"name");
        xmlChar* vr = xmlGetProp(node, (xmlChar*)"valueReference");
        xmlChar* causality = xmlGetProp(node, (xmlChar*)"causality");
        if (name == NULL || vr == NULL || causality == NULL) goto next;

        /* Look for the encoding annotation. */
        encoding = parse_tool_anno(
            node, "dse.standards.fmi-ls-binary-to-text", "encoding");
        if (encoding == NULL) goto next;
        if (strcmp((char*)encoding, "ascii85") == 0) {
            if (strcmp((char*)causality, "input") == 0) {
                hashmap_set(decode_func, (char*)vr, ascii85_decode);
            } else if (strcmp((char*)causality, "output") == 0) {
                hashmap_set(encode_func, (char*)vr, ascii85_encode);
            }
        }
    next:
        /* Cleanup. */
        xmlFree(encoding);
        xmlFree(name);
        xmlFree(vr);
        xmlFree(causality);
    }

    /* Cleanup. */
    xmlXPathFreeObject(obj);
    xmlXPathFreeContext(ctx);
    xmlFreeDoc(doc);
}
