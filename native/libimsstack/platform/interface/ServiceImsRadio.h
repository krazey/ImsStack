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
#ifndef SERVICE_IMS_RADIO_H_
#define SERVICE_IMS_RADIO_H_

#include "IImsRadio.h"
#include "IImsTraffic.h"
#include "ImsMessage.h"
#include "PlatformService.h"

class ImsRadioServicePrivate;

class ImsRadioService : public PlatformService
{
public:
    ImsRadioService();
    ImsRadioService(IN const ImsRadioService&) = delete;
    ImsRadioService& operator=(IN const ImsRadioService&) = delete;

protected:
    virtual ~ImsRadioService();

public:
    virtual IImsRadio* GetImsRadio(IN IMS_SINT32 nSlotId);
    virtual IImsTraffic* GetImsTraffic();

    void DispatchServiceMessage(IN ImsMessage& objMsg);
    static ImsRadioService* GetImsRadioService();

private:
    ImsRadioServicePrivate* m_pPrivate;
};

#endif
