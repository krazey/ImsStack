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

#ifndef _IMS_UCE_SERVICE_THREAD_H_
#define _IMS_UCE_SERVICE_THREAD_H_

#include "BaseService.h"
#include "ImsAppThread.h"

class JniUceServiceThread : public ImsAppThread
{
private:
    JniUceServiceThread();

public:
    static ImsAppThread* GetInstance();
    virtual ~JniUceServiceThread();

    int SetCallback(IN IMS_UINTP nNativeObj, IN Jni_SendDataToJava pfnSendDataToJava);

protected:
    virtual IMS_BOOL Initialize();
    virtual void Uninitialize() override;

    virtual IMS_BOOL OnStart(IN IMSMSG& objMSG);
    virtual IMS_BOOL OnTerminate(IN IMSMSG& objMSG);
    virtual IMS_BOOL OnMessage(IN IMSMSG& objMSG);

private:
    IMS_UINTP m_nNativeObj;
    Jni_SendDataToJava m_pfnSendDataToJava;
};

#endif
