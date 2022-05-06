#include "XmlApi.h"
#include "libxml/xmlversion.h"

PUBLIC GLOBAL const IMS_CHAR* XmlApi::GetVersion()
{
    return LIBXML_VERSION_STRING;
}
