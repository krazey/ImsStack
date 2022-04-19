#ifndef _AOS_PCSCF_H_
#define _AOS_PCSCF_H_

#include "ITimer.h"
#include "IPAddress.h"
#include "interface/IAosPcscf.h"
#include "IConfigUpdateListener.h"
#include "IEventListener.h"
#include "provider/AosStaticProfile.h"
#include "provider/AosUtil.h"

class ISubscriberConfig;
class IAosAppContext;
class IAosPcscfListener;
class ITimerListener;

class Pcscf
    : public ITimerListener
{
public:
    Pcscf(IN const AString& strAddress, IN IMS_SINT32 nPort)
        : m_strAddress(strAddress)
        , m_nPort(nPort)
        , m_bIsAvailable(IMS_TRUE)
        , m_piTimer(IMS_NULL)
    {
    }

    virtual ~Pcscf()
    {
        StopTimer();
    }

    inline const AString& GetAddress()
    {
        return m_strAddress;
    }

    inline IMS_SINT32 GetPort()
    {
        return m_nPort;
    }

    inline IMS_BOOL IsAvailable()
    {
        return m_bIsAvailable;
    }

    inline IMS_BOOL IsEqual(IN const AString& strCurr)
    {
        IPAddress objIpaCurr(strCurr);
        IPAddress objIpa(m_strAddress);

        return objIpaCurr.Equals(objIpa);
    }

    inline void SetAddress(IN AString& strAddress)
    {
        m_strAddress = strAddress;
    }

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

    inline void SetUnavailableWithDuration(IN IMS_UINT32 nSeconds)
    {
       m_bIsAvailable = IMS_FALSE;
       if (nSeconds > 0)
       {
           StartTimer(nSeconds * 1000);
       }
    }

    inline void SetPort(IN IMS_SINT32 nPort)
    {
         m_nPort = nPort;
    }

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

    inline virtual void Timer_TimerExpired(IN ITimer* piTimer)
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
    ITimer* m_piTimer;
};

class RetryHost
{
public:
    RetryHost(IN const AString& strHost, IN IMS_SINT32 nPort, IN IMS_SINT32 nIpVersion)
        : m_strHost(strHost)
        , m_nPort(nPort)
        , m_nIpVersion(nIpVersion)
    {
    }

    virtual ~RetryHost()
    {
    }

    inline const AString& GetHost()
    {
        return m_strHost;
    }

    inline IMS_SINT32 GetPort()
    {
        return m_nPort;
    }

    inline IMS_SINT32 GetIpVersion()
    {
        return m_nIpVersion;
    }

private:
    AString m_strHost;
    IMS_SINT32 m_nPort;
    IMS_SINT32 m_nIpVersion;
};

class AosPcscf
    : public IAosPcscf
    , public ITimerListener
{
public:
    AosPcscf(IN IAosAppContext* piAppContext);
    virtual ~AosPcscf();

    virtual void Init();
    virtual void CleanUp();

    virtual void Configure(IN IMS_UINT32 nIpVersion = IPAddress::UNKNOWN);
    virtual IMS_BOOL IsConfigured() const;

    virtual IMS_BOOL IsAsyncDnsDiscovery() const;
    virtual IMS_BOOL IsSinglePcoScheme();

    virtual const AStringArray& GetPcscfs();
    virtual const IMSList<IMS_SINT32>& GetPcscfsPorts();
    virtual void UpdatePcscfs(IN const AStringArray& objPcscfs,
            IN IMSList<IMS_SINT32> objPorts = IMSList<IMS_SINT32>());

    virtual IMS_BOOL HasPcscf(IN IMS_SINT32 nIndex);
    virtual IMS_UINT32 GetPcscfCount();

    virtual void SetCurrentPcscfInvalid(IN IMS_BOOL bIsTimer = IMS_FALSE,
            IN IMS_UINT32 nSeconds = 0);
    virtual void RemoveCurrentPcscf();
    virtual void SetAllPcscfValid();

    virtual IMS_BOOL GetCurrentPcscf(OUT AString& objPcscf, OUT IMS_UINT32& nPort);
    virtual IMS_UINT32 GetCurrentIndex() const;

    virtual IMS_BOOL IsFirstPcscf();
    virtual IMS_BOOL GetFirstPcscf(OUT AString& objPcscf, OUT IMS_UINT32& nPort);

    virtual IMS_BOOL HasNextPcscf();
    virtual IMS_SINT32 GetNextPcscfIndex();
    virtual IMS_BOOL GetNextPcscf(OUT AString& objPcscf, OUT IMS_UINT32& nPort);

    virtual void SetFirstPcscfIndex();

    virtual IMS_BOOL CheckAndProcessChangeFromPco();
    virtual IMS_UINT32 GetChangedType();

    virtual void RequestCmd(IN IMS_UINT32 nType);

    virtual void SetListener(IN IAosPcscfListener* piListener);

protected:
    void AddPcscf(IN const AString& strHost, IN IMS_SINT32 nPort);

    IMS_BOOL GetChangedPcscfs(OUT AStringArray& objPcscfs, IN IMS_SINT32 nIpVersion);

    IMS_SINT32 GetLocalIpVersion();

    IMS_BOOL IsLocalAddressValid(IN IMS_SINT32 nIpVersion);
    IMS_BOOL IsLocalAddressTypeValid(IN IMS_SINT32 nIpVersion) const;
    IMS_BOOL IsSamePcscf(IN const IPAddress& objPcscfAddress, IN IMS_SINT32 nPort);

    void SetConfigured(IN IMS_BOOL bConfigured);

    virtual IMS_BOOL IsFakeDiscoverySchemes() const;
    virtual void ProcessDiscovery(IN IMS_SINT32 nIpVersion);
    virtual IMS_BOOL GetNextDiscoveryMethod(OUT IMS_SINT32& nMethod);

    virtual IMS_BOOL GetFromPco(IN IMS_SINT32 nIpVersion);
    virtual IMS_BOOL GetFromConf(IN IMS_SINT32 nIpVersion);
    virtual IMS_BOOL ProcessDnsQuery(IN const AString& strHost, IN IMS_SINT32 nPort,
            IN IMS_SINT32 nIpVersion);

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

    // IConfigUpdateListener
    virtual void ConfigUpdate_NotifyUpdate(IN IMS_SINT32 nCPI,
            IN const AString& strConfName = AString::ConstNull(),
            IN const AString& strExtraParam = AString::ConstNull());

    // ITimerListener
    virtual void Timer_TimerExpired(IN ITimer* piTimer);

    // Log
    void PrintPcscfs();

    static const IMS_CHAR* TimerToString(IN IMS_UINT32 nType);

    enum
    {
        TIMER_DNS_QUERY_RETRY = 0
    };

private:
    static const IMS_SINT32 DNS_QEUERY_RETRY_WAITING_TIME_MILLS = 4000;
    IMS_SINT32 GetPcscfPort();
    void ProcessReorder(IN AString& strCurrentPcscf, IN AStringArray& objNewPcscfs);
    void UpdatePcscfs(IN const AStringArray& objPcscfs, IN IMS_SINT32 nPort);

protected:
    IAosAppContext* m_piAppContext;
    IAosPcscfListener* m_piListener;
    ITimer* m_piDnsQueryRetryTimer;

    AosRegistrationType m_eRegType;
    IMS_UINT32 m_nChangedType;

    IMS_BOOL m_bIsConfigured;
    IMS_UINT32 m_nCurrentPcscfIndex;
    IMS_UINT32 m_bIsDnsQueryRetry;
    IMS_UINT32 m_bOtherIpTypeRequired;
    IMS_UINT32 m_nDiscoveryMethodIndex;

    AStringArray m_objCurrAddresses;
    IMSList<IMS_SINT32> m_objCurrPorts;

    IMSList<Pcscf*> m_objPcscfList;
    IMSList<RetryHost*> m_objRetryHostList;

    AString m_strTag;
};
#endif // _AOS_PCSCF_H_
