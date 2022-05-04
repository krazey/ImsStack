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
#ifndef JNI_SYSTEM_H_
#define JNI_SYSTEM_H_

#include "BaseService.h"
#include "SystemCallback.h"

class JniSystem : public BaseService, public SystemCallback
{
public:
    JniSystem(IN JniSystem_SendDataToJava pSendDataToJava);
    virtual ~JniSystem();

    JniSystem(IN const JniSystem&) = delete;
    JniSystem& operator=(IN const JniSystem&) = delete;

public:
    // BaseService class
    int SendData(IN const android::Parcel& in) override;
    int SendData(IN const android::Parcel& in, OUT android::Parcel& out) override;

    // SystemCallback class
    IMS_SINT32 SendDataToJava(IN const android::Parcel& in, OUT android::Parcel& out,
            int fileDescriptor = -1) override;

private:
    JniSystem_SendDataToJava m_pSendDataToJava;
};

#endif
