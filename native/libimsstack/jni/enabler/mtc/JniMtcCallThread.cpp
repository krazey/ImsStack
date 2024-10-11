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

#include "IuMtcCall.h"
#include "JniMtcCallThread.h"
#include "JniMtcUtils.h"
#include "ServiceTrace.h"
#include <binder/Parcel.h>
#include <utils/String8.h>

using namespace android;

__IMS_TRACE_TAG_USER_DECL__("JNI.MTC");

PUBLIC
JniMtcCallThread::JniMtcCallThread() :
        BaseServiceThread()
{
    IMS_TRACE_D("+JniMtcCallThread", 0, 0, 0);
}

PUBLIC VIRTUAL JniMtcCallThread::~JniMtcCallThread()
{
    IMS_TRACE_D("~JniMtcCallThread", 0, 0, 0);
}

PUBLIC
void JniMtcCallThread::OnStarted(IN const JniCallInfo& objCallInfo,
        IN const MediaInfo& objMediaInfo, IN const ImsMap<SuppType, SuppService*>& objSuppServices)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::STARTED);
    SetCallDetails(objParcel, objCallInfo, objMediaInfo, objSuppServices);
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnStartFailed(IN const CallReasonInfo& objReason)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::START_FAILED);
    JniMtcUtils::WriteCallReasonInfoToParcel(objReason, objParcel);
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnInitiating(
        IN const JniCallInfo& objCallInfo, IN const MediaInfo& objMediaInfo)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::INITIATING);
    JniMtcUtils::WriteCallInfoToParcel(objCallInfo, objParcel);
    JniMtcUtils::WriteMediaInfoToParcel(objMediaInfo, objParcel);
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnProgressing(IN const JniCallInfo& objCallInfo,
        IN const MediaInfo& objMediaInfo, IN const ImsMap<SuppType, SuppService*>& objSuppServices)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::PROGRESSING);
    SetCallDetails(objParcel, objCallInfo, objMediaInfo, objSuppServices);
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnHeld(IN const JniCallInfo& objCallInfo, IN const MediaInfo& objMediaInfo,
        IN const ImsMap<SuppType, SuppService*>& objSuppServices)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::HELD);
    SetCallDetails(objParcel, objCallInfo, objMediaInfo, objSuppServices);
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnHoldFailed(IN const CallReasonInfo& objReason)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::HOLD_FAILED);
    JniMtcUtils::WriteCallReasonInfoToParcel(objReason, objParcel);
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnResumed(IN const JniCallInfo& objCallInfo,
        IN const MediaInfo& objMediaInfo, IN const ImsMap<SuppType, SuppService*>& objSuppServices)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::RESUMED);
    SetCallDetails(objParcel, objCallInfo, objMediaInfo, objSuppServices);
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnResumeFailed(IN const CallReasonInfo& objReason)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::RESUME_FAILED);
    JniMtcUtils::WriteCallReasonInfoToParcel(objReason, objParcel);
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnHeldBy(IN const JniCallInfo& objCallInfo, IN const MediaInfo& objMediaInfo,
        IN const ImsMap<SuppType, SuppService*>& objSuppServices)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::HELD_BY);
    SetCallDetails(objParcel, objCallInfo, objMediaInfo, objSuppServices);
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnResumedBy(IN const JniCallInfo& objCallInfo,
        IN const MediaInfo& objMediaInfo, IN const ImsMap<SuppType, SuppService*>& objSuppServices)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::RESUMED_BY);
    SetCallDetails(objParcel, objCallInfo, objMediaInfo, objSuppServices);
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnTerminated(IN const CallReasonInfo& objReason)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::TERMINATED);
    JniMtcUtils::WriteCallReasonInfoToParcel(objReason, objParcel);
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnIncomingResume(IN const JniCallInfo& objCallInfo,
        IN const MediaInfo& objMediaInfo, IN const ImsMap<SuppType, SuppService*>& objSuppServices)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::INCOMING_RESUME);
    SetCallDetails(objParcel, objCallInfo, objMediaInfo, objSuppServices);
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnIncomingUpdate(IN const JniCallInfo& objCallInfo,
        IN const MediaInfo& objMediaInfo, IN const ImsMap<SuppType, SuppService*>& objSuppServices)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::INCOMING_UPDATE);
    SetCallDetails(objParcel, objCallInfo, objMediaInfo, objSuppServices);
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnUpdated(IN const JniCallInfo& objCallInfo,
        IN const MediaInfo& objMediaInfo, IN const ImsMap<SuppType, SuppService*>& objSuppServices)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::UPDATED);
    SetCallDetails(objParcel, objCallInfo, objMediaInfo, objSuppServices);
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnUpdateFailed(IN const CallReasonInfo& objReason)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::UPDATE_FAILED);
    JniMtcUtils::WriteCallReasonInfoToParcel(objReason, objParcel);
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnUpdatedBy(IN const JniCallInfo& objCallInfo,
        IN const MediaInfo& objMediaInfo, IN const ImsMap<SuppType, SuppService*>& objSuppServices)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::UPDATED_BY);
    SetCallDetails(objParcel, objCallInfo, objMediaInfo, objSuppServices);
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnMerged(IN const JniCallInfo& objCallInfo, IN const MediaInfo& objMediaInfo,
        IN const ImsMap<SuppType, SuppService*>& objSuppServices,
        IN const ImsList<ConfUser*>& objUsers)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::CONF_MERGED);
    SetCallDetails(objParcel, objCallInfo, objMediaInfo, objSuppServices);
    JniMtcUtils::WriteConfUsersToParcel(objUsers, objParcel);
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnMergeFailed(IN const CallReasonInfo& objReason)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::CONF_MERGEFAILED);
    JniMtcUtils::WriteCallReasonInfoToParcel(objReason, objParcel);
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnConferenceParticipantAdded()
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::CONF_JOINED);
    objParcel.writeInt32(1);  // success
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnConferenceParticipantAddFailed(IN const CallReasonInfo& objReason)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::CONF_JOINED);
    objParcel.writeInt32(0);  // failure
    JniMtcUtils::WriteCallReasonInfoToParcel(objReason, objParcel);
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnConferenceParticipantRemoved()
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::CONF_DROPPED);
    objParcel.writeInt32(1);  // success
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnConferenceParticipantRemoveFailed(IN const CallReasonInfo& objReason)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::CONF_DROPPED);
    objParcel.writeInt32(0);  // failure
    JniMtcUtils::WriteCallReasonInfoToParcel(objReason, objParcel);
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnConferenceInfoChanged(IN const AString& /*strDisplayText*/,
        IN const AString /*strSubject*/, IN IMS_UINT32 /*nUserCount*/,
        IN IMS_UINT32 /*nMaxUserCount*/, IN const AString& /*strHost*/)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::CONF_NOTIFY_CONF_INFO);
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnConferenceParticipantsInfoChanged(IN const ImsList<ConfUser*>& objUsers)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::CONF_NOTIFY_USERS_INFO);
    JniMtcUtils::WriteConfUsersToParcel(objUsers, objParcel);
    SendData2Java(objParcel);
}

void JniMtcCallThread::OnEctCompleted(IN IMS_RESULT nResult, IN const CallReasonInfo& objReason)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::ECT_COMPLETED);
    IMS_SINT32 _nResult = nResult == IMS_SUCCESS ? 1 : 0;
    objParcel.writeInt32(_nResult);
    JniMtcUtils::WriteCallReasonInfoToParcel(objReason, objParcel);
    SendData2Java(objParcel);
}

void JniMtcCallThread::OnIncomingCallReceived(IN IMS_UINTP nCallKey,
        IN const JniCallInfo& objCallInfo, IN const MediaInfo& objMediaInfo,
        IN const ImsMap<SuppType, SuppService*>& objSuppServices, IN OipType eOipType,
        IN const AString& strRemoteNumber)
{
    IMS_TRACE_D("OnIncomingCallReceived", 0, 0, 0);
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::INCOMING_CALL_RECEIVED);
    objParcel.writeInt32(nCallKey);

    JniMtcUtils::WriteCallInfoToParcel(objCallInfo, objParcel);
    JniMtcUtils::WriteMediaInfoToParcel(objMediaInfo, objParcel);

    /* Display Info */
    // Need to set callee number when the "child number" is used.
    objParcel.writeInt32(static_cast<IMS_SINT32>(eOipType));
    objParcel.writeString16(android::String16(AString::ConstNull().GetStr()));
    objParcel.writeString16(android::String16(strRemoteNumber.GetStr()));

    /* Supp Info */
    JniMtcUtils::WriteSuppServicesToParcel(objSuppServices, objParcel);

    SendData2Java(objParcel);
}

void JniMtcCallThread::OnInformationNotificationReceived(
        IN IMS_UINT32 eType, IN const AString strValue, IN IMS_SINT32 nValue, IN IMS_BOOL bValue)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::NOTIFY_INFO);
    objParcel.writeInt32(eType);
    objParcel.writeString16(android::String16(strValue.GetStr()));
    objParcel.writeInt32(nValue);
    objParcel.writeInt32(bValue);

    SendData2Java(objParcel);
}

PRIVATE
void JniMtcCallThread::SetCallDetails(IN_OUT Parcel& objParcel, IN const JniCallInfo& objCallInfo,
        IN const MediaInfo& objMediaInfo, IN const ImsMap<SuppType, SuppService*>& objSuppServices)
{
    JniMtcUtils::WriteCallInfoToParcel(objCallInfo, objParcel);
    JniMtcUtils::WriteMediaInfoToParcel(objMediaInfo, objParcel);
    JniMtcUtils::WriteSuppServicesToParcel(objSuppServices, objParcel);
}
