#ifndef XML_API_TREE_H_
#define XML_API_TREE_H_

#include "libxml/tree.h"

extern xmlBufferPtr XmlApi_BufferCreate(void);
extern void XmlApi_BufferFree(xmlBufferPtr buf);
extern xmlChar* XmlApi_GetProp(xmlNodePtr node, const xmlChar* name);
extern xmlChar* XmlApi_NodeListGetString(xmlDocPtr doc, xmlNodePtr list, int inLine);
extern xmlNodePtr XmlApi_DocGetRootElement(xmlDocPtr doc);
extern void XmlApi_FreeDoc(xmlDocPtr cur);
extern xmlChar* XmlApi_NodeGetContent(xmlNodePtr cur);
extern xmlChar* XmlApi_GetNodePath(xmlNodePtr node);
extern xmlDocPtr XmlApi_CopyDoc(xmlDocPtr doc, int recursive);
extern xmlNodePtr XmlApi_DocSetRootElement(xmlDocPtr doc, xmlNodePtr root);
extern void XmlApi_DocDumpMemory(xmlDocPtr cur, xmlChar** mem, int* size);
extern void XmlApi_UnlinkNode(xmlNodePtr cur);
extern xmlNodePtr XmlApi_NewChild(
        xmlNodePtr parent, xmlNsPtr ns, const xmlChar* name, const xmlChar* content);
extern xmlDocPtr XmlApi_NewDoc(const xmlChar* version);
extern xmlNodePtr XmlApi_NewNode(xmlNsPtr ns, const xmlChar* name);
extern xmlAttrPtr XmlApi_NewProp(xmlNodePtr node, const xmlChar* name, const xmlChar* value);

extern void XmlApi_NodeSetContent(xmlNodePtr cur, const xmlChar* content);
extern int XmlApi_UnsetProp(xmlNodePtr node, const xmlChar* name);
extern xmlNodePtr XmlApi_AddSibling(xmlNodePtr cur, xmlNodePtr elem);
extern void XmlApi_NodeSetName(xmlNodePtr cur, const xmlChar* name);
extern xmlNsPtr* XmlApi_GetNsList(xmlDocPtr doc, xmlNodePtr node);

#endif
