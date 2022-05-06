#include "XmlApiTree.h"

xmlBufferPtr XmlApi_BufferCreate()
{
    return xmlBufferCreate();
}

void XmlApi_BufferFree(xmlBufferPtr buf)
{
    xmlBufferFree(buf);
}

xmlChar* XmlApi_GetProp(xmlNodePtr node, const xmlChar* name)
{
    return xmlGetProp(node, name);
}

xmlChar* XmlApi_NodeListGetString(xmlDocPtr doc, xmlNodePtr list, int inLine)
{
    return xmlNodeListGetString(doc, list, inLine);
}

xmlNodePtr XmlApi_DocGetRootElement(xmlDocPtr doc)
{
    return xmlDocGetRootElement(doc);
}

void XmlApi_FreeDoc(xmlDocPtr cur)
{
    xmlFreeDoc(cur);
}

xmlChar* XmlApi_NodeGetContent(xmlNodePtr cur)
{
    return xmlNodeGetContent(cur);
}

xmlChar* XmlApi_GetNodePath(xmlNodePtr node)
{
    return xmlGetNodePath(node);
}

xmlDocPtr XmlApi_CopyDoc(xmlDocPtr doc, int recursive)
{
    return xmlCopyDoc(doc, recursive);
}

xmlNodePtr XmlApi_DocSetRootElement(xmlDocPtr doc, xmlNodePtr root)
{
    return xmlDocSetRootElement(doc, root);
}

void XmlApi_DocDumpMemory(xmlDocPtr cur, xmlChar** mem, int* size)
{
    xmlDocDumpMemory(cur, mem, size);
}

void XmlApi_UnlinkNode(xmlNodePtr cur)
{
    xmlUnlinkNode(cur);
}

xmlNodePtr XmlApi_NewChild(
        xmlNodePtr parent, xmlNsPtr ns, const xmlChar* name, const xmlChar* content)
{
    return xmlNewChild(parent, ns, name, content);
}

xmlDocPtr XmlApi_NewDoc(const xmlChar* version)
{
    return xmlNewDoc(version);
}

xmlNodePtr XmlApi_NewNode(xmlNsPtr ns, const xmlChar* name)
{
    return xmlNewNode(ns, name);
}

xmlAttrPtr XmlApi_NewProp(xmlNodePtr node, const xmlChar* name, const xmlChar* value)
{
    return xmlNewProp(node, name, value);
}

void XmlApi_NodeSetContent(xmlNodePtr cur, const xmlChar* content)
{
    xmlNodeSetContent(cur, content);
}

int XmlApi_UnsetProp(xmlNodePtr node, const xmlChar* name)
{
    return xmlUnsetProp(node, name);
}

xmlNodePtr XmlApi_AddSibling(xmlNodePtr cur, xmlNodePtr elem)
{
    return xmlAddSibling(cur, elem);
}

void XmlApi_NodeSetName(xmlNodePtr cur, const xmlChar* name)
{
    xmlNodeSetName(cur, name);
}

xmlNsPtr* XmlApi_GetNsList(xmlDocPtr doc, xmlNodePtr node)
{
    return xmlGetNsList(doc, node);
}
