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

#ifndef JNI_SIP_CONTROLLER_SERVICE_THREAD_H_
#define JNI_SIP_CONTROLLER_SERVICE_THREAD_H_

#include "BaseService.h"
#include "ImsAppThread.h"

class JniSipControllerServiceThread : public ImsAppThread
{
public:
    JniSipControllerServiceThread();

public:
    static ImsAppThread* GetInstance();
    virtual ~JniSipControllerServiceThread();

    int SetCallback(IN IMS_UINTP nNativeObj, IN Jni_SendDataToJava pfnSendDataToJava);

private:
    virtual IMS_BOOL Initialize();
    virtual void Uninitialize();
    virtual IMS_BOOL OnStart(IN IMSMSG& objMSG);
    virtual IMS_BOOL OnTerminate(IN IMSMSG& objMSG);
    virtual IMS_BOOL OnMessage(IN IMSMSG& objMSG);
    void HandleMsg(IN IMSMSG& objMSG);
    inline void WriteStringToParcel(IN CONST IMS_CHAR* pszValue, OUT android::Parcel& parcel);

private:
    IMS_UINTP m_nNativeObj;
    Jni_SendDataToJava m_pfnSendDataToJava;
};

#endif  // JNI_SIP_CONTROLLER_SERVICE_THREAD_H_
