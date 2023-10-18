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

#include "BaseServiceThread.h"
#include "IJniSipControllerServiceThread.h"

class JniSipControllerServiceThread :
        public BaseServiceThread,
        public IJniSipControllerServiceThread
{
public:
    JniSipControllerServiceThread();
    virtual ~JniSipControllerServiceThread();

    virtual void OnMessageReceived() override;
    virtual void OnMessageSent() override;
    virtual void OnMessageSendFailure() override;
    virtual void OnRegistrationUpdated(IN IMS_UINTP nParam) override;
    virtual void OnConfigurationUpdated() override;
};
#endif  // JNI_SIP_CONTROLLER_SERVICE_THREAD_H_
