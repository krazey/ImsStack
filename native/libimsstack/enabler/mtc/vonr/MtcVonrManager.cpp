/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20200604  dongo.yi@                 Created
    </table>

    Description

*/

#include "ServiceTrace.h"
#include "ServiceUtil.h"

#include "IuMtcService.h"
#include "vonr/MtcVonr.h"
#include "vonr/MtcVonrForMtk.h"
#include "vonr/MtcVonrForQct.h"
#include "vonr/MtcVonrManager.h"


__IMS_TRACE_TAG_COM_UC__;


PUBLIC
// TODO, MTC BUILD
MtcVonrManager::MtcVonrManager()
// MtcVonrManager::MtcVonrManager(IN IMS_UINT32 nSlotId)
//     : m_nSlotId(nSlotId)
//     , m_objUCVoNRs(IMSList<MtcVonr*>())
    : m_eTotalInitiateType(MtcVonr::VonrInitType::NONE)
    , m_bMtk(IMS_FALSE)
{
    //---------------------------------------------------------------------------------------------
    IMS_TRACE_I("+MtcVonrManager", 0, 0, 0);
    Initialize();
}

PUBLIC VIRTUAL
MtcVonrManager::~MtcVonrManager()
{
    //---------------------------------------------------------------------------------------------
    IMS_TRACE_I("~MtcVonrManager : [%" PFLS_x "]", this, 0, 0);

    // TODO, MTC BUILD
    // for (IMS_UINT32 i = 0; i < m_objUCVoNRs.GetSize(); i ++)
    // {
    //     delete m_objUCVoNRs.GetAt(i);
    // }
    // m_objUCVoNRs.Clear();
}

PUBLIC GLOBAL
IMS_BOOL MtcVonrManager::IsSupported()
{
    //---------------------------------------------------------------------------------------------
    // TODO: use carrier-config
    IMS_BOOL bSupported = IMS_FALSE; /*UtilService::GetUtilService()->GetFeatureUtil()
            ->IsFeatureSupported(IFeatureUtil::FEATURE_VONR);*/

#ifdef _VONR_TEST_
    bSupported = IMS_TRUE;
#endif

    IMS_TRACE_D("IsSupported [%s]", _TRACE_B_(bSupported), 0, 0);
    return bSupported;
}

#if _PUBLIC_METHOD_
#endif

PUBLIC
IMS_BOOL MtcVonrManager::IsUacRequired(IN IMS_BOOL bWifi)
{
    //---------------------------------------------------------------------------------------------
    // do not check in wifi call for QCT
    if (!m_bMtk && bWifi)
    {
        return IMS_FALSE;
    }

#ifdef _VONR_TEST_
    return IMS_TRUE;
#else
    // TODO, MTC BUILD
    return IMS_TRUE;
    // return VoNrService::GetVoNrService()->GetVoNr(m_nSlotId)->IsVoNrSupported();

#endif
}

PUBLIC
void MtcVonrManager::CheckBarring(IN IMtcCall* piMtcCall, IN CallType eCallType,
        IN IMS_BOOL bEmergency)
{
    //---------------------------------------------------------------------------------------------
    IMS_TRACE_D("CheckBarring", 0, 0, 0);

    MtcVonr* pVoNR = CreateMtcVonr();
    pVoNR->CheckBarring(piMtcCall, eCallType, bEmergency);
}

PUBLIC VIRTUAL
void MtcVonrManager::OnTerminated(IN MtcVonr* pUCVoNR)
{
    //---------------------------------------------------------------------------------------------
    IMS_TRACE_D("OnTerminated", 0, 0, 0);
    DestroyMtcVonr(pUCVoNR);

    // TODO, MTC BUILD
    // if (m_objUCVoNRs.GetSize() == 0)
    // {
    //     IMS_TRACE_D("OnTerminated :: reset total init type", 0, 0, 0);
    //     m_eTotalInitiateType = MtcVonr::VonrInitType::NONE;
    // }
}

PUBLIC VIRTUAL
IMS_BOOL MtcVonrManager::IsOtherSessionAlive(IN MtcVonr* pUCVoNR, IN IMS_UINT32 nUacType)
{
    //---------------------------------------------------------------------------------------------

    IMS_BOOL bResult = IMS_FALSE;
    // TODO, MTC BUILD
    UNUSED_PARAM(pUCVoNR);
    UNUSED_PARAM(nUacType);
    // IMS_UINT32 nSize = m_objUCVoNRs.GetSize();
    // for (IMS_UINT32 i = 0; i < nSize; i ++)
    // {
    //     MtcVonr* pTemp = m_objUCVoNRs.GetAt(i);
    //     if (pTemp == pUCVoNR)
    //     {
    //         continue;
    //     }
    //     // get session type.
    //     if (pTemp->GetUacType() == nUacType)
    //     {
    //         bResult = IMS_TRUE;
    //         break;
    //     }
    // }

    // IMS_TRACE_D("IsOtherSessionAlive :: [size=%d][is? %s]", nSize, _TRACE_B_(bResult), 0);
    return bResult;
}

PUBLIC VIRTUAL
void MtcVonrManager::SetInitiateType(IN MtcVonr::VonrInitType eType)
{
    //---------------------------------------------------------------------------------------------
    if (m_eTotalInitiateType != MtcVonr::VonrInitType::NONE)
    {
        return;
    }

    IMS_TRACE_D("SetInitiateType :: total init type is updated[%d]", eType, 0, 0);
    m_eTotalInitiateType = eType;
}

PUBLIC VIRTUAL
MtcVonr::VonrInitType MtcVonrManager::GetTotalInitiateType()
{
    //---------------------------------------------------------------------------------------------
    return m_eTotalInitiateType;
}

#if _PRIVATE_METHOD_
#endif
PRIVATE
void MtcVonrManager::Initialize()
{
    //---------------------------------------------------------------------------------------------
    IMS_TRACE_D("Initialize", 0, 0, 0);
    AString strChipset = UtilService::GetUtilService()->GetSystemProperty()->GetChipsetVendor();
    if (strChipset.EqualsIgnoreCase("MediaTek"))
    {
        m_bMtk = IMS_TRUE;
    }
}

PRIVATE
MtcVonr* MtcVonrManager::CreateMtcVonr()
{
    //---------------------------------------------------------------------------------------------
    IMS_TRACE_D("CreateUCVoNR", 0, 0, 0);

    MtcVonr* pVoNR = IMS_NULL;
    // TODO, MTC BUILD
    // if (m_bMtk)
    // {
    //     pVoNR = new UCVoNRForMtk(m_nSlotId, this);
    // }
    // else
    // {
    //     pVoNR = new UCVoNRForQct(m_nSlotId, this);
    // }
    // m_objUCVoNRs.Append(pVoNR);
    return pVoNR;
}

PRIVATE
void MtcVonrManager::DestroyMtcVonr(IN MtcVonr* pUCVoNR)
{
    //---------------------------------------------------------------------------------------------
    IMS_TRACE_D("DestroyUCVoNR", 0, 0, 0);

    // TODO, MTC BUILD
    UNUSED_PARAM(pUCVoNR);
    // for (IMS_UINT32 i = 0; i < m_objUCVoNRs.GetSize(); i ++)
    // {
    //     MtcVonr* pTemp = m_objUCVoNRs.GetAt(i);
    //     if (pTemp == pUCVoNR)
    //     {
    //         delete pTemp;
    //         m_objUCVoNRs.RemoveAt(i);
    //         return;
    //     }
    // }
    IMS_TRACE_E(0, "DestroyUCVoNR Failed", 0, 0, 0);
}
