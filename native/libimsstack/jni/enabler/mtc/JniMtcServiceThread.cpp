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

#include "IuMtcService.h"
#include "JniMtcCall.h"
#include "JniMtcServiceThread.h"
#include "JniMtcUtils.h"
#include "ServiceTrace.h"
#include "call/ParticipantInfo.h"
#include <binder/Parcel.h>

using namespace android;

__IMS_TRACE_TAG_USER_DECL__("JNI.MTC");

PUBLIC
JniMtcServiceThread::JniMtcServiceThread() :
        BaseServiceThread()
{
    IMS_TRACE_D("+JniMtcServiceThread", 0, 0, 0);
}

PUBLIC VIRTUAL JniMtcServiceThread::~JniMtcServiceThread()
{
    IMS_TRACE_D("~JniMtcServiceThread", 0, 0, 0);
}

PUBLIC
void JniMtcServiceThread::OnServiceChanged(IN IMS_SINT32 eStatus, IN IMS_SINT32 eReason)
{
    IMS_TRACE_D("OnServiceChanged [%d]", eStatus, 0, 0);
    Parcel objParcel;
    objParcel.writeInt32(IuMtcService::SERVICE_CHANGED);
    objParcel.writeInt32(eStatus);
    objParcel.writeInt32(eReason);

    SendData2Java(objParcel);
}

PUBLIC
void JniMtcServiceThread::OnEmergencyServiceChanged(IN IMS_SINT32 eStatus, IN IMS_SINT32 eReason,
        IN IMS_SINT32 eServiceType)
{
    IMS_TRACE_D("OnEmergencyServiceChanged [%d]", eStatus, 0, 0);
    Parcel objParcel;
    objParcel.writeInt32(IuMtcService::E_SERVICE_CHANGED);
    objParcel.writeInt32(eStatus);
    objParcel.writeInt32(eReason);
    objParcel.writeInt32(eServiceType);

    SendData2Java(objParcel);
}

PUBLIC
void JniMtcServiceThread::OnPreIncomingCallReceived(IN IMS_ULONG nCallKey)
{
    IMS_TRACE_D("OnPreIncomingCallReceived", 0, 0, 0);
    Parcel objParcel;
    objParcel.writeInt32(IuMtcService::PRE_INCOMING_CALL);
    objParcel.writeInt64(nCallKey);

    objParcel.writeString16(android::String16(AString("MTCLOG").GetStr())); // TODO: Log.

    SendData2Java(objParcel);
}
