#include "XmlApiParser.h"

void XmlApi_CleanupParser()
{
    xmlCleanupParser();
}

xmlDocPtr XmlApi_ReadDoc(const xmlChar* cur, const char* url, const char* encoding, int options)
{
    return xmlReadDoc(cur, url, encoding, options);
}

xmlDocPtr XmlApi_ReadMemory(
        const char* buffer, int size, const char* url, const char* encoding, int options)
{
    return xmlReadMemory(buffer, size, url, encoding, options);
}
