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
#ifndef AOS_PCSCF_H_
#define AOS_PCSCF_H_

#include "ITimer.h"
#include "IpAddress.h"
#include "interface/IAosPcscf.h"
#include "IEventListener.h"
#include "provider/AosStaticProfile.h"
#include "provider/AosUtil.h"

class ISubscriberConfig;
class IAosAppContext;
class IAosPcscfListener;

class Pcscf : public ITimerListener
{
public:
    Pcscf(IN const AString& strAddress, IN IMS_SINT32 nPort) :
            m_strAddress(strAddress),
            m_nPort(nPort),
            m_bIsAvailable(IMS_TRUE),
            m_bIsTried(IMS_FALSE),
            m_nTriedCount(0),
            m_piTimer(IMS_NULL)
    {
    }

    virtual ~Pcscf() { StopTimer(); }

    inline const AString& GetAddress() { return m_strAddress; }
    inline IMS_SINT32 GetPort() { return m_nPort; }
    inline IMS_UINT32 GetTriedCount() { return m_nTriedCount; }

    inline void IncreaseTriedCount() { m_nTriedCount++; }
    inline IMS_BOOL IsAvailable() { return m_bIsAvailable; }

    inline IMS_BOOL IsEqual(IN const AString& strCurr)
    {
        IpAddress objIpaCurr(strCurr);
        IpAddress objIpa(m_strAddress);
        return objIpaCurr.Equals(objIpa);
    }

    inline IMS_BOOL IsTried() { return m_bIsTried; }

    inline void ResetTriedCount() { m_nTriedCount = 0; }

    inline void SetAddress(IN const AString& strAddress) { m_strAddress = strAddress; }

    inline void SetAvailable(IN IMS_BOOL bAvailable)
    {
        if (bAvailable)
        {
            if (m_piTimer != IMS_NULL)
            {
                StopTimer();
            }
        }

        m_bIsAvailable = bAvailable;
    }

    inline void SetTry(IN IMS_BOOL bTry) { m_bIsTried = bTry; }

    inline void SetUnavailableWithDuration(IN IMS_UINT32 nSeconds)
    {
        m_bIsAvailable = IMS_FALSE;
        if (nSeconds > 0)
        {
            StartTimer(nSeconds * 1000);
        }
    }

    inline void SetPort(IN IMS_SINT32 nPort) { m_nPort = nPort; }

    inline void StartTimer(IN IMS_UINT32 nDuration)
    {
        if (m_piTimer != IMS_NULL)
        {
            StopTimer();
        }

        m_piTimer = AosUtil::GetInstance()->StartTimer(nDuration, this, "PCSCF_BLOCK_TIMER");
    }

    inline void StopTimer()
    {
        if (m_piTimer == IMS_NULL)
        {
            return;
        }

        AosUtil::GetInstance()->StopTimer(m_piTimer, "PCSCF_BLOCK_TIMER");
    }

    inline void Timer_TimerExpired(IN ITimer* piTimer) override
    {
        if (piTimer == IMS_NULL)
        {
            return;
        }

        if (m_piTimer == piTimer)
        {
            StopTimer();
            m_bIsAvailable = IMS_TRUE;
        }
    }

private:
    AString m_strAddress;
    IMS_SINT32 m_nPort;
    IMS_BOOL m_bIsAvailable;
    IMS_BOOL m_bIsTried;
    IMS_UINT32 m_nTriedCount;
    ITimer* m_piTimer;
};

class RetryHost
{
public:
    RetryHost(IN const AString& strHost, IN IMS_SINT32 nPort, IN IMS_SINT32 nIpVersion) :
            m_strHost(strHost),
            m_nPort(nPort),
            m_nIpVersion(nIpVersion)
    {
    }

    virtual ~RetryHost() {}

    inline const AString& GetHost() { return m_strHost; }

    inline IMS_SINT32 GetPort() { return m_nPort; }

    inline IMS_SINT32 GetIpVersion() { return m_nIpVersion; }

private:
    AString m_strHost;
    IMS_SINT32 m_nPort;
    IMS_SINT32 m_nIpVersion;
};

class AosPcscf : public IAosPcscf, public ITimerListener
{
public:
    explicit AosPcscf(IN IAosAppContext* piAppContext);
    virtual ~AosPcscf();

    void Init() override;
    void CleanUp() override;

    void Configure(IN IMS_UINT32 nIpVersion = IpAddress::UNKNOWN) override;
    IMS_BOOL IsConfigured() const override;

    IMS_BOOL IsAsyncDnsDiscovery() const override;
    IMS_BOOL IsSinglePcoScheme() override;

    const AStringArray& GetPcscfs() override;
    const ImsList<IMS_SINT32>& GetPcscfsPorts() override;
    void UpdatePcscfs(IN const AStringArray& objPcscfs,
            IN ImsList<IMS_SINT32> objPorts = ImsList<IMS_SINT32>()) override;

    IMS_BOOL HasPcscf(IN IMS_SINT32 nIndex) override;
    IMS_UINT32 GetPcscfCount() override;

    void SetCurrentPcscfInvalid(
            IN IMS_BOOL bIsTimer = IMS_FALSE, IN IMS_UINT32 nSeconds = 0) override;
    void RemoveCurrentPcscf() override;
    void SetAllPcscfValid() override;

    IMS_BOOL IsAllPcscfTried() override;
    void SetCurrentPcscfTried() override;
    void ResetAllPcscfTried() override;

    IMS_UINT32 GetCurrentPcscfTriedCount() override;
    void IncreaseCurrentPcscfTriedCount() override;
    void ResetCurrentPcscfTriedCount() override;
    void ResetAllPcscfTriedCount() override;

    IMS_BOOL GetCurrentPcscf(OUT AString& objPcscf, OUT IMS_UINT32& nPort) override;
    IMS_UINT32 GetCurrentIndex() const override;

    IMS_BOOL IsFirstPcscf() override;
    IMS_BOOL GetFirstPcscf(OUT AString& objPcscf, OUT IMS_UINT32& nPort) override;

    IMS_BOOL HasNextPcscf() override;
    IMS_SINT32 GetNextPcscfIndex() override;
    IMS_BOOL GetNextPcscf(OUT AString& objPcscf, OUT IMS_UINT32& nPort) override;

    void SetFirstPcscfIndex() override;

    IMS_BOOL CheckAndProcessChangeFromPco() override;
    IMS_UINT32 GetChangedType() override;

    void SetListener(IN IAosPcscfListener* piListener) override;

    enum
    {
        TIMER_DNS_QUERY_RETRY = 0
    };

protected:
    void AddPcscf(IN const AString& strHost, IN IMS_SINT32 nPort);

    IMS_BOOL GetChangedPcscfs(OUT AStringArray& objPcscfs, IN IMS_SINT32 nIpVersion);

    IMS_SINT32 GetLocalIpVersion();

    IMS_BOOL IsLocalAddressValid(IN IMS_SINT32 nIpVersion);
    IMS_BOOL IsLocalAddressTypeValid(IN IMS_SINT32 nIpVersion) const;
    IMS_BOOL IsSamePcscf(IN const IpAddress& objPcscfAddress, IN IMS_SINT32 nPort);

    void SetConfigured(IN IMS_BOOL bConfigured);

    virtual IMS_BOOL IsFakeDiscoverySchemes() const;
    virtual void ProcessDiscovery(IN IMS_SINT32 nIpVersion);
    virtual IMS_BOOL GetNextDiscoveryMethod(OUT IMS_SINT32& nMethod);

    virtual IMS_BOOL GetFromPco(IN IMS_SINT32 nIpVersion);
    virtual IMS_BOOL GetFromConf(IN IMS_SINT32 nIpVersion);
    virtual IMS_BOOL ProcessDnsQuery(
            IN const AString& strHost, IN IMS_SINT32 nPort, IN IMS_SINT32 nIpVersion);

    virtual const ISubscriberConfig* GetSubscriberConfig(IN IMS_SINT32 nType = -1);
    virtual IMS_SINT32 GetDefaultPcscfPort();

    virtual void CleanAll();
    virtual void ClearPcscfList();
    virtual void ClearRetryHostList();
    virtual void ClearDiscoveryContents();

    // Timer
    virtual void ProcessDnsRetryTimerExpired();
    virtual void StartTimer(IN IMS_UINT32 nType, IN IMS_UINT32 nDuration);
    virtual void StopTimer(IN IMS_UINT32 nType);
    virtual void ClearTimers();

    // ITimerListener
    void Timer_TimerExpired(IN ITimer* piTimer) override;

    // Log
    void PrintPcscfs();

    static const IMS_CHAR* TimerToString(IN IMS_UINT32 nType);

private:
    static const IMS_SINT32 DNS_QEUERY_RETRY_WAITING_TIME_MILLS = 4000;
    IMS_SINT32 GetPcscfPort();
    void ProcessReorder(IN const AString& strCurrentPcscf, IN const AStringArray& objNewPcscfs);
    void UpdatePcscfs(IN const AStringArray& objPcscfs, IN IMS_SINT32 nPort);
    IMS_BOOL IsRegRetryCountOnSinglePcscfConfigured();
    IMS_BOOL IsRegRetryCountPerPcscfConfigured();

protected:
    IAosAppContext* m_piAppContext;
    IAosPcscfListener* m_piListener;
    ITimer* m_piDnsQueryRetryTimer;

    AosRegistrationType m_eRegType;
    IMS_SINT32 m_nSlotId;
    IMS_UINT32 m_nChangedType;

    IMS_BOOL m_bIsConfigured;
    IMS_BOOL m_bIsDnsQueryRetry;
    IMS_BOOL m_bOtherIpTypeRequired;
    IMS_UINT32 m_nCurrentPcscfIndex;
    IMS_UINT32 m_nDiscoveryMethodIndex;

    AStringArray m_objCurrAddresses;
    ImsList<IMS_SINT32> m_objCurrPorts;

    ImsList<Pcscf*> m_objPcscfList;
    ImsList<RetryHost*> m_objRetryHostList;

    AString m_strTag;
};
#endif  // AOS_PCSCF_H_
