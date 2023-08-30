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

#ifndef USSI_CONSTANTS_
#define USSI_CONSTANTS_

#include "ImsTypeDef.h"

class UssiConstants
{
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

#endif
