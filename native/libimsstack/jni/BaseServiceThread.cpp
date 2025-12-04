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
#include "BaseServiceThread.h"
#include "IThread.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_USER_DECL__("JNI");

PUBLIC
BaseServiceThread::BaseServiceThread() :
        BaseThread(),
        m_nNativeObject(0),
        m_pfnSendDataToJava(IMS_NULL)
{
}

PUBLIC
void BaseServiceThread::SetCallback(
        IN IMS_SINTP nNativeObject, Jni_SendDataToJava pfnSendDataToJava)
{
    m_nNativeObject = nNativeObject;
    m_pfnSendDataToJava = pfnSendDataToJava;
}

PROTECTED VIRTUAL IMS_BOOL BaseServiceThread::OnMessage(IN ImsMessage& objMsg)
{
    IMS_TRACE_D("OnMessage (%d)", objMsg.GetName(), 0, 0);

    switch (objMsg.GetName())
    {
        case MESSAGE_THREAD_SWITCHING:
        {
            android::Parcel* pParcel = reinterpret_cast<android::Parcel*>(objMsg.nLparam);
            SendData2Java(*pParcel, IMS_TRUE);
            delete pParcel;
            break;
        }
        default:
            break;
    }

    return IMS_TRUE;
}

PROTECTED
IMS_BOOL BaseServiceThread::SendData2Java(
        IN const android::Parcel& objParcel, IN IMS_BOOL bThreadSwitched /*= IMS_FALSE*/)
{
    if (m_nNativeObject == 0)
    {
        return IMS_FALSE;
    }

    if (m_pfnSendDataToJava == IMS_NULL)
    {
        return IMS_FALSE;
    }

    int nMsg = objParcel.readInt32();
    objParcel.setDataPosition(0);
    IMS_BOOL bSendCurrentThread = bThreadSwitched || !IsThreadSwitchingRequired(nMsg);

    IMS_TRACE_D("SendData2Java (%s)", _TRACE_B_(bSendCurrentThread), 0, 0);

    if (bSendCurrentThread)
    {
        (*m_pfnSendDataToJava)(m_nNativeObject, objParcel);
        return IMS_TRUE;
    }

    android::Parcel* pParcelOut = new android::Parcel();
    pParcelOut->write(objParcel.data(), objParcel.dataSize());
    pParcelOut->setDataPosition(0);

    IThread* piThread = GetThread();

    if (piThread != IMS_NULL)
    {
        piThread->PostMessageI(
                MESSAGE_THREAD_SWITCHING, 0, reinterpret_cast<IMS_UINTP>(pParcelOut));
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL BaseServiceThread::IsThreadSwitchingRequired(
        IN IMS_SINT32 /*nMsg*/) const
{
    return IMS_TRUE;
}
