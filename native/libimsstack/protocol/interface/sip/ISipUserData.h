#ifndef __ISIP_USER_DATA__H__
#define __ISIP_USER_DATA__H__

#include "sip_pf_datatypes.h"
#include "SipConfiguration.h"
/****************************************************************************
ISipUserData: Class Declaration Starts
 *****************************************************************************/
class ISipUserData
{
private:
    SIP_UINT32 m_nMsgOptions;
    SIP_BOOL m_bDeleteFlag;
    SIP_VOID* m_pvUserData; /* Stack User Specific Data */

public:
    inline ISipUserData() :
            m_nMsgOptions(ESIPMSGOPT_NONE),
            m_bDeleteFlag(SIP_FALSE),
            m_pvUserData(SIP_NULL)
    {
    }

    inline ISipUserData(SIP_VOID* pvUserData) :
            m_nMsgOptions(ESIPMSGOPT_NONE),
            m_bDeleteFlag(SIP_FALSE),
            m_pvUserData(pvUserData)
    {
    }

    inline SIP_VOID* GetUserData() const { return m_pvUserData; }

    inline void SetUserData(SIP_VOID* pvUserData) { m_pvUserData = pvUserData; }

    inline SIP_BOOL GetDeleteFlag() const { return m_bDeleteFlag; }

    inline void SetDeleteFlag(SIP_BOOL bFlag) { m_bDeleteFlag = bFlag; }

    inline SIP_UINT32 GetMsgOptions() const { return m_nMsgOptions; }

    inline void SetMsgOptions(SIP_UINT32 nMsgOptions) { m_nMsgOptions = nMsgOptions; }

    inline virtual ~ISipUserData() {}
};

#endif  // __ISIP_USER_DATA__H__
