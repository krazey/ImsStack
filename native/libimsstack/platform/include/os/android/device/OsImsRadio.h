
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

#include "IImsTraffic.h"
#include "ImsList.h"
#include "ImsMap.h"
#include "ImsRadio.h"

class IThread;

class ConnectionListener
{
public:
    ConnectionListener(IN IMS_UINT32 nId, IN IMS_UINT32 nTrafficType,
            IN IImsRadioConnectionListener* piListener) :
            m_nId(nId),
            m_nTrafficType(nTrafficType),
            m_piListener(piListener)
    {
    }

    virtual ~ConnectionListener() {}

    inline IImsRadioConnectionListener* GetListener() const { return m_piListener; }
    inline IMS_UINT32 GetId() const { return m_nId; }
    inline IMS_UINT32 GetTrafficType() const { return m_nTrafficType; }

private:
    IMS_UINT32 m_nId;
    IMS_UINT32 m_nTrafficType;
    IImsRadioConnectionListener* m_piListener;
};

class OsImsRadio : public ImsRadio, public IImsTrafficListener, public ISystemListener
{
public:
    explicit OsImsRadio(IN IMS_SINT32 nSlotId);
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

    const SsacInfo& GetSsacInfo() const override;

    void AddListenerForSsac(IN IImsRadioSsacListener* piListener) override;
    void RemoveListenerForSsac(IN IImsRadioSsacListener* piListener) override;

    void AddListenerForTrafficPriority(IN IImsRadioTrafficPriorityListener* piListener) override;
    void RemoveListenerForTrafficPriority(IN IImsRadioTrafficPriorityListener* piListener) override;

protected:
    void DispatchServiceMessage(IN IMS_UINTP nWparam, IN IMS_UINTP nLparam) override;
    void ImsTraffic_OnPriorityChanged() override;
    void System_NotifyEvent(
            IN IMS_UINT32 nEvent, IN IMS_UINTP nWParam, IN IMS_UINTP nLParam) override;

private:
    IImsRadioConnectionListener* GetConnectionListener(IN IMS_UINT32 nId);
    IMS_UINT32 GetId();
    void NotifyConnectionFailed(IN IMS_UINT32 nId, IN IMS_UINT32 nFailureReason,
            IN IMS_UINT32 nCauseCode, IN IMS_UINT32 nWaitTimeMillis);
    void NotifyConnectionSetupPrepared(IN IMS_UINT32 nId);
    void NotifySsacInfoChanged(IN IMS_SINT32 nFactorForVoice, IN IMS_SINT32 nTimeSecForVoice,
            IN IMS_SINT32 nFactorForVideo, IN IMS_SINT32 nTimeSecForVideo);
    void NotifySimultaneousCallingSupportChanged(IN IMS_BOOL bSupported);
    static const IMS_CHAR* EventToString(IN IMS_UINT32 nEvent);

private:
    IMS_UINT32 m_nId;
    IThread* m_piOwnerThread;
    SsacInfo m_objSsacInfo;
    ImsMap<IMS_UINT32, IMS_UINT32> m_objTrafficStartedCount;
    ImsList<ConnectionListener*> m_objConnectionListeners;
    ImsList<IImsRadioSsacListener*> m_objSsacListeners;
    ImsList<IImsRadioTrafficPriorityListener*> m_objTrafficPriorityListeners;

    static const IMS_UINT32 ID_MAX = 100000;
};

#endif
