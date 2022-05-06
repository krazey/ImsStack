#ifndef XML_API_PARSER_H_
#define XML_API_PARSER_H_

#include "libxml/xpath.h"

extern void XmlApi_CleanupParser();

extern xmlDocPtr XmlApi_ReadDoc(
        const xmlChar* cur, const char* url, const char* encoding, int options);

extern xmlDocPtr XmlApi_ReadMemory(
        const char* buffer, int size, const char* url, const char* encoding, int options);

#endif
