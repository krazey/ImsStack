#ifndef INTERFACE_MTC_VONR_LISTENER_H_
#define INTERFACE_MTC_VONR_LISTENER_H_

#include "vonr/MtcVonr.h"

class IMtcVonrListener
{
public:
    virtual void OnTerminated(IN MtcVonr* pMtcVonr) = 0;
    virtual IMS_BOOL IsOtherSessionAlive(IN MtcVonr* pMtcVonr, IN IMS_UINT32 nUacType) = 0;
    virtual void SetInitiateType(IN MtcVonr::VonrInitType eType) = 0;
    virtual MtcVonr::VonrInitType GetTotalInitiateType() = 0;
};

#endif  // INTERFACE_MTC_VONR_LISTENER_H_
