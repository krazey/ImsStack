#ifndef XML_API_XPATH_H_
#define XML_API_XPATH_H_

#include "libxml/xpath.h"

extern xmlXPathContextPtr XmlApi_XPathNewContext(xmlDocPtr doc);
extern void XmlApi_XPathFreeContext(xmlXPathContextPtr ctxt);
extern xmlXPathObjectPtr XmlApi_XPathEvalExpression(const xmlChar* str, xmlXPathContextPtr ctxt);
extern void XmlApi_XPathFreeObject(xmlXPathObjectPtr obj);

#endif
