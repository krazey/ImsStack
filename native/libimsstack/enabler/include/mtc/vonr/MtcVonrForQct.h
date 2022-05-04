/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20200924  dongo.yi@                 Created
    </table>

    Description

*/

#ifndef UC_VONR_FOR_QCT_H_
#define UC_VONR_FOR_QCT_H_

#include "vonr/MtcVonr.h"

class UCVoNRForQct : public MtcVonr
{
public:
    explicit UCVoNRForQct(IN IMS_UINT32 nSlotId, IN IMtcVonrListener* piListener);
    virtual ~UCVoNRForQct();

public:
    // for MtcVonr child classes
    void CheckBarring(
            IN IMtcCall* piMtcCall, IN CallType eCallType, IN IMS_BOOL bEmergency) override;

protected:
    virtual void OnSessionStopped(IN IMS_UINTP nParam);
    virtual void NotifyCallState(IN IMS_UINT32 nState);
    virtual void OnNotifyUacResponse(IN IMS_UINT32 nType, IN IMS_RESULT nResult,
            IN IMS_SINT32 nReason, IN IMS_UINT32 nSysMode, IN IMS_UINT32 nBarringTime);
    virtual void OnNotifyCallPreferenceReady(IN IMS_UINT32 nSysMode);
    virtual IMS_BOOL IsUacCheckRequired();

private:
    IMS_UINT32 GetConvertedCallState(IN IMS_UINT32 nState, IN IMS_UINT32 nDirection);
};
#endif  // UC_VONR_FOR_QCT_H_
