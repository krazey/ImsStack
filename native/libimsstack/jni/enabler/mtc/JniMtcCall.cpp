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
#include "CallReasonInfo.h"
#include "EnablerUtils.h"
#include "IJniEnablerThread.h"
#include "IMtcCallController.h"
#include "INativeEnabler.h"
#include "IThread.h"
#include "ImsList.h"
#include "ImsProcess.h"
#include "IuMtcCall.h"
#include "IuMtcService.h"
#include "JniEnablerConnector.h"
#include "JniMediaSession.h"
#include "JniMtcCall.h"
#include "JniMtcCallThread.h"
#include "JniMtcUtils.h"
#include "ServiceTrace.h"
#include "call/IMtcCall.h"
#include "conferencecall/ConferenceDef.h"

__IMS_TRACE_TAG_USER_DECL__("JNI.MTC");

JniMtcCall::JniMtcCall(IN Jni_SendDataToJava pfnSendDataToJava, IN IMS_SINT32 nSlotId /* = 0*/) :
        BaseService(nSlotId),
        m_pThread(IMS_NULL),
        m_pfnSendDataToJava(pfnSendDataToJava),
        m_nCallKey(IMtcCall::CALL_KEY_INVALID),
        m_pJniMediaSession(IMS_NULL)
{
    IMS_TRACE_D("+JniMtcCall SlotId[%d]", nSlotId, 0, 0);
    Initialize();
}

JniMtcCall::~JniMtcCall()
{
    IMS_TRACE_D("~JniMtcCall CallKey[%d]", m_nCallKey, 0, 0);

    JniEnablerConnector::GetInstance().SetJniEnabler(
            GetSlotId(), EnablerType::MTC_CALL, IMS_NULL, m_nCallKey);
    if (m_pThread)
    {
        ImsProcess::GetInstance()->UnloadAppThread(m_pThread->GetName());
    }
    delete m_pJniMediaSession;

    IMtcCallController* piCallController = GetCallController();
    if (piCallController)
    {
        piCallController->Detach(m_nCallKey);
    }
}

PUBLIC VIRTUAL void JniMtcCall::Destroy()
{
    IMS_TRACE_D("Destroy", 0, 0, 0);
    ImsMessage objMsg(MSG_DESTROY, 0, 0, this);
    IThread* piThread = ImsProcess::GetInstance()
                                ->GetThread(EnablerUtils::GetEnablerThreadName(m_nSlotId))
                                ->GetThread();
    piThread->PostMessageI(objMsg);
}

PUBLIC VIRTUAL int JniMtcCall::SendData(IN const android::Parcel& objParcel)
{
    int nMsg = objParcel.readInt32();

    if (JniMediaSession::IsMediaMessage(nMsg) && m_pJniMediaSession)
    {
        objParcel.setDataPosition(0);
        return m_pJniMediaSession->SendData(objParcel);
    }

    if (IsThreadSwitchingRequired(nMsg))
    {
        SendDataUsingEnablerThread(objParcel);
    }
    else
    {
        HandleMessage(nMsg, objParcel);
    }

    return 1;
}

PUBLIC
void JniMtcCall::Initialize()
{
    if (m_pfnSendDataToJava == IMS_NULL)
    {
        return;
    }

    AString strThreadName;
    strThreadName.Sprintf("JniMtcCallThread_%08" PFLS_x, reinterpret_cast<IMS_UINTP>(this));

    auto fnEntry = []() -> BaseThread*
    {
        return new JniMtcCallThread();
    };
    ImsProcess::GetInstance()->LoadThread(strThreadName, fnEntry, GetSlotId());
    m_pThread = reinterpret_cast<JniMtcCallThread*>(
            ImsProcess::GetInstance()->GetThread(strThreadName));

    if (m_pThread == IMS_NULL)
    {
        IMS_TRACE_E(0, "JniMtcCall : can't create listener thread", 0, 0, 0);
        return;
    }

    m_pThread->SetCallback(reinterpret_cast<IMS_SINTP>(this), m_pfnSendDataToJava);
}

PUBLIC VIRTUAL IJniEnablerThread* JniMtcCall::GetJniThread() const
{
    return DYNAMIC_CAST(IJniEnablerThread*, m_pThread);
}

PROTECTED VIRTUAL void JniMtcCall::HandleMessage(
        IN IMS_SINT32 nMsg, IN const android::Parcel& objParcel)
{
    IMS_TRACE_D("HandleCallMessage() CallKey[%d] Message=[%d]", m_nCallKey, nMsg, 0);
    IMtcCallController* piCallController = GetCallController();
    if (!piCallController)
    {
        IMS_TRACE_E(0, "INativeEnabler for MTC_CALL is null", 0, 0, 0);
        return;
    }
    switch (nMsg)
    {
        case IuMtcCall::OPEN:
            Open(*piCallController, objParcel);
            break;

        case IuMtcCall::ATTACH:
            Attach(*piCallController, objParcel);
            break;

        case IuMtcCall::START:
            Start(*piCallController, objParcel);
            break;

        case IuMtcCall::USER_ALERT:
            OnUserAlert(*piCallController, objParcel);
            break;

        case IuMtcCall::ACCEPT:
            Accept(*piCallController, objParcel);
            break;

        case IuMtcCall::REJECT:
            Reject(*piCallController, objParcel);
            break;

        case IuMtcCall::HOLD:
            Hold(*piCallController, objParcel);
            break;

        case IuMtcCall::RESUME:
            Resume(*piCallController, objParcel);
            break;

        case IuMtcCall::TERMINATE:
            Terminate(*piCallController, objParcel);
            break;

        case IuMtcCall::UPDATE:
            Update(*piCallController, objParcel);
            break;

        case IuMtcCall::ACCEPT_UPDATE:
            AcceptUpdate(*piCallController, objParcel);
            break;

        case IuMtcCall::REJECT_UPDATE:
            RejectUpdate(*piCallController, objParcel);
            break;

        case IuMtcCall::CANCEL_UPDATE:
            CancelUpdate(*piCallController, objParcel);
            break;

        case IuMtcCall::ACCEPT_RESUME:
            AcceptResume(*piCallController, objParcel);
            break;

        case IuMtcCall::REJECT_RESUME:
            RejectResume(*piCallController, objParcel);
            break;

        case IuMtcCall::SEND_USSD:
            SendUssd(*piCallController, objParcel);
            break;

        case IuMtcCall::STARTCONF:
            StartGroupCall(*piCallController, objParcel);
            break;

        case IuMtcCall::CONF_MERGE:
            MergeToConference(*piCallController, objParcel);
            break;

        case IuMtcCall::CONF_EXPAND:
            ExpandToConference(*piCallController, objParcel);
            break;

        case IuMtcCall::CONF_JOIN:
            AddToConference(*piCallController, objParcel);
            break;

        case IuMtcCall::CONF_DROP:
            RemoveFromConference(*piCallController, objParcel);
            break;

        case IuMtcCall::ECT_START:
            Transfer(*piCallController);
            break;

        case IuMtcCall::ECT_START_BLIND:
            TransferWithNumber(*piCallController, objParcel);
            break;

        default:
            break;
    }
}

PROTECTED VIRTUAL IMS_BOOL JniMtcCall::IsThreadSwitchingRequired(IN IMS_SINT32 nMsg) const
{
    switch (nMsg)
    {
        case IuMtcCall::TERMINATE:
            return IMS_FALSE;
        default:
            return IMS_TRUE;
    }
}

PRIVATE
void JniMtcCall::Attach(IN IMtcCallController& objCallController)
{
    IMS_TRACE_D("Attach Key[%d]", m_nCallKey, 0, 0);

    JniEnablerConnector::GetInstance().SetJniEnabler(
            GetSlotId(), EnablerType::MTC_CALL, this, m_nCallKey);

    m_pJniMediaSession = new JniMediaSession(
            m_pfnSendDataToJava, GetSlotId(), m_nCallKey, reinterpret_cast<IMS_SINTP>(this));
    objCallController.Attach(m_nCallKey);
}

PRIVATE
void JniMtcCall::Attach(
        IN IMtcCallController& objCallController, IN const android::Parcel& objParcel)
{
    m_nCallKey = objParcel.readInt64();

    IMS_TRACE_D("Attach for incoming call case Key[%d]", m_nCallKey, 0, 0);
    Attach(objCallController);
}

PRIVATE
void JniMtcCall::Open(IN IMtcCallController& objCallController, IN const android::Parcel& objParcel)
{
    IMS_TRACE_D("Open Key[%d]", m_nCallKey, 0, 0);

    const ServiceType eServiceType = JniMtcUtils::ReadServiceType(objParcel);
    const JniCallInfo objJniCallInfo = JniMtcUtils::ReadCallInfo(objParcel);
    AString strLogTag;
    JniMtcUtils::ConvertString(objParcel.readString16(), strLogTag);

    CallInfo objCallInfo;
    objCallInfo.eInitialCallType = objJniCallInfo.eCallType;
    objCallInfo.eEmergencyType = objJniCallInfo.eEmergencyType;
    objCallInfo.bOffline = objJniCallInfo.bOffline;
    objCallInfo.bUssi = objJniCallInfo.bUssi;

    m_nCallKey = objCallController.Open(eServiceType, objCallInfo, strLogTag);

    if (m_nCallKey != IMtcCall::CALL_KEY_INVALID)
    {
        Attach(objCallController);
    }
}

PRIVATE
void JniMtcCall::Start(
        IN IMtcCallController& objCallController, IN const android::Parcel& objParcel)
{
    if (m_nCallKey == IMtcCall::CALL_KEY_INVALID)
    {
        IMS_TRACE_E(0, "Invalid call key.", 0, 0, 0);
        return m_pThread->OnStartFailed(CallReasonInfo(
                CODE_LOCAL_INTERNAL_ERROR, EXTRA_CODE_INTERNAL_ERROR_INVALID_CALL_KEY));
    }

    CallType eCallType = JniMtcUtils::ReadCallType(objParcel);

    AString strTarget;
    JniMtcUtils::ConvertString(objParcel.readString16(), strTarget);
    MediaInfo objMediaInfo = JniMtcUtils::ReadMediaInfo(objParcel, objMediaInfo);
    ImsList<SuppService*> objSuppServices = JniMtcUtils::ReadSupplementaryService(objParcel);

    objCallController.Start(m_nCallKey, eCallType, strTarget, objMediaInfo, objSuppServices);
}

PRIVATE
void JniMtcCall::OnUserAlert(
        IN IMtcCallController& objCallController, IN const android::Parcel& /*objParcel*/)
{
    objCallController.HandleUserAlert(m_nCallKey);
}

PRIVATE
void JniMtcCall::Accept(
        IN IMtcCallController& objCallController, IN const android::Parcel& objParcel)
{
    MediaInfo objMediaInfo;
    objCallController.Accept(m_nCallKey, JniMtcUtils::ReadCallType(objParcel),
            JniMtcUtils::ReadMediaInfo(objParcel, objMediaInfo));
}

PRIVATE
void JniMtcCall::Reject(
        IN IMtcCallController& objCallController, IN const android::Parcel& objParcel)
{
    objCallController.Reject(m_nCallKey, CallReasonInfo(objParcel.readInt32()));
}

PRIVATE
void JniMtcCall::Hold(IN IMtcCallController& objCallController, IN const android::Parcel& objParcel)
{
    MediaInfo objMediaInfo;
    objCallController.Hold(m_nCallKey, JniMtcUtils::ReadMediaInfo(objParcel, objMediaInfo));
}

PRIVATE
void JniMtcCall::Resume(
        IN IMtcCallController& objCallController, IN const android::Parcel& objParcel)
{
    MediaInfo objMediaInfo;
    objCallController.Resume(m_nCallKey, JniMtcUtils::ReadMediaInfo(objParcel, objMediaInfo));
}

PRIVATE
void JniMtcCall::Terminate(
        IN IMtcCallController& objCallController, IN const android::Parcel& objParcel)
{
    objCallController.Terminate(m_nCallKey, CallReasonInfo(objParcel.readInt32(), -1));
}

PRIVATE
void JniMtcCall::Update(
        IN IMtcCallController& objCallController, IN const android::Parcel& objParcel)
{
    MediaInfo objMediaInfo;
    objCallController.Update(m_nCallKey, JniMtcUtils::ReadCallType(objParcel),
            JniMtcUtils::ReadMediaInfo(objParcel, objMediaInfo));
}

PRIVATE
void JniMtcCall::AcceptUpdate(
        IN IMtcCallController& objCallController, IN const android::Parcel& objParcel)
{
    MediaInfo objMediaInfo;
    objCallController.AcceptUpdate(m_nCallKey, JniMtcUtils::ReadCallType(objParcel),
            JniMtcUtils::ReadMediaInfo(objParcel, objMediaInfo));
}

PRIVATE
void JniMtcCall::RejectUpdate(
        IN IMtcCallController& objCallController, IN const android::Parcel& objParcel)
{
    objCallController.RejectUpdate(m_nCallKey, CallReasonInfo(objParcel.readInt32()));
}

PRIVATE
void JniMtcCall::CancelUpdate(
        IN IMtcCallController& objCallController, IN const android::Parcel& objParcel)
{
    objCallController.CancelUpdate(m_nCallKey, CallReasonInfo(objParcel.readInt32()));
}

PRIVATE
void JniMtcCall::AcceptResume(
        IN IMtcCallController& objCallController, IN const android::Parcel& objParcel)
{
    MediaInfo objMediaInfo;
    objCallController.AcceptResume(m_nCallKey, JniMtcUtils::ReadCallType(objParcel),
            JniMtcUtils::ReadMediaInfo(objParcel, objMediaInfo));
}

PRIVATE
void JniMtcCall::RejectResume(
        IN IMtcCallController& objCallController, IN const android::Parcel& objParcel)
{
    objCallController.RejectResume(m_nCallKey, CallReasonInfo(objParcel.readInt32()));
}

PRIVATE
void JniMtcCall::SendUssd(
        IN IMtcCallController& objCallController, IN const android::Parcel& objParcel)
{
    android::String8 str8(objParcel.readString16());
    AString strUssd = str8.c_str();
    objCallController.SendUssd(m_nCallKey, strUssd);
}

PRIVATE
void JniMtcCall::StartGroupCall(
        IN IMtcCallController& /*objCallController*/, IN const android::Parcel& /*objParcel*/)
{
    /*
    if (m_nCallKey != IMtcCall::CALL_KEY_INVALID)
    {
        return m_pThread->OnStartFailed(CallReasonInfo(CODE_LOCAL_NOT_REGISTERED));
    }

    CallType eCallType = JniMtcUtils::ReadCallType(objParcel);

    AString strTarget;
    JniMtcUtils::ConvertString(objParcel.readString16(), strTarget);
    MediaInfo* pMediaInfo = JniMtcUtils::ReadMediaInfo(objParcel);
    ImsList<SuppService*> objSuppServices = JniMtcUtils::ReadSupplementaryService(
            objParcel);

    objCallController.StartGroupCall(m_nCallKey, eCallType, strTarget,
            pMediaInfo, objSuppServices);
    */
}

PRIVATE
void JniMtcCall::MergeToConference(
        IN IMtcCallController& objCallController, IN const android::Parcel& objParcel)
{
    // TODO: delete pointers after function call.
    ImsList<ConfUser*> objUsers = JniMtcUtils::ReadConferenceParticipants(objParcel);
    for (IMS_UINT32 i = 0; i < objUsers.GetSize(); i++)
    {
        IMS_TRACE_D("MergeToConference connectionId=[%d]", objUsers.GetAt(i)->nConnectionId, 0, 0);
    }
    objCallController.MergeToConference(m_nCallKey, objUsers);
}

PRIVATE
void JniMtcCall::ExpandToConference(
        IN IMtcCallController& /*objCallController*/, IN const android::Parcel& /*objParcel*/)
{
    // objCallController.ExpandToConference(m_nCallKey, ReadConferenceParticipants(objParcel));
}

PRIVATE
void JniMtcCall::AddToConference(
        IN IMtcCallController& objCallController, IN const android::Parcel& objParcel)
{
    ImsList<ConfUser*> objUsers = JniMtcUtils::ReadConferenceParticipants(objParcel);
    for (IMS_UINT32 i = 0; i < objUsers.GetSize(); i++)
    {
        IMS_TRACE_D("AddToConference nConnectionId=[%d]", objUsers.GetAt(i)->nConnectionId, 0, 0);
    }
    objCallController.AddToConference(m_nCallKey, objUsers);
}

PRIVATE
void JniMtcCall::RemoveFromConference(
        IN IMtcCallController& objCallController, IN const android::Parcel& objParcel)
{
    ImsList<ConfUser*> objUsers = JniMtcUtils::ReadConferenceParticipants(objParcel);
    for (IMS_UINT32 i = 0; i < objUsers.GetSize(); i++)
    {
        IMS_TRACE_D(
                "RemoveFromConference nConnectionId=[%d]", objUsers.GetAt(i)->nConnectionId, 0, 0);
    }
    objCallController.RemoveFromConference(m_nCallKey, objUsers);
}

PRIVATE
void JniMtcCall::Transfer(IN IMtcCallController& objCallController)
{
    objCallController.Transfer(m_nCallKey, AString::ConstNull());
}

PRIVATE
void JniMtcCall::TransferWithNumber(
        IN IMtcCallController& objCallController, IN const android::Parcel& objParcel)
{
    AString strTarget;
    JniMtcUtils::ConvertString(objParcel.readString16(), strTarget);
    objCallController.Transfer(m_nCallKey, strTarget);
}

PRIVATE
IMtcCallController* JniMtcCall::GetCallController() const
{
    return DYNAMIC_CAST(IMtcCallController*,
            JniEnablerConnector::GetInstance().GetNativeEnabler(
                    GetSlotId(), EnablerType::MTC_CALL));
}
