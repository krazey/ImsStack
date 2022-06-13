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

#include "IMSTypeDef.h"
#include "message/MtsMessageController.h"

class MtsServiceState final
{
public:
    MtsServiceState(IN IMS_SINT32 nSlotId);
    ~MtsServiceState();

    static MtsServiceState* GetInstance(IN IMS_SINT32 nSlotId);
    void SetImsRegConnected(IN IMS_BOOL bConnected);
    void SetImsRegConnected(IN IMS_BOOL bConnected, IMS_BOOL bIsEmergencyType);
    void SetImsSuspendState(IN IMS_BOOL bState);
    void SetSmsOverIpState(IN IMS_BOOL bState);
    void SetTemporaryServiceBlocked(IN IMS_BOOL bBlocked);
    void SetMtsMessageController(IN MtsMessageController* pMtsMessageController);
    void SetMtsServiceState(IN IMS_SINT32 nServiceState);
    void SetConnectedServices(IN IMS_UINT32 nServices);

    IMS_BOOL GetImsRegState() const;
    IMS_BOOL GetImsRegMod() const;
    IMS_BOOL GetImsSuspendState() const;
    IMS_BOOL GetSmsOverIpState() const;
    IMS_SINT32 GetMtsServiceState() const;
    IMS_UINT32 GetConnectedServices() const;
    IMS_BOOL IsServiceConnected(IN IMS_UINT32 nService);

    void OnImsConnected();
    void OnImsDisconnected(IN IMS_UINT32 nReason);
    void OnImsDisconnecting(IN IMS_UINT32 nReason);
    void OnImsSuspended(IN IMS_UINT32 nReason);
    void OnImsResumed();
    void NotifySpecificMessage(IN IMS_UINT32 nMsg, IN IMS_UINT32 nWparam, IN IMS_UINT32 nLparam);

    IMS_SINT32 GetServiceState();
    void UpdateServiceState();
    IMS_BOOL IsMoServiceBlocked();
    IMS_BOOL IsMtServiceBlocked();
    IMS_BOOL IsTemporaryServiceBlocked();
    IMS_SINT32 GetSlotId() const;

protected:
    IMS_SINT32 m_nMtsServiceState;

    // Check Condition for SMS SERVICE MODE
    IMS_BOOL m_bIsImsConnected;    // if Connected true enable sms mo/mt service.
    IMS_BOOL m_bIsAosRegModAdmin;  // if Mod Admin true. block mo service.
    IMS_BOOL m_bIsImsSuspend;      // if IMSAoSApp_IMSSuspended true. block mo service
    // if sms_over_ip_network Ind is false. block mo service
    IMS_BOOL m_bIsSmsOverIpConf;
    IMS_BOOL m_bIsTemporaryBlocked;
    IMS_UINT32 m_nConnectedServices;
    IMS_SINT32 m_nSlotId;
    MtsMessageController* m_pMtsMessageController;
};

#endif
