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

#ifndef _IMS_UCE_SERVICE_H_
#define _IMS_UCE_SERVICE_H_

#include "BaseService.h"

class JniUceServiceThread;

using namespace android;

class JniUceService : public BaseService
{
public:
    JniUceService(IN IMS_UINT32 nSimSlot = 0);
    JniUceService(Jni_SendDataToJava pfnSendDataToJava, IN IMS_UINT32 nSimSlot = 0);
    virtual ~JniUceService();

    virtual int SendData(const Parcel& pParcel);

private:
    void HandleMessage(int nMsg, const Parcel& pParcel);

private:
    JniUceServiceThread* m_pJniUceServiceThread;
    IMS_UINT32 m_nSimSlot;
    AString m_strTarget;
};

#endif  //_IMS_PEOPLE_SERVICE_H_
