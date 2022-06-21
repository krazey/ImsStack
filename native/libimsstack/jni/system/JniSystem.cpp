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
#include "JniSystem.h"
#include "System.h"

PUBLIC
JniSystem::JniSystem(IN JniSystem_SendDataToJava pfnSendDataToJava) :
        m_pfnSendDataToJava(pfnSendDataToJava)
{
    if (m_pfnSendDataToJava != IMS_NULL)
    {
        System::GetInstance()->SetCallback(this);
    }
}

PUBLIC
JniSystem::~JniSystem() {}

PUBLIC VIRTUAL int JniSystem::SendData(IN const android::Parcel& /*in*/)
{
    // This method is not used for system interface.
    return 0;
}

PUBLIC VIRTUAL int JniSystem::SendData(IN const android::Parcel& in, OUT android::Parcel& out)
{
    System::GetInstance()->NotifyData(in, out);
    return 1;
}

PUBLIC VIRTUAL IMS_SINT32 JniSystem::SendDataToJava(
        IN const android::Parcel& in, OUT android::Parcel& out, int fileDescriptor /*= -1*/)
{
    if (m_pfnSendDataToJava != IMS_NULL)
    {
        return m_pfnSendDataToJava(reinterpret_cast<IMS_SINTP>(this), in, out, fileDescriptor);
    }

    return 0;
}
