/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ussi/UssiConstants.h"

PUBLIC GLOBAL const IMS_CHAR UssiConstants::ELEMENT_USSD_DATA[] = "ussd-data";
PUBLIC GLOBAL const IMS_CHAR UssiConstants::ELEMENT_LANGUAGE[] = "language";
PUBLIC GLOBAL const IMS_CHAR UssiConstants::ELEMENT_USSD_STRING[] = "ussd-string";
PUBLIC GLOBAL const IMS_CHAR UssiConstants::ELEMENT_ERROR_CODE[] = "error-code";
PUBLIC GLOBAL const IMS_CHAR UssiConstants::ELEMENT_ANYEXT[] = "anyExt";
PUBLIC GLOBAL const IMS_CHAR UssiConstants::ELEMENT_USS_REQUEST[] = "UnstructuredSS-Request";
PUBLIC GLOBAL const IMS_CHAR UssiConstants::ELEMENT_USS_NOTIFY[] = "UnstructuredSS-Notify";
PUBLIC GLOBAL const IMS_CHAR UssiConstants::ELEMENT_ALERTING_PATTERN[] = "alertingPattern";
PUBLIC GLOBAL const IMS_CHAR UssiConstants::ELEMENT_LANGUAGE_EN[] = "en";

PUBLIC GLOBAL const IMS_CHAR UssiConstants::HEADER_USSD_PACKAGE[] = "g.3gpp.ussd";
PUBLIC GLOBAL const IMS_CHAR UssiConstants::HEADER_APPLICATION_SDP[] = "application/sdp";
PUBLIC GLOBAL const IMS_CHAR UssiConstants::HEADER_APPLICATION_IMSXML[] =
        "application/3gpp-ims+xml";
PUBLIC GLOBAL const IMS_CHAR UssiConstants::HEADER_APPLICATION_USSD[] = "application/vnd.3gpp.ussd";
PUBLIC GLOBAL const IMS_CHAR UssiConstants::HEADER_APPLICATION_USSDXML[] =
        "application/vnd.3gpp.ussd+xml";
PUBLIC GLOBAL const IMS_CHAR UssiConstants::HEADER_MULTIPART_MIXED[] = "multipart/mixed";
PUBLIC GLOBAL const IMS_CHAR UssiConstants::HEADER_INFO_PACKAGE[] = "Info-Package";
PUBLIC GLOBAL const IMS_CHAR UssiConstants::HEADER_RENDER_HANDLING[] = "render;handling=optional";

PUBLIC GLOBAL const IMS_CHAR UssiConstants::XML_PROCESSING_INSTRUCTION[] =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
PUBLIC GLOBAL const IMS_SINT32 UssiConstants::XML_BUFFER_SIZE = 512;
