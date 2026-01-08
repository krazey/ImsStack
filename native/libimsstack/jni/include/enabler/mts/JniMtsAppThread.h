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

#ifndef JNI_MTS_APP_THREAD_H_
#define JNI_MTS_APP_THREAD_H_

#include "BaseServiceThread.h"
#include "IJniMtsAppThread.h"
#include "MtsDef.h"

class JniMtsAppThread : public BaseServiceThread, public IJniMtsAppThread
{
public:
    JniMtsAppThread();
    virtual ~JniMtsAppThread() override;

    void ReportMoStatus(IN IMS_SINT32 nReason, IN SmsFormatType eSmsFormat, IN IMS_SINT32 nSeqId,
            IN IMS_SINT32 nSlotId) override;
    void ReportMtSms(IN SmsFormatType eSmsFormat, IN const ByteArray& objContent,
            IN IMS_SINT32 nSlotId) override;

private:
    static IMS_UINT32 ConvertSmsFormatToInt(IN SmsFormatType eSmsFormat);
};

#endif
