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

#define IMS_STL_USE

#include "JniMtsServiceThread.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "IuMtsService.h"
#include <utils/String8.h>

using namespace android;

__IMS_TRACE_TAG_USER_DECL__("JNI.MTS");

PUBLIC
JniMtsServiceThread::JniMtsServiceThread() :
        BaseServiceThread()
{
    IMS_TRACE_I("JniMtsServiceThread : ", 0, 0, 0);
}

PUBLIC VIRTUAL JniMtsServiceThread::~JniMtsServiceThread()
{
    IMS_TRACE_I("~JniMtsServiceThread : ", 0, 0, 0);
}

PUBLIC
void JniMtsServiceThread::ReportMoStatus(IN IMS_SINT32 nReason, IN SmsFormatType eSmsFormat,
        IN IMS_UINT8 nRetryAfter, IN IMS_SINT32 nSeqId, IN IMS_SINT32 nSlotId)
{
    IMS_TRACE_D("ReportMoStatus", 0, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IuMtsService::REPORT_MTS_MO_STATUS);
    objParcel.writeInt32(nReason);
    objParcel.writeInt32(ConvertSmsFormatToInt(eSmsFormat));
    objParcel.writeInt32(nRetryAfter);
    objParcel.writeInt32(nSeqId);
    objParcel.writeInt32(nSlotId);

    SendData2Java(objParcel);
}

PUBLIC
void JniMtsServiceThread::ReportMtSms(
        IN SmsFormatType eSmsFormat, IN const ByteArray& objData, IN IMS_SINT32 nSlotId)
{
    IMS_TRACE_D("ReportMtSms", 0, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IuMtsService::REPORT_MTS_MT_SMS);
    objParcel.writeInt32(ConvertSmsFormatToInt(eSmsFormat));
    objParcel.writeString16(android::String16(objData.ToString().GetStr()));
    objParcel.writeInt32(nSlotId);

    SendData2Java(objParcel);
}

PRIVATE IMS_UINT32 JniMtsServiceThread::ConvertSmsFormatToInt(IN SmsFormatType eSmsFormat)
{
    switch (eSmsFormat)
    {
        case SmsFormatType::SMSFORMAT_3GPP:
            return SMSFORMAT_3GPP;

        case SmsFormatType::SMSFORMAT_3GPP2:
            return SMSFORMAT_3GPP2;

        default:
            return SMSFORMAT_INVALID;
    }
}
