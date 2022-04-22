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
#ifndef OS_SRVCC_H_
#define OS_SRVCC_H_

#include "IEventListener.h"
#include "IMSList.h"
#include "ImsSlot.h"
#include "ISrvcc.h"

class OsSrvcc
    : public IMSSlot
    , public ISRVCC
    , public IEventListener
{
public:
    OsSrvcc(IN IMS_SINT32 nSlotId);
    virtual ~OsSrvcc();

    OsSrvcc(IN const OsSrvcc&) = delete;
    OsSrvcc& operator=(IN const OsSrvcc&) = delete;

public:
    void SubscribeSRVCCListener(IN ISRVCCListener* piListener) override;
    void UnsubscribeSRVCCListener(IN ISRVCCListener* piListener) override;

protected:
    // IEventListener class
    void Event_NotifyEvent(IN IMS_SINT32 nEvent,
            IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam) override;

private:
    void ListenSrvccEvent();
    void UnlistenSrvccEvent();
    void NotifySrvccEvent(IN IMS_UINT32 nEvent);

private:
    IMS_BOOL m_bSrvccEventRegistered;

    IMSList<ISRVCCListener*> m_objSrvccListeners;
};

#endif
