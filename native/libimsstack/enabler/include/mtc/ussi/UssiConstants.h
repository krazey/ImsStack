#ifndef USSI_CONSTANTS_
#define USSI_CONSTANTS_

#include "IMSTypeDef.h"

class USSDConstants
{
private:
    USSDConstants();

public:
    static const IMS_CHAR ELEMENT_USSD_DATA[];
    static const IMS_CHAR ELEMENT_LANGUAGE[];
    static const IMS_CHAR ELEMENT_USSD_STRING[];
    static const IMS_CHAR ELEMENT_ERROR_CODE[];
    static const IMS_CHAR ELEMENT_ANYEXT[];
    static const IMS_CHAR ELEMENT_USS_REQUEST[];
    static const IMS_CHAR ELEMENT_USS_NOTIFY[];
    static const IMS_CHAR ELEMENT_ALERTING_PATTERN[];
    static const IMS_CHAR ELEMENT_LANGUAGE_EN[];

    static const IMS_CHAR HEADER_USSD_PACKAGE[];
    static const IMS_CHAR HEADER_RECVINFO[];
    static const IMS_CHAR HEADER_APPLICATION_SDP[];
    static const IMS_CHAR HEADER_APPLICATION_IMSXML[];
    static const IMS_CHAR HEADER_APPLICATION_USSD[];
    static const IMS_CHAR HEADER_APPLICATION_USSDXML[];
    static const IMS_CHAR HEADER_MULTIPART_MIXED[];
    static const IMS_CHAR HEADER_INFO_PACKAGE[];
    static const IMS_CHAR HEADER_RENDER_HANDLING[];

    static const IMS_CHAR XML_PROCESSING_INSTRUCTION[];
    static const IMS_SINT32 XML_BUFFER_SIZE;
};

#endif  // USSI_CONSTANTS_