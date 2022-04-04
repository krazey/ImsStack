/******************************************************************************
 * Project Name  : SIP_RTP
 * Group    : IP-CS [MSG-2]
 * Security   : Confidential
 *****************************************************************************/

/******************************************************************************
 * Filename    : ISipUserData.h
 * Purpose    : Defines the Message Classes used by Stack User
 * Platform    : Windows OR Android
 * Author(s)    : Syed Malgimani
 * E-mail id.    : syed.malgimani@
 * Creation date   : May. 25,2010
 *
 * Edit History     Modification           Description(s)
 *
 * Date      Name    Version    Bug-ID    Description
 * ----------    ----------    -------    ------    -------------
 * Month. Date,10    Name       0.0a    ---    Initial creation

 *****************************************************************************/
#ifndef  __ISIP_USER_DATA__H__
#define  __ISIP_USER_DATA__H__

/*****************************************************************************
  Header Inclusions
 *****************************************************************************/
#include "sip_pf_datatypes.h"
#include "SipConfiguration.h"
//swmd - vijay.nair@
#if 0
#include "sip_trace.h"
#include "sip_error.h"
#endif
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
    inline ISipUserData()
        : m_nMsgOptions(ESIPMSGOPT_NONE)
        , m_bDeleteFlag(SIP_FALSE)
        , m_pvUserData(SIP_NULL)
    { }

    inline ISipUserData(SIP_VOID* pvUserData)
        : m_nMsgOptions(ESIPMSGOPT_NONE)
        , m_bDeleteFlag(SIP_FALSE)
        , m_pvUserData(pvUserData)
    { }

    inline SIP_VOID* GetUserData() const
    { return m_pvUserData; }

    inline void SetUserData(SIP_VOID* pvUserData)
    { m_pvUserData = pvUserData; }

    inline SIP_BOOL GetDeleteFlag() const
    { return m_bDeleteFlag; }

    inline void SetDeleteFlag(SIP_BOOL bFlag)
    { m_bDeleteFlag = bFlag; }

    inline SIP_UINT32 GetMsgOptions() const
    { return m_nMsgOptions; }

    inline void SetMsgOptions(SIP_UINT32 nMsgOptions)
    { m_nMsgOptions = nMsgOptions; }

    inline virtual ~ISipUserData()
    { }
};

#endif // __ISIP_USER_DATA__H__
