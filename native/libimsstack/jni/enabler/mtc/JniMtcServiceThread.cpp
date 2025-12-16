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

#include "ImsList.h"
#include "IuMtcService.h"
#include "JniMtcCall.h"
#include "JniMtcServiceThread.h"
#include "JniMtcUtils.h"
#include "MtcDef.h"
#include "ServiceTrace.h"
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
void JniMtcServiceThread::OnServiceChanged(
        IN IuMtcService::ServiceState eState, IN IMS_SINT32 eReason)
{
    IMS_TRACE_D("OnServiceChanged [%d]", eState, 0, 0);
    Parcel objParcel;
    objParcel.writeInt32(IuMtcService::SERVICE_CHANGED);
    objParcel.writeInt32(static_cast<IMS_SINT32>(eState));
    objParcel.writeInt32(eReason);

    SendData2Java(objParcel);
}

PUBLIC
void JniMtcServiceThread::OnEmergencyServiceChanged(IN IuMtcService::EmergencyServiceState eState,
        IN IuMtcService::EmergencyServiceUnavailableReason eReason, IN ServiceType eServiceType)
{
    IMS_TRACE_D("OnEmergencyServiceChanged [%d]", eState, 0, 0);
    Parcel objParcel;
    objParcel.writeInt32(IuMtcService::E_SERVICE_CHANGED);
    objParcel.writeInt32(static_cast<IMS_SINT32>(eState));
    objParcel.writeInt32(static_cast<IMS_SINT32>(eReason));
    objParcel.writeInt32(static_cast<IMS_SINT32>(eServiceType));

    SendData2Java(objParcel);
}

PUBLIC
void JniMtcServiceThread::OnPreIncomingCallReceived(
        IN IMS_ULONG nCallKey, IN const AString& strLogTag)
{
    IMS_TRACE_D("OnPreIncomingCallReceived logtag[%s]", strLogTag.GetStr(), 0, 0);
    Parcel objParcel;
    objParcel.writeInt32(IuMtcService::PRE_INCOMING_CALL);
    objParcel.writeInt64(nCallKey);

    objParcel.writeString16(android::String16(strLogTag.GetStr()));

    SendData2Java(objParcel);
}

PUBLIC
void JniMtcServiceThread::OnRejectedIncomingCall(IN IMS_ULONG nCallKey,
        IN const JniCallInfo& objCallInfo, IN const MediaInfo& objMediaInfo,
        IN const ImsList<SuppService*>& objSuppServices, IN OipType eOipType,
        IN const AString& strRemoteNumber, IN const CallReasonInfo& objReason,
        IN const AString& strLogTag)
{
    IMS_TRACE_D("OnRejectedIncomingCall", 0, 0, 0);
    Parcel objParcel;
    objParcel.writeInt32(IuMtcService::AUTO_REJECTED_CALL);
    objParcel.writeInt64(nCallKey);
    objParcel.writeString16(android::String16(strLogTag.GetStr()));

    JniMtcUtils::WriteCallInfoToParcel(objCallInfo, objParcel);
    JniMtcUtils::WriteMediaInfoToParcel(objMediaInfo, objParcel);

    objParcel.writeInt32(static_cast<IMS_SINT32>(eOipType));
    objParcel.writeString16(android::String16(strRemoteNumber.GetStr()));

    JniMtcUtils::WriteSuppServicesToParcel(objSuppServices, objParcel);

    JniMtcUtils::WriteCallReasonInfoToParcel(objReason, objParcel);

    SendData2Java(objParcel);
}

PUBLIC
void JniMtcServiceThread::OnJniReady()
{
    IMS_TRACE_D("OnJniReady", 0, 0, 0);
    Parcel objParcel;
    objParcel.writeInt32(IuMtcService::JNI_READY);

    SendData2Java(objParcel);
}

PUBLIC
void JniMtcServiceThread::OnExternalCallsChanged(
        IN ImsList<const JniExternalCall*>& objJniExternalCalls)
{
    IMS_TRACE_D("OnExternalCallsChanged", 0, 0, 0);
    Parcel objParcel;
    objParcel.writeInt32(IuMtcService::EXTERNAL_CALLS_CHANGED);
    JniMtcUtils::WriteExternalCallsToParcel(objJniExternalCalls, objParcel);

    SendData2Java(objParcel);
}
