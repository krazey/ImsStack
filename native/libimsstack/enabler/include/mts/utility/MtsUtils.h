#ifndef MTS_UTIL_H_
#define MTS_UTIL_H_

#include "ITimer.h"
#include "MtsService.h"

class MtsUtils final
{
public:
    MtsUtils();
    ~MtsUtils();

    static MtsUtils* GetInstance();
    static const IMS_CHAR* RegTimerToString(IN IMS_UINT32 nType);

    ITimer* StartTimer(IN IMS_UINT32 nDuration, IN ITimerListener* piListener,
            IN AString strLog /* = AString("") */);
    void StopTimer(IN ITimer*& piTimer, IN AString strLog /* = AString("") */);

    IMS_BOOL IsEccNumber(IN const IMS_CHAR* strDstAddr, IMS_SINT32 nSlotId);
    IMS_BOOL IsEpdgConnected(IN MtsService* pMtsService);
    IMS_BOOL IsSupportFeature(IN const IMS_CHAR* pszProperty);

    void SetScbm(IN IMS_BOOL bIsScbmTimerStatus);
    IMS_BOOL GetScbm();

public:
    // error codes for received sms messages.
    enum
    {
        EXPIRED_TIME_SCBM = 300000  // 5min, 300s, 300000ms
    };

public:
    static const IMS_CHAR PROPERTY_RIL_ECCLIST[];
    static const IMS_CHAR PROPERTY_FEATURESET_SMSTO911[];
    static const IMS_CHAR PROPERTY_SCBM_MODE[];

private:
    static MtsUtils* m_pMtsUtils;
    IMS_BOOL m_bIsScbm;
};

#endif
