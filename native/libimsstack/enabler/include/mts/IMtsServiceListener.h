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

#ifndef INTERFACE_MTS_SERVICE_LISTENER_H_
#define INTERFACE_MTS_SERVICE_LISTENER_H_

#include "IPageMessage.h"
#include "MtsDef.h"

class IMtsServiceListener
{
public:
    virtual void NotifyMoSms(IN SmsFormatType eSmsFormat, IN const ByteArray& objData,
            IN const AString& strAddress, IN IMS_SINT32 nSeqId, IN IMS_BOOL bEmergency) = 0;
    virtual void NotifyMtSms(IN IPageMessage* piMessage) = 0;
};

#endif
