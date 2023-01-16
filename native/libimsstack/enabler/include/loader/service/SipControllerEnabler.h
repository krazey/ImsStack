/*
 * Copyright (C) 2023 The Android Open Source Project
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
#ifndef SIPCONTROLLER_ENABLER_H_
#define SIPCONTROLLER_ENABLER_H_

#include "Enabler.h"

class ImsApp;

class SipControllerEnabler : public Enabler
{
public:
    explicit SipControllerEnabler(IN IMS_SINT32 nSlotId);
    virtual ~SipControllerEnabler();

    SipControllerEnabler(IN const SipControllerEnabler&) = delete;
    SipControllerEnabler& operator=(IN const SipControllerEnabler&) = delete;

public:
    // IEnabler class
    virtual void Start() override;
    virtual void Stop() override;
};
#endif  // SIPCONTROLLER_ENABLER_H_
