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

#include "JniSipControllerServiceThread.h"

#include <binder/Parcel.h>
#include "IURcsMessageService.h"
#include "ImsMessageDef.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"

using namespace android;

__IMS_TRACE_TAG_USER_DECL__("IMS_SNC");

PUBLIC JniSipControllerServiceThread::JniSipControllerServiceThread()
{
    IMS_TRACE_I("+JniSipControllerServiceThread : ", 0, 0, 0);
    IMS_TRACE_MEM("SNC_MSG", "IM_M : JniSipControllerServiceThread = %" PFLS_u,
            sizeof(JniSipControllerServiceThread), 0, 0);
}

PUBLIC VIRTUAL JniSipControllerServiceThread::~JniSipControllerServiceThread()
{
    IMS_TRACE_I("-JniSipControllerServiceThread : ", 0, 0, 0);
    IMS_TRACE_MEM("SNC_MSG", "IM_F : JniSipControllerServiceThread = %" PFLS_u,
            sizeof(JniSipControllerServiceThread), 0, 0);
}

PUBLIC void JniSipControllerServiceThread::OnMessageReceived()
{
    IMS_TRACE_I("OnMessageReceived : ", 0, 0, 0);
    // TODO Implementation
}

PUBLIC void JniSipControllerServiceThread::OnMessageSent()
{
    IMS_TRACE_I("OnMessageSent : ", 0, 0, 0);
    // TODO Implementation
}

PUBLIC void JniSipControllerServiceThread::OnMessageSendFailure()
{
    IMS_TRACE_I("OnMessageSendFailure : ", 0, 0, 0);
    // TODO Implementation
}

PUBLIC void JniSipControllerServiceThread::OnRegistrationUpdated(IN IMS_UINTP nParam)
{
    IMS_TRACE_I("OnRegistrationUpdated : ", 0, 0, 0);
    IUSncFeatureTagsParam* pParam = reinterpret_cast<IUSncFeatureTagsParam*>(nParam);

    Parcel objParcel;
    objParcel.writeInt32(IUSncControl::ONREGISTRATION_UPDATED_IND);
    objParcel.writeInt32(pParam->m_nFeatureCount);
    objParcel.writeInt32(pParam->m_nRegState);
    objParcel.writeInt32(pParam->m_nReason);
    for (IMS_UINT32 i = 0; i < pParam->m_nFeatureCount; i++)
    {
        objParcel.writeString16(
                android::String16(pParam->m_objFeatureTags.GetElementAt(i).GetStr()));
    }
    SendData2Java(objParcel);
}

PUBLIC void JniSipControllerServiceThread::OnConfigurationUpdated()
{
    IMS_TRACE_I("OnConfigurationUpdated : ", 0, 0, 0);
    // TODO Implementation
}
