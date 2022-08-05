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
#include "ServiceTrace.h"
#include "ImsProcess.h"
#include "EnablerUtils.h"

#include "call/IMtcCall.h"
#include "IMtcCallController.h"
#include "IuMtcService.h"
#include "JniConnector.h"
#include "JniConnectorFactory.h"
#include "JniMtcCall.h"
#include "JniMtcUtils.h"
#include "conferencecall/ConferenceDef.h"

__IMS_TRACE_TAG_USER_DECL__("JNI.MTC");

JniMtcCall::JniMtcCall(IN Jni_SendDataToJava pfnSendDataToJava, IN IMS_SINT32 nSlotId /* = 0*/) :
        m_pThread(IMS_NULL),
        m_pfnSendDataToJava(pfnSendDataToJava),
        m_strThreadName(AString::ConstNull()),
        m_nSlotId(nSlotId),
        m_objCallController(*(JniConnectorFactory::GetInstance()
                                      ->GetMtcCallConnector(m_nSlotId)
                                      ->GetEnablerService())),
        m_nCallKey(IMtcCall::CALL_KEY_INVALID),
        m_pJniMediaSession(IMS_NULL)
{
    IMS_TRACE_D("+JniMtcCall SlotId[%d]", m_nSlotId, 0, 0);
    Initialize();
}

JniMtcCall::~JniMtcCall()
{
    IMS_TRACE_D("~JniMtcCall", 0, 0, 0);

    // m_objCallController.Detach(m_nCallKey);

    if (m_pThread != IMS_NULL)
    {
        ImsProcess::GetInstance()->UnloadAppThread(m_strThreadName);
        m_pThread = IMS_NULL;
    }
    delete m_pJniMediaSession;
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
        SendDataUsingEnablerThread(objParcel, m_nSlotId);
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

    m_strThreadName.Sprintf("JniMtcCallThread_%08" PFLS_x, reinterpret_cast<IMS_UINTP>(this));

    auto fnEntry = []() -> BaseThread*
    {
        return new JniMtcCallThread();
    };
    ImsProcess::GetInstance()->LoadThread(m_strThreadName, fnEntry, m_nSlotId);
    m_pThread = (JniMtcCallThread*)(ImsProcess::GetInstance()->GetThread(m_strThreadName));

    if (m_pThread == IMS_NULL)
    {
        IMS_TRACE_E(0, "JniMtcCall : can't create listener thread", 0, 0, 0);
        return;
    }

    m_pThread->SetSlotId(m_nSlotId);  // TODO: required?
    m_pThread->SetCallback(reinterpret_cast<IMS_SINTP>(this), m_pfnSendDataToJava);
}

PROTECTED VIRTUAL void JniMtcCall::HandleMessage(
        IN IMS_SINT32 nMsg, IN const android::Parcel& objParcel)
{
    IMS_TRACE_D("HandleCallMessage() MSG=[%d]", nMsg, 0, 0);

    switch (nMsg)
    {
        case IuMtcCall::OPEN:
            Open(objParcel);
            break;

        case IuMtcCall::ATTACH:
            Attach(objParcel);
            break;

        case IuMtcCall::START:
            Start(objParcel);
            break;

        case IuMtcCall::USER_ALERT:
            OnUserAlert(objParcel);
            break;

        case IuMtcCall::ACCEPT:
            Accept(objParcel);
            break;

        case IuMtcCall::REJECT:
            Reject(objParcel);
            break;

        case IuMtcCall::HOLD:
            Hold(objParcel);
            break;

        case IuMtcCall::RESUME:
            Resume(objParcel);
            break;

        case IuMtcCall::TERMINATE:
            Terminate(objParcel);
            break;

        case IuMtcCall::UPDATE:
            Update(objParcel);
            break;

        case IuMtcCall::ACCEPT_UPDATE:
            AcceptUpdate(objParcel);
            break;

        case IuMtcCall::REJECT_UPDATE:
            RejectUpdate(objParcel);
            break;

        case IuMtcCall::CANCEL_UPDATE:
            CancelUpdate(objParcel);
            break;

        case IuMtcCall::ACCEPT_RESUME:
            AcceptResume(objParcel);
            break;

        case IuMtcCall::REJECT_RESUME:
            RejectResume(objParcel);
            break;

        case IuMtcCall::SEND_USSD:
            SendUssd(objParcel);
            break;

        case IuMtcCall::STARTCONF:
            StartGroupCall(objParcel);
            break;

        case IuMtcCall::CONF_MERGE:
            MergeToConference(objParcel);
            break;

        case IuMtcCall::CONF_EXPAND:
            ExpandToConference(objParcel);
            break;

        case IuMtcCall::CONF_JOIN:
            AddToConference(objParcel);
            break;

        case IuMtcCall::CONF_DROP:
            RemoveFromConference(objParcel);
            break;
        case IuMtcCall::ECT_START:
            Transfer();
            break;
        case IuMtcCall::ECT_START_BLIND:
            TransferWithNumber(objParcel);
            break;
        default:
            break;
    }
}

PRIVATE
void JniMtcCall::Attach()
{
    IMS_TRACE_D("Attach (%" PFLS_d ")", m_nCallKey, 0, 0);

    // TODO: okay?? need to check timing
    m_pJniMediaSession = new JniMediaSession(
            m_pfnSendDataToJava, m_nSlotId, m_nCallKey, reinterpret_cast<IMS_SINTP>(this));
    m_objCallController.Attach(m_nCallKey, m_pThread, m_pJniMediaSession->GetThread());
}

PRIVATE
void JniMtcCall::Attach(IN const android::Parcel& objParcel)
{
    m_nCallKey = objParcel.readInt64();

    IMS_TRACE_D("Attach for incoming (%" PFLS_d ")", m_nCallKey, 0, 0);

    Attach();
}

PRIVATE
void JniMtcCall::Open(IN const android::Parcel& objParcel)
{
    IMS_TRACE_D("Open (%" PFLS_d ")", m_nCallKey, 0, 0);

    ServiceType eServiceType = JniMtcUtils::ReadServiceType(objParcel);
    JniCallInfo objJniCallInfo = JniMtcUtils::ReadCallInfo(objParcel);

    CallInfo objCallInfo;
    objCallInfo.bEmergency = objJniCallInfo.bEmergency;
    objCallInfo.bOffline = objJniCallInfo.bOffline;
    objCallInfo.bUssi = objJniCallInfo.bUssi;

    m_nCallKey = m_objCallController.Open(eServiceType, objCallInfo);
    Attach();
}

PRIVATE
void JniMtcCall::Start(IN const android::Parcel& objParcel)
{
    CallType eCallType = JniMtcUtils::ReadCallType(objParcel);

    AString strTarget;
    JniMtcUtils::ConvertString(objParcel.readString16(), strTarget);
    MediaInfo* pMediaInfo = JniMtcUtils::ReadMediaInfo(objParcel);
    IMSMap<SuppType, SuppService*> objSuppService =
            JniMtcUtils::ReadSupplementaryService(objParcel);

    m_objCallController.Start(
            m_nCallKey, eCallType, strTarget, pMediaInfo, objSuppService, IMS_NULL);
}

PRIVATE
void JniMtcCall::OnUserAlert(IN const android::Parcel& /*objParcel*/)
{
    m_objCallController.HandleUserAlert(m_nCallKey);
}

PRIVATE
void JniMtcCall::Accept(IN const android::Parcel& objParcel)
{
    m_objCallController.Accept(m_nCallKey, JniMtcUtils::ReadCallType(objParcel),
            JniMtcUtils::ReadMediaInfo(objParcel));
}

PRIVATE
void JniMtcCall::Reject(IN const android::Parcel& objParcel)
{
    m_objCallController.Reject(m_nCallKey, CallReasonInfo(objParcel.readInt32()));
}

PRIVATE
void JniMtcCall::Hold(IN const android::Parcel& objParcel)
{
    m_objCallController.Hold(m_nCallKey, JniMtcUtils::ReadMediaInfo(objParcel));
}

PRIVATE
void JniMtcCall::Resume(IN const android::Parcel& objParcel)
{
    m_objCallController.Resume(m_nCallKey, JniMtcUtils::ReadMediaInfo(objParcel));
}

PRIVATE
void JniMtcCall::Terminate(IN const android::Parcel& objParcel)
{
    m_objCallController.Terminate(m_nCallKey, CallReasonInfo(objParcel.readInt32(), -1));
}

PRIVATE
void JniMtcCall::Update(IN const android::Parcel& objParcel)
{
    m_objCallController.Update(m_nCallKey, JniMtcUtils::ReadCallType(objParcel),
            JniMtcUtils::ReadMediaInfo(objParcel));
}

PRIVATE
void JniMtcCall::AcceptUpdate(IN const android::Parcel& objParcel)
{
    m_objCallController.AcceptUpdate(m_nCallKey, JniMtcUtils::ReadCallType(objParcel),
            JniMtcUtils::ReadMediaInfo(objParcel));
}

PRIVATE
void JniMtcCall::RejectUpdate(IN const android::Parcel& objParcel)
{
    m_objCallController.RejectUpdate(m_nCallKey, CallReasonInfo(objParcel.readInt32()));
}

PRIVATE
void JniMtcCall::CancelUpdate(IN const android::Parcel& objParcel)
{
    m_objCallController.CancelUpdate(m_nCallKey, CallReasonInfo(objParcel.readInt32()));
}

PRIVATE
void JniMtcCall::AcceptResume(IN const android::Parcel& objParcel)
{
    m_objCallController.AcceptResume(m_nCallKey, JniMtcUtils::ReadCallType(objParcel),
            JniMtcUtils::ReadMediaInfo(objParcel));
}

PRIVATE
void JniMtcCall::RejectResume(IN const android::Parcel& objParcel)
{
    m_objCallController.RejectResume(m_nCallKey, CallReasonInfo(objParcel.readInt32()));
}

PRIVATE
void JniMtcCall::SendUssd(IN const android::Parcel& objParcel)
{
    android::String8 str8(objParcel.readString16());
    AString strUssd = str8.string();
    m_objCallController.SendUssd(m_nCallKey, strUssd);
}

PRIVATE
void JniMtcCall::StartGroupCall(IN const android::Parcel& /*objParcel*/)
{
    /*
    CallType eCallType = JniMtcUtils::ReadCallType(objParcel);

    AString strTarget;
    JniMtcUtils::ConvertString(objParcel.readString16(), strTarget);
    MediaInfo* pMediaInfo = JniMtcUtils::ReadMediaInfo(objParcel);
    IMSMap<SuppType, SuppService*> objSuppService = JniMtcUtils::ReadSupplementaryService(
            objParcel);

    m_objCallController.StartGroupCall(m_nCallKey, eCallType, strTarget,
            pMediaInfo, objSuppService);
    */
}

PRIVATE
void JniMtcCall::MergeToConference(IN const android::Parcel& objParcel)
{
    // TODO: delete pointers after function call.
    IMSList<ConfUser*> objUsers = JniMtcUtils::ReadConferenceParticipants(objParcel);
    for (IMS_UINT32 i = 0; i < objUsers.GetSize(); i++)
    {
        IMS_TRACE_D("MergeToConference connectionId=[%d]", objUsers.GetAt(i)->nConnectionId, 0, 0);
    }
    m_objCallController.MergeToConference(m_nCallKey, objUsers);
}

PRIVATE
void JniMtcCall::ExpandToConference(IN const android::Parcel& /*objParcel*/)
{
    // m_objCallController.ExpandToConference(m_nCallKey, ReadConferenceParticipants(objParcel));
}

PRIVATE
void JniMtcCall::AddToConference(IN const android::Parcel& objParcel)
{
    IMSList<ConfUser*> objUsers = JniMtcUtils::ReadConferenceParticipants(objParcel);
    for (IMS_UINT32 i = 0; i < objUsers.GetSize(); i++)
    {
        IMS_TRACE_D("AddToConference nConnectionId=[%d]", objUsers.GetAt(i)->nConnectionId, 0, 0);
    }
    m_objCallController.AddToConference(m_nCallKey, objUsers);
}

PRIVATE
void JniMtcCall::RemoveFromConference(IN const android::Parcel& objParcel)
{
    IMSList<ConfUser*> objUsers = JniMtcUtils::ReadConferenceParticipants(objParcel);
    for (IMS_UINT32 i = 0; i < objUsers.GetSize(); i++)
    {
        IMS_TRACE_D(
                "RemoveFromConference nConnectionId=[%d]", objUsers.GetAt(i)->nConnectionId, 0, 0);
    }
    m_objCallController.RemoveFromConference(m_nCallKey, objUsers);
}

PRIVATE
void JniMtcCall::Transfer()
{
    m_objCallController.Transfer(m_nCallKey, AString::ConstNull());
}

PRIVATE
void JniMtcCall::TransferWithNumber(IN const android::Parcel& objParcel)
{
    AString strTarget;
    JniMtcUtils::ConvertString(objParcel.readString16(), strTarget);
    m_objCallController.Transfer(m_nCallKey, strTarget);
}
