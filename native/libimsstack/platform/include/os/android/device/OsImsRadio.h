
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
#ifndef OS_IMS_RADIO_H_
#define OS_IMS_RADIO_H_

#include "ImsList.h"
#include "ImsMap.h"
#include "ImsRadio.h"

class IThread;

class OsImsRadio : public ImsRadio
{
public:
    OsImsRadio(IN IMS_SINT32 nSlotId);
    virtual ~OsImsRadio();

    OsImsRadio(IN const OsImsRadio&) = delete;
    OsImsRadio& operator=(IN const OsImsRadio&) = delete;

public:
    // IImsRadio class
    IMS_BOOL IsImsTrafficAllowed(IN IMS_UINT32 nTrafficType) override;

    void StartImsTraffic(IN IMS_UINT32 nTrafficType, IN IMS_UINT32 nAccessNetworkType,
            IN IMS_UINT32 nDirection, IN IImsRadioConnectionListener* piListener) override;
    void StopImsTraffic(IN IImsRadioConnectionListener* piListener) override;

    void TriggerEpsFallback(IN IMS_UINT32 nEpsfbReason) override;

    void AddListenerForTrafficPriority(IN IImsRadioTrafficPriorityListener* piListener) override;
    void RemoveListenerForTrafficPriority(IN IImsRadioTrafficPriorityListener* piListener) override;

protected:
    void DispatchServiceMessage(IN IMS_UINTP nWparam, IN IMS_UINTP nLparam) override;

private:
    IMS_UINT32 GetId();
    void NotifyConnectionSetupPrepared(IN IMS_UINT32 nId);
    static const IMS_CHAR* EventToString(IN IMS_UINT32 nEvent);

private:
    IThread* m_piOwnerThread;
    ImsMap<IMS_UINT32, IImsRadioConnectionListener*> m_objConnectionListeners;
    ImsList<IImsRadioTrafficPriorityListener*> m_objTrafficPriorityListeners;

    static const IMS_UINT32 ID_MAX = 0xFFFFFFFF;
};

#endif
