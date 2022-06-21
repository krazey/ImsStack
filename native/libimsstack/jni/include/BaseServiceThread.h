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
#ifndef BASE_SERVICE_THREAD_H_
#define BASE_SERVICE_THREAD_H_

#include <binder/Parcel.h>

#include "BaseService.h"
#include "BaseThread.h"

class BaseServiceThread : public BaseThread
{
public:
    BaseServiceThread();
    inline virtual ~BaseServiceThread() {}

public:
    void SetCallback(IN IMS_SINTP nNativeObject, Jni_SendDataToJava pfnSendDataToJava);

protected:
    IMS_BOOL OnMessage(IN ImsMessage& objMsg) override;
    IMS_BOOL SendData2Java(
            IN const android::Parcel& objParcel, IN IMS_BOOL bThreadSwitched = IMS_FALSE);
    virtual IMS_BOOL IsThreadSwitchingRequired(IN IMS_SINT32 nMsg) const;

protected:
    IMS_SINTP m_nNativeObject;
    Jni_SendDataToJava m_pfnSendDataToJava;
    static const IMS_UINT32 MESSAGE_THREAD_SWITCHING = 0;
};

#endif
