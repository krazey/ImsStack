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

#ifndef MTS_DEF_H_
#define MTS_DEF_H_

#include "ByteArray.h"
#include "ImsTypeDef.h"

enum class MtsTimerType
{
    TIMER_UNKNOWN = 0,
    TIMER_SMS_CALLBACK_MODE = 1,
    TIMER_RETRY_AFTER = 2,
};

enum class SmsFormatType
{
    SMSFORMAT_3GPP = 1,
    SMSFORMAT_3GPP2,
    SMSFORMAT_INVALID
};

class EmergencySmsSendRequestInfo
{
public:
    SmsFormatType eSmsFormat;
    AString strAddress;
    ByteArray objSmsData;
    IMS_SINT32 nSeqId;
};

enum
{
    EXPIRED_TIME_SCBM = 300000  // 5min, 300s, 300000ms
};

#endif
