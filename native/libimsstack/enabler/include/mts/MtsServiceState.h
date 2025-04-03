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

#ifndef MTS_SERVICESTATE_H_
#define MTS_SERVICESTATE_H_

#include "ICarrierConfigListener.h"
#include "IMtsServiceState.h"

class ICarrierConfig;
class IImsAos;

class MtsServiceState final : public ICarrierConfigListener, public IMtsServiceState
{
public:
    explicit MtsServiceState(IN IMS_SINT32 nSlotId);
    ~MtsServiceState();

    // ICarrierConfigListener
    void CarrierConfig_NotifyConfigChanged(IN IMS_SINT32 nSlotId) override;

    // IMtsServiceState
    void Init(IN IImsAos* piImsAos) override;
    IMS_SINT32 GetState() const override;
    void OnImsConnected() override;
    void OnImsDisconnected(IN IMS_UINT32 nReason) override;
    void OnImsDisconnecting(IN IMS_UINT32 nReason) override;
    void OnImsSuspended(IN IMS_UINT32 nReason) override;
    void OnImsResumed() override;
    void SetImsRegConnected(IN IMS_BOOL bConnected) override;
    inline IMS_BOOL GetImsRegConnected() override { return m_bImsConnected; }

    IMS_BOOL IsMoServiceBlocked() const override;
    IMS_BOOL IsMtServiceBlocked() const override;

private:
    IMS_BOOL LoadCarrierConfig(IN const ICarrierConfig& objCc);
    void SetImsSuspendState(IN IMS_BOOL bState);
    void Update();

    IImsAos* m_piImsAos;
    IMS_SINT32 m_nState;
    // Check Condition for SMS SERVICE MODE
    IMS_BOOL m_bImsConnected;    // if Connected true enable sms mo/mt service.
    IMS_BOOL m_bAosRegModAdmin;  // if Mod Admin true. block mo service.
    IMS_BOOL m_bImsSuspend;      // if IMSAoSApp_IMSSuspended true. block mo service
    // if sms_over_ip_network Ind is false. block mo service
    IMS_BOOL m_bSmsOverIpConf;
    IMS_BOOL m_bAllowImsiBasedSipUri;
    IMS_SINT32 m_nSlotId;
};

#endif
