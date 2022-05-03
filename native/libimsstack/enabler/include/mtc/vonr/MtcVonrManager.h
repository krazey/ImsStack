#include "vonr/IMtcVonrListener.h"
#include "vonr/MtcVonr.h"

#ifndef MTC_VONR_MANAGER_H_
#define MTC_VONR_MANAGER_H_

class MtcVonrManager : IMtcVonrListener
{
public:
    explicit MtcVonrManager();
    virtual ~MtcVonrManager();
    MtcVonrManager(IN const MtcVonrManager&) = delete;
    MtcVonrManager& operator=(IN const MtcVonrManager&) = delete;

    // to check FEATURE_VONR
    static IMS_BOOL IsSupported();

    // to check call type / VoNR mode by runtime.
    IMS_BOOL IsUacRequired(IN IMS_BOOL bWifi);
    void CheckBarring(IN IMtcCall* piMtcCall, IN CallType eCallType, IN IMS_BOOL bEmergency);

    // IMtcVonrListener implementation
    virtual void OnTerminated(IN MtcVonr* pMtcVonr);
    virtual IMS_BOOL IsOtherSessionAlive(IN MtcVonr* pMtcVonr, IN IMS_UINT32 nUacType);
    virtual void SetInitiateType(IN MtcVonr::VonrInitType eType);
    virtual MtcVonr::VonrInitType GetTotalInitiateType();

private:
    void Initialize();
    MtcVonr* CreateMtcVonr();
    void DestroyMtcVonr(IN MtcVonr* pMtcVonr);

private:
    IMSList<MtcVonr*> m_lstMtcVonrs;
    MtcVonr::VonrInitType m_eTotalInitiateType;
    IMS_BOOL m_bMtk;
};

#endif  // MTC_VONR_MANAGER_H_
