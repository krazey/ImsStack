/*
 * author : aromi.kwak
 * version : 1.0
 * date : 2016.10
 * brief : Create USSDConstants
 */

#include "ussi/UssiConstants.h"

PUBLIC GLOBAL const IMS_CHAR USSDConstants::ELEMENT_USSD_DATA[] = "ussd-data";
PUBLIC GLOBAL const IMS_CHAR USSDConstants::ELEMENT_LANGUAGE[] = "language";
PUBLIC GLOBAL const IMS_CHAR USSDConstants::ELEMENT_USSD_STRING[] = "ussd-string";
PUBLIC GLOBAL const IMS_CHAR USSDConstants::ELEMENT_ERROR_CODE[] = "error-code";
PUBLIC GLOBAL const IMS_CHAR USSDConstants::ELEMENT_ANYEXT[] = "anyExt";
PUBLIC GLOBAL const IMS_CHAR USSDConstants::ELEMENT_USS_REQUEST[] = "UnstructuredSS-Request";
PUBLIC GLOBAL const IMS_CHAR USSDConstants::ELEMENT_USS_NOTIFY[] = "UnstructuredSS-Notify";
PUBLIC GLOBAL const IMS_CHAR USSDConstants::ELEMENT_ALERTING_PATTERN[] = "alertingPattern";
PUBLIC GLOBAL const IMS_CHAR USSDConstants::ELEMENT_LANGUAGE_EN[] = "en";

PUBLIC GLOBAL const IMS_CHAR USSDConstants::HEADER_USSD_PACKAGE[] = "g.3gpp.ussd";
PUBLIC GLOBAL const IMS_CHAR USSDConstants::HEADER_RECVINFO[] = "Recv-Info";
PUBLIC GLOBAL const IMS_CHAR USSDConstants::HEADER_APPLICATION_SDP[] = "application/sdp";
PUBLIC GLOBAL const IMS_CHAR USSDConstants::HEADER_APPLICATION_IMSXML[] =
        "application/3gpp-ims+xml";
PUBLIC GLOBAL const IMS_CHAR USSDConstants::HEADER_APPLICATION_USSD[] = "application/vnd.3gpp.ussd";
PUBLIC GLOBAL const IMS_CHAR USSDConstants::HEADER_APPLICATION_USSDXML[] =
        "application/vnd.3gpp.ussd+xml";
PUBLIC GLOBAL const IMS_CHAR USSDConstants::HEADER_MULTIPART_MIXED[] = "multipart/mixed";
PUBLIC GLOBAL const IMS_CHAR USSDConstants::HEADER_INFO_PACKAGE[] = "Info-Package";
PUBLIC GLOBAL const IMS_CHAR USSDConstants::HEADER_RENDER_HANDLING[] = "render;handling=optional";

PUBLIC GLOBAL const IMS_CHAR USSDConstants::XML_PROCESSING_INSTRUCTION[] =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
PUBLIC GLOBAL const IMS_SINT32 USSDConstants::XML_BUFFER_SIZE = 512;
