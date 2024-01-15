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
#ifndef AOS_SERVICE_AVAILABLE_H_
#define AOS_SERVICE_AVAILABLE_H_

#include "ImsList.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosBlock.h"
#include "interface/IAosBlockListener.h"
#include "interface/IAosCallTracker.h"
#include "interface/IAosConnection.h"
#include "interface/IAosRegistration.h"

class IAosServiceAvailableListener;

class AosServiceAvailable
{
protected:
    explicit AosServiceAvailable(const AString& strName);
    virtual ~AosServiceAvailable();

public:
    void Init(IN IAosAppContext* piAosAppContext);
    void CleanUp();
    void SetListener(IN IAosServiceAvailableListener* piListener);
    void RemoveListener(IN IAosServiceAvailableListener* piListener);
    void RefreshServiceAvailablility();
    IMS_BOOL IsAvailable();
    IMS_UINT32 HandleEvent(IN IMS_UINT32 eEvent, IN IMS_UINT32 nState, IN IMS_SINT32 nStateEx);

    inline virtual IMS_BOOL StartToCheckNetworkConnection() { return IMS_FALSE; };
    virtual IMS_BOOL StopToCheckNetworkConnection(IN IMS_BOOL bNeedToCheckAvailable = IMS_TRUE);

    void SetBlock(IN IAosBlock* piBlock);
    IMS_BOOL IsRoaming();

protected:
    inline virtual void RegisterListener(){};
    inline virtual void DeregisterListener(){};

    virtual void HandleCallStateChanged(IN IMS_UINT32 nState, IN IMS_SINT32 nStateEx);
    virtual void HandleNetworkStateChanged();
    virtual void HandleBlockChanged(IN IMS_UINT32 nState, IN IMS_UINT32 nStateEx);
    virtual void HandleRoamingChanged(IN IMS_UINT32 nState);
    virtual void HandleAirplaneModeChanged(IN IMS_UINT32 nState);
    virtual void HandleVopsChanged(IN IMS_UINT32 nState);
    virtual void HandleWifiConnectionChanged();
    virtual void HandleLocationInfoChanged();
    virtual IMS_BOOL CheckServiceAvailable();

    void Notify();
    void RequestCommand(IN IMS_UINT32 nCommand, IN IMS_UINT32 nReason);

    IMS_BOOL IsSameAsBeforeUnavailableReason();

    static const IMS_CHAR* EventToString(IN IMS_UINT32 eEvent);

public:
    enum
    {
        EVENT_AIRPLANE = 0,
        EVENT_ROAMING,
        EVENT_VOPS,
        EVENT_LOCATION,
        EVENT_CALL,
        EVENT_NETWORK,
        EVENT_WIFI_STATE,
        EVENT_BLOCK,
        EVENT_MAX
    };

protected:
    IAosAppContext* m_piAppContext;
    IMS_SINT32 m_nSlotId;
    IAosBlock* m_piBlock;
    IAosRegistration* m_piRegistration;
    IAosConnection* m_piConnection;
    IAosCallTracker* m_piCallTracker;
    AString m_strTag;
    AString m_strName;

    IMS_BOOL m_bAirplaneMode;
    IMS_BOOL m_bRoamingState;

private:
    IMS_BOOL m_bAvailableLastNotified;

    ImsList<IMS_UINT32> m_objBlockReasonsLastNotified;
    ImsList<IAosServiceAvailableListener*> m_objListeners;

private:
    friend class AosServiceAvailableTest;
    friend class AosConditionTest;
};

#endif  // AOS_SERVICE_AVAILABLE_H_