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
#include "IThread.h"
#include "ServiceTrace.h"

#include "BaseService.h"
#include "BaseThread.h"

__IMS_TRACE_TAG_USER_DECL__("JNI");

PUBLIC
BaseService::BaseService(IN IMS_SINT32 nSlotId) :
        m_nSlotId(nSlotId)
{
}

PUBLIC VIRTUAL BaseService::~BaseService()
{
    if (m_nSlotId > IMS_SLOT_ANY)
    {
        RemovePendingMessages();
    }
}

PUBLIC void BaseService::Destroy()
{
    delete this;
}

PROTECTED
void BaseService::MessageCallback_OnMessage(IN ImsMessage& objMsg)
{
    if (objMsg.nMSG == MSG_DESTROY)
    {
        delete this;
        return;
    }
    android::Parcel* pParcel = reinterpret_cast<android::Parcel*>(objMsg.nLparam);
    pParcel->setDataPosition(0);
    pParcel->readInt32();  // consumes nMsg

    HandleMessage(objMsg.nMSG, *pParcel);
    delete pParcel;
    objMsg.nLparam = 0;
}

PROTECTED
IMS_UINT32 BaseService::RemovePendingMessages()
{
    BaseThread* pEnablerThread =
            ImsProcess::GetInstance()->GetThread(EnablerUtils::GetEnablerThreadName(m_nSlotId));
    IThread* piThread = (pEnablerThread != IMS_NULL) ? pEnablerThread->GetThread() : IMS_NULL;
    if (piThread != IMS_NULL)
    {
        ImsList<ImsMessage> objImsMsgs;

        if (piThread->RemoveMessages(this, &objImsMsgs) > 0)
        {
            for (IMS_UINT32 i = 0; i < objImsMsgs.GetSize(); ++i)
            {
                const ImsMessage& objMsg = objImsMsgs.GetAt(i);
                android::Parcel* pParcel = reinterpret_cast<android::Parcel*>(objMsg.nLparam);

                if (pParcel != IMS_NULL)
                {
                    delete pParcel;
                }
            }

            IMS_TRACE_D("RemovePendingMessages: slotId=%d, count=%d, %p", m_nSlotId,
                    objImsMsgs.GetSize(), this);

            return objImsMsgs.GetSize();
        }
    }

    return 0;
}

PROTECTED
void BaseService::SendDataUsingEnablerThread(IN const android::Parcel& objParcel)
{
    android::Parcel* pParcelOut = new android::Parcel();
    pParcelOut->write(objParcel.data(), objParcel.dataSize());
    pParcelOut->setDataPosition(0);

    ImsMessage objMsg(pParcelOut->readInt32(), 0, reinterpret_cast<IMS_UINTP>(pParcelOut), this);
    IThread* piThread = ImsProcess::GetInstance()
                                ->GetThread(EnablerUtils::GetEnablerThreadName(m_nSlotId))
                                ->GetThread();
    piThread->PostMessageI(objMsg);
}
