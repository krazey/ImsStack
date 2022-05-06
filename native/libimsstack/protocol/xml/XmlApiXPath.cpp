#include "XmlApiXPath.h"

xmlXPathContextPtr XmlApi_XPathNewContext(xmlDocPtr doc)
{
    return xmlXPathNewContext(doc);
}

void XmlApi_XPathFreeContext(xmlXPathContextPtr ctxt)
{
    xmlXPathFreeContext(ctxt);
}

xmlXPathObjectPtr XmlApi_XPathEvalExpression(const xmlChar* str, xmlXPathContextPtr ctxt)
{
    return xmlXPathEvalExpression(str, ctxt);
}

void XmlApi_XPathFreeObject(xmlXPathObjectPtr obj)
{
    xmlXPathFreeObject(obj);
}
