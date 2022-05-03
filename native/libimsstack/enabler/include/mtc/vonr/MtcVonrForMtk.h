/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20200924  dongo.yi@                 Created
    </table>

    Description

*/

#ifndef UC_VONR_FOR_MTK_H_
#define UC_VONR_FOR_MTK_H_

#include "vonr/MtcVonr.h"

class UCVoNRForMtk : public MtcVonr
{
public:
    explicit UCVoNRForMtk(IN IMS_UINT32 nSlotId, IN IMtcVonrListener* piListener);
    virtual ~UCVoNRForMtk();

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
    void NotifyCallStart(
            IN IMS_BOOL bSameType, IN IMS_BOOL bDiffType, IN IMS_BOOL bDiffVoiceDomain);
    void NotifyCallStop(IN IMS_BOOL bSameType, IN IMS_BOOL bDiffType, IN IMS_BOOL bDiffVoiceDomain);
    void SetVoice(IN IMS_UINT32 nState, IN IMS_BOOL bDiffType, IN IMS_BOOL bDiffVoiceDomain);
    void SetImsVoice(IN IMS_UINT32 nState, IN IMS_BOOL bSameType, IN IMS_BOOL bDiffType,
            IN IMS_BOOL bDiffVoiceDomain);
    void SetImsSession(IN IMS_UINT32 nState);
    void SetUacCheck(IN IMS_UINT32 nState);

    void GetOngoingSessionStatus(
            OUT IMS_BOOL& bSameType, OUT IMS_BOOL& bDiffType, OUT IMS_BOOL& bDiffVoiceDomain);

private:
    IMS_BOOL m_bEmergency;
    IMS_UINT32 m_nCurrentSysMode;
    VonrInitType m_eTotalInitiateType;
};
#endif  // UC_VONR_FOR_MTK_H_
