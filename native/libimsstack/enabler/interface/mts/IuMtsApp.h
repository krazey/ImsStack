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

#ifndef IU_MTS_APP_H_
#define IU_MTS_APP_H_

#include "ImsMessageDef.h"
#include "ImsTypeDef.h"

#define JAVA2MTSENABLER (IMS_MSG_BASE_SERVICE + 0)
#define MTSENABLER2JAVA (IMS_MSG_BASE_SERVICE + 50)

class IuMtsApp
{
public:
    // Event : Java to IMS
    static const IMS_SINT32 NOTI_MTSENABLER_SEND_MO_SMS = JAVA2MTSENABLER + 1;
    static const IMS_SINT32 NOTI_MTSENABLER_MO_SMS_TIMED_OUT = JAVA2MTSENABLER + 2;

    // Event : IMS to Java
    static const IMS_UINT32 REPORT_MTS_MO_STATUS = MTSENABLER2JAVA + 1;
    static const IMS_UINT32 REPORT_MTS_MT_SMS = MTSENABLER2JAVA + 2;
};

enum
{
    MO_INVALID = 0,
    MO_SUCCESS = 1,
    MO_ERROR_GENERIC = 2,
    MO_ERROR_RETRY = 3,
    MO_ERROR_FALLBACK = 4,
    MO_ERROR_BY_RETRY_AFTER = 5,  // Internal usage
};

enum
{
    SMSFORMAT_INVALID = 0,
    SMSFORMAT_3GPP = 1,
    SMSFORMAT_3GPP2 = 2,
};

#endif
