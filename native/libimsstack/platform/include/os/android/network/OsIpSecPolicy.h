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
#ifndef OS_IPSEC_POLICY_H_
#define OS_IPSEC_POLICY_H_

#include "IIpSecPolicy.h"
#include "ITimer.h"
#include "ImsList.h"

class IIpSecSp;
class IIpSecSa;
class OsIpSecSp;
class OsIpSecSa;
class IIPSecPolicyListener;

class OsIpSecPolicy : public IIpSecPolicy, public ITimerListener
{
public:
    explicit OsIpSecPolicy(IN IMS_SINT32 nId);
    virtual ~OsIpSecPolicy();

    // IIpSecPolicy Interface
    IMS_SINT32 GetId() const override;
    IIpSecSp* CreateSp() override;
    void DestroySp(IN IIpSecSp* piSp) override;
    IIpSecSa* CreateSa() override;
    void DestroySa(IN IIpSecSa* piSa) override;
    void ManageLifetime(IN IMS_UINT32 nDuration) override;
    void SetListener(IN IIpSecPolicyListener* piListener) override;

    void DestroyAllSas();

    OsIpSecSp* FindSp(IN IMS_UINT32 nSpi);
    OsIpSecSa* FindSa(IN IMS_UINT32 nSpi);
    const IMSList<OsIpSecSp*>& GetSPs() const;
    const IMSList<OsIpSecSa*>& GetSAs() const;

private:
    // ITimerListener class
    void Timer_TimerExpired(IN ITimer* piTimer) override;

    void StopTimer();

private:
    // An identifier of IPSec SA parameter
    IMS_SINT32 m_nId;

    IMSList<OsIpSecSp*> m_objIpSecSps;
    IMSList<OsIpSecSa*> m_objIpSecSas;

    IIpSecPolicyListener* m_piListener;
    ITimer* m_piTimer;
};

#endif
