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

#include <utils/String8.h>

#define IMS_STL_USE

#include "EnablerUtils.h"
#include "ImsProcess.h"
#include "IUce.h"
#include "JniUceService.h"
#include "JniUceServiceThread.h"
#include "JniEnablerConnector.h"
#include "ServiceMemory.h"
#include "ServiceMessage.h"
#include "ServiceTrace.h"
#include "IUceJni.h"

__IMS_TRACE_TAG_USER_DECL__("JNI.UCE");

JniUceService::JniUceService(Jni_SendDataToJava pfnSendDataToJava, IN IMS_UINT32 nSimSlot) :
        BaseService(nSimSlot)
{
    IMS_TRACE_D("UCE_M : JniUceService = %" PFLS_u, sizeof(JniUceService), 0, 0);
    //---------------------------------------------------------------------------------------------

    if (pfnSendDataToJava == NULL)
    {
        IMS_TRACE_E(0, "JniUceService:pfnSendDataToJava is null", 0, 0, 0);
        return;
    }
    AString strThreadName;
    strThreadName.Sprintf("JniUceServiceThread%d", GetSlotId());

    auto fnEntry = []() -> BaseThread*
    {
        return new JniUceServiceThread();
    };

    ImsProcess::GetInstance()->LoadThread(strThreadName, fnEntry, GetSlotId());
    m_pJniUceServiceThread =
            DYNAMIC_CAST(JniUceServiceThread*, ImsProcess::GetInstance()->GetThread(strThreadName));

    if (m_pJniUceServiceThread == IMS_NULL)
    {
        IMS_TRACE_E(0, "JniUceService : can't create listener thread", 0, 0, 0);
        return;
    }

    m_pJniUceServiceThread->SetCallback(reinterpret_cast<IMS_SINTP>(this), pfnSendDataToJava);
    JniEnablerConnector::GetInstance().SetJniEnabler(GetSlotId(), EnablerType::UCE, this);
}

JniUceService::~JniUceService()
{
    IMS_TRACE_D("UCE_F : JniUceService = %" PFLS_u, sizeof(JniUceService), 0, 0);
    IMS_TRACE_I("~JniUceService :", 0, 0, 0);
    //---------------------------------------------------------------------------------------------
    JniEnablerConnector::GetInstance().SetJniEnabler(GetSlotId(), EnablerType::UCE, IMS_NULL);

    if (m_pJniUceServiceThread != IMS_NULL)
    {
        ImsProcess::GetInstance()->UnloadAppThread(m_pJniUceServiceThread->GetName());
        m_pJniUceServiceThread = IMS_NULL;
    }
}

PUBLIC VIRTUAL IJniEnablerThread* JniUceService::GetJniThread() const
{
    return DYNAMIC_CAST(IJniEnablerThread*, m_pJniUceServiceThread);
}

int JniUceService::SendData(const Parcel& pParcel)
{
    int nMsg = pParcel.readInt32();

    IMS_TRACE_I("SendData : msg = %d", nMsg, 0, 0);

    if (IsThreadSwitchingRequired(nMsg))
    {
        SendDataUsingEnablerThread(pParcel);
    }
    else
    {
        HandleMessage(nMsg, pParcel);
    }

    return 1;
}

PRIVATE
IUceJni* JniUceService::GetNativeService()
{
    return DYNAMIC_CAST(IUceJni*,
            JniEnablerConnector::GetInstance().GetNativeEnabler(GetSlotId(), EnablerType::UCE));
}

PROTECTED VIRTUAL void JniUceService::HandleMessage(int nMsg, const Parcel& pParcel)
{
    //---------------------------------------------------------------------------------------------
    IUceJni* piUceJni = GetNativeService();
    if (piUceJni == IMS_NULL)
    {
        IMS_TRACE_E(0, "HandleMessage:GetNativeService is null", 0, 0, 0);
        return;
    }

    switch (nMsg)
    {
        case IUUceService::UCE_SEND_PUBLISH_CMD:
        {
            SendPublishCmd(piUceJni, pParcel);
        }
        break;
        case IUUceService::UCE_SEND_SINGLE_SUBSCRIBE_CMD:
        {
            SendSingleSubscribeCmd(piUceJni, pParcel);
        }
        break;
        case IUUceService::UCE_SEND_LIST_SUBSCRIBE_CMD:
        {
            SendListSubscribeCmd(piUceJni, pParcel);
        }
        break;
        case IUUceService::UCE_SEND_OPTIONS_CMD:
        {
            SendOptionsCmd(piUceJni, pParcel);
        }
        break;
        case IUUceService::UCE_SEND_OPTIONS_RESP_CMD:
        {
            SendOptionsRespCmd(piUceJni, pParcel);
        }
        break;
        case IUUceService::UCE_GET_IMS_REGISTRATION_CMD:
        {
            ImsRegistrationCheck(piUceJni);
        }
        break;
        default:
            IMS_TRACE_E(0, "unknown message : %d", nMsg, 0, 0);
            break;
    }
}

PRIVATE
void JniUceService::SendPublishCmd(IUceJni* pJniUce, const Parcel& pParcel)
{
    IMS_TRACE_D("SendPublishCmd", 0, 0, 0);

    IMS_UINT32 key = pParcel.readInt32();

    String16 str16PidfXml = pParcel.readString16();
    String8 str8PidfXml(str16PidfXml);
    AString pidfXml = str8PidfXml.c_str();

    // cppcheck-suppress duplicateAssignExpression
    IMS_UINT32 extended = pParcel.readInt32();
    IMS_UINT32 capability = pParcel.readInt32();

    AString eTag = AString::ConstEmpty();
    if (pParcel.readInt32() == 1)
    {
        String16 str16ETag = pParcel.readString16();
        String8 str8ETag(str16ETag);
        eTag = str8ETag.c_str();
    }

    pJniUce->SendPublishCmd(key, extended, capability, pidfXml, eTag);
}

void JniUceService::SendSingleSubscribeCmd(IUceJni* pJniUce, const Parcel& pParcel)
{
    IMS_TRACE_D("SendSingleSubscribeCmd", 0, 0, 0);

    IMS_UINT32 key = pParcel.readInt32();
    pParcel.readInt32();

    String16 str16User = pParcel.readString16();
    String8 str8User(str16User);
    AString user = str8User.c_str();

    pJniUce->SendSingleSubscribeCmd(key, user);
}

void JniUceService::SendListSubscribeCmd(IUceJni* pJniUce, const Parcel& pParcel)
{
    IMS_TRACE_D("SendListSubscribeCmd", 0, 0, 0);

    // cppcheck-suppress duplicateAssignExpression
    IMS_UINT32 key = pParcel.readInt32();
    IMS_UINT32 size = pParcel.readInt32();
    ImsList<AString> userList;

    for (IMS_SINT32 i = 0; i < size; i++)
    {
        String16 str16User = pParcel.readString16();
        String8 str8User(str16User);
        AString szUser = str8User.c_str();
        userList.Append(szUser);
    }

    pJniUce->SendListSubscribeCmd(key, userList);
}

void JniUceService::SendOptionsCmd(IUceJni* pJniUce, const Parcel& pParcel)
{
    IMS_TRACE_D("SendOptionsCmd", 0, 0, 0);
    IMS_UINT32 key = pParcel.readInt32();

    String16 str16RemoteUri = pParcel.readString16();
    String8 str8RemoteUri(str16RemoteUri);
    AString remoteUri = str8RemoteUri.c_str();

    IMS_UINT32 myCaps = pParcel.readInt32();

    pJniUce->SendOptionsCmd(key, myCaps, remoteUri);
}

void JniUceService::SendOptionsRespCmd(IUceJni* pJniUce, const Parcel& pParcel)
{
    IMS_TRACE_D("SendOptionsRespCmd", 0, 0, 0);

    // cppcheck-suppress duplicateAssignExpression
    IMS_UINT32 key = pParcel.readInt32();
    IMS_SINT32 responseCode = pParcel.readInt32();

    String16 str16Reason = pParcel.readString16();
    String8 str8Reason(str16Reason);

    AString reason = str8Reason.c_str();

    IMS_UINT32 myCaps = pParcel.readInt32();

    pJniUce->SendOptionsRespCmd(key, responseCode, reason, myCaps);
}

void JniUceService::ImsRegistrationCheck(IUceJni* pJniUce)
{
    IMS_TRACE_D("ImsRegistrationCheck", 0, 0, 0);
    pJniUce->ImsRegistrationCheck();
}