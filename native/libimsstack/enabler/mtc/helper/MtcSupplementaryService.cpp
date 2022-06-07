#include "IMessage.h"
#include "IMtcContext.h"
#include "ISipHeader.h"
#include "IuMtcService.h"
#include "MtcDef.h"
#include "ServiceThread.h"
#include "ServiceTrace.h"
#include "SipAddress.h"
#include "SipHeaderName.h"
#include "SipParameter.h"
#include "SipParsingHelper.h"

#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "define/MtcStringDef.h"
#include "helper/MtcSupplementaryService.h"
#include "ussi/UssiController.h"
#include "utility/MessageUtil.h"

__IMS_TRACE_TAG_COM_MTC__;

PRIVATE GLOBAL const IMS_CHAR MtcSupplementaryService::STR_VERSTAT[] = "verstat";
const IMS_CHAR MtcSupplementaryService::STR_VERSTAT_TN_VALIDATION_PASSED[] = "TN-Validation-Passed";
const IMS_CHAR MtcSupplementaryService::STR_VERSTAT_TN_VALIDATION_FAILED[] = "TN-Validation-Failed";

PUBLIC
MtcSupplementaryService::MtcSupplementaryService(IN MtcConfigurationProxy& objConfigurationProxy,
        IN IMSMap<SuppType, SuppService*> objSuppServices) :
        m_objSuppService(objSuppServices),
        m_objConfigurationProxy(objConfigurationProxy),
        m_nCnapType(CNAP_SCHEME_PAID_FROM)
{
    IMS_TRACE_MEM("mtc", "mtc_M : MtcSupplementaryService[%" PFLS_u "][%" PFLS_x "]",
            sizeof(MtcSupplementaryService), this, 0);
    LoadConfig();
}

PUBLIC
MtcSupplementaryService::~MtcSupplementaryService()
{
    IMS_TRACE_MEM("mtc", "mtc_F : MtcSupplementaryService[%" PFLS_u "][%" PFLS_x "]",
            sizeof(MtcSupplementaryService), this, 0);

    DeleteServices();
}

PUBLIC
void MtcSupplementaryService::UpdateOutgoingServices(
        IN const IMSMap<SuppType, SuppService*>& objSuppServices)
{
    IMS_UINT32 nInServiceSize = objSuppServices.GetSize();
    IMS_TRACE_I("MtcSupplementaryService : ServiceNum[%d] InServiceNum[%d]",
            m_objSuppService.GetSize(), nInServiceSize, 0);

    for (IMS_UINT32 i = 0; i < nInServiceSize; i++)
    {
        const SuppType eType = objSuppServices.GetKeyAt(i);
        IMS_SLONG nIndex = m_objSuppService.GetIndexOfKey(eType);

        if (nIndex >= 0)
        {
            delete m_objSuppService.GetValueAt(nIndex);
        }

        m_objSuppService.Add(eType, objSuppServices.GetValueAt(i));
    }
}

PUBLIC
void MtcSupplementaryService::UpdateTip(IN IMessage* piMessage)
{
    AString strPrivacy;
    MessageUtil::GetHeader(piMessage, ISipHeader::PRIVACY, strPrivacy);
    IMS_BOOL bHasPAssertedIdentity =
            MessageUtil::IsHeaderPresent(piMessage, ISipHeader::P_ASSERTED_IDENTITY);

    IMS_SINT32 tipType;
    AString tipStr;
    if (!bHasPAssertedIdentity && strPrivacy.EqualsIgnoreCase("id"))
    {
        tipType = TIP_TYPE_RESTRICTED;
    }
    else if (!bHasPAssertedIdentity && !(strPrivacy.EqualsIgnoreCase("id")))
    {
        tipType = TIP_TYPE_NONE;
    }
    else
    {
        tipType = TIP_TYPE_IDENTITY;
        AString strNumber;
        AString strName;
        MessageUtil::GetUserPart(piMessage, ISipHeader::P_ASSERTED_IDENTITY, strNumber);
        MessageUtil::GetDisplayName(piMessage, ISipHeader::P_ASSERTED_IDENTITY, strName);
        tipStr.Append(strNumber);
        tipStr.Append(',');
        tipStr.Append(strName);
    }
    Add(SuppType::TIP, tipType);
    Add(SuppType::TIP, tipStr);
}

PUBLIC
IMS_BOOL MtcSupplementaryService::UpdateIncomingServices(IN IMessage* piMessage)
{
    IMS_BOOL bUpdate = IMS_FALSE;

    bUpdate |= UpdateCallerId(piMessage);
    bUpdate |= UpdateCnap(piMessage);
    bUpdate |= UpdateCnapEx(piMessage);
    bUpdate |= UpdateMmc(piMessage);
    bUpdate |= UpdateGtt(piMessage);
    bUpdate |= UpdateCdivCause(piMessage);
    bUpdate |= UpdateCdivHistory(piMessage);
    bUpdate |= UpdateCw(piMessage);
    bUpdate |= UpdateVm(piMessage);
    bUpdate |= UpdateAnswerHold(piMessage);
    bUpdate |= UpdateMcid(piMessage);
    bUpdate |= UpdateDualNumber(piMessage);
    bUpdate |= UpdateCallingNumVerification(piMessage);

    IMS_TRACE_I("UpdateService : [%s]", PS_BOOL(bUpdate), 0, 0);
    return bUpdate;
}

PUBLIC
IMS_BOOL MtcSupplementaryService::UpdateCallerId(IN IMessage* piMessage)
{
    AString strPrivacy;
    OipType eOipType = OipType::INVALID;
    MessageUtil::GetHeader(piMessage, ISipHeader::PRIVACY, strPrivacy);

    if (MessageUtil::GetHeader(piMessage, ISipHeader::PRIVACY, strPrivacy) == IMS_SUCCESS)
    {
        if (strPrivacy.EqualsIgnoreCase("id"))
        {
            eOipType = OipType::RESTRICTED;
        }
        else if (strPrivacy.EqualsIgnoreCase("none"))
        {
            eOipType = OipType::IDENTITY;
        }
    }

    switch (m_nCnapType)
    {
        case CNAP_SCHEME_PAID_FROM:
            eOipType = (MessageUtil::IsHeaderPresent(piMessage, ISipHeader::P_ASSERTED_IDENTITY) ||
                               MessageUtil::IsHeaderPresent(piMessage, ISipHeader::FROM))
                    ? OipType::IDENTITY
                    : OipType::NONE;
            break;
        case CNAP_SCHEME_FROM:
            eOipType = MessageUtil::IsHeaderPresent(piMessage, ISipHeader::FROM) ? OipType::IDENTITY
                                                                                 : OipType::NONE;
            break;
        case CNAP_SCHEME_PAID:
            eOipType = MessageUtil::IsHeaderPresent(piMessage, ISipHeader::P_ASSERTED_IDENTITY)
                    ? OipType::IDENTITY
                    : OipType::NONE;
            break;
        default:
            IMS_TRACE_E(0, "Unhandled CNAP type[%d]", m_nCnapType, 0, 0);
    }

    Add(SuppType::CALLER_ID, static_cast<IMS_SINT32>(eOipType));

    if (eOipType == OipType::INVALID)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL MtcSupplementaryService::UpdateCnap(IN IMessage* piMessage)
{
    AString strDisplayName;

    if (m_nCnapType == CNAP_SCHEME_PAID_FROM || m_nCnapType == CNAP_SCHEME_PAID)
    {
        MessageUtil::GetDisplayName(piMessage, ISipHeader::P_ASSERTED_IDENTITY, strDisplayName);
    }

    if (strDisplayName.GetLength() <= 0 &&
            (m_nCnapType == CNAP_SCHEME_PAID_FROM || m_nCnapType == CNAP_SCHEME_FROM))
    {
        MessageUtil::GetDisplayName(piMessage, ISipHeader::FROM, strDisplayName);
    }

    if (strDisplayName.GetLength() <= 0)
    {
        return IMS_FALSE;
    }

    Add(SuppType::CNAP, strDisplayName);

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL MtcSupplementaryService::UpdateCnapEx(IN IMessage* /*piMessage*/)
{
    return IMS_FALSE;
}

PUBLIC
IMS_BOOL MtcSupplementaryService::UpdateMmc(IN IMessage* /*piMessage*/)
{
    IMS_BOOL bUseMMC = m_objConfigurationProxy.Is(Feature::USE_MMC_SUPPLEMENTARY_SERVICE);

    if (bUseMMC)
    {
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL MtcSupplementaryService::UpdateGtt(IN IMessage* /*piMessage*/)
{
    return IMS_FALSE;
}

PUBLIC
IMS_BOOL MtcSupplementaryService::UpdateCdivCause(IN IMessage* piMessage)
{
    ISipHeader* piHeader = GetHistoryInfoHeader(piMessage);

    if (piHeader == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nCause;

    if (GetCdivCause(piHeader->GetSipAddress(), nCause))
    {
        Add(SuppType::CDIV_CAUSE, ConvertCdivCause(nCause));
    }

    piHeader->Destroy();

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL MtcSupplementaryService::UpdateCdivHistory(IN IMessage* piMessage)
{
    ISipHeader* piHeader = GetHistoryInfoHeader(piMessage);

    if (piHeader == IMS_NULL)
    {
        return IMS_FALSE;
    }

    AString strTarget;

    if (GetCdivTarget(piHeader->GetSipAddress(), strTarget))
    {
        Add(SuppType::CDIV_HISTORY, strTarget);
    }

    piHeader->Destroy();

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL MtcSupplementaryService::UpdateCw(IN IMessage* piMessage)
{
    if (MessageUtil::IsHeaderPresent(piMessage, ISipHeader::UNKNOWN, SipHeaderName::ALERT_INFO) ==
            IMS_FALSE)
    {
        return IMS_FALSE;
    }

    Add(SuppType::CW, IMS_TRUE);

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL MtcSupplementaryService::UpdateVm(IN IMessage* /*piMessage*/)
{
    return IMS_FALSE;
}

PUBLIC
IMS_BOOL MtcSupplementaryService::UpdateAnswerHold(IN IMessage* /*piMessage*/)
{
    return IMS_FALSE;
}

PUBLIC
IMS_BOOL MtcSupplementaryService::UpdateMcid(IN IMessage* /*piMessage*/)
{
    IMS_BOOL bUseMCID = m_objConfigurationProxy.Is(Feature::USE_MCID_SUPPLEMENTARY_SERVICE);

    if (bUseMCID)
    {
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL MtcSupplementaryService::UpdateDualNumber(IN IMessage* /*piMessage*/)
{
    return IMS_FALSE;
}

PUBLIC
IMS_BOOL MtcSupplementaryService::UpdateCallingNumVerification(IN IMessage* piMessage)
{
    // TODO :: need to check stiil using scheme below
    // PRIVATE
    // IMS_BOOL IdleState::IsSupportCallingNumberVerification()
    // {
    //     /* TODO:
    //     IMS_UINT32 nSupported = AoSSupportability::NOT_SUPPORTED;

    //     if (m_objContext.GetService().GetIImsAosApp()->GetDetailedState(
    //             AoSAppRequest::STATE_SUPPORT_CALLING_NUMBER_VERIFICATION, nSupported))
    //     {
    //         if (nSupported == AoSSupportability::SUPPORTED)
    //         {
    //             return IMS_TRUE;
    //         }
    //     }
    //     */
    //     return IMS_FALSE;
    // }

    IMS_SINT32 nHeaderType = GetCnvHeaderType(piMessage);

    AString strValue;
    if (MessageUtil::GetParameterValueFromUri(
                piMessage, STR_VERSTAT, nHeaderType, strValue, AString::ConstNull()) == IMS_FAILURE)
    {
        return IMS_FALSE;
    }

    Add(SuppType::CALLING_NUM_VERIFICATION, GetCallingNumVerificationResult(strValue));

    return IMS_TRUE;
}

PUBLIC
void MtcSupplementaryService::Delete(IN SuppType eType)
{
    IMS_SLONG nIndex = m_objSuppService.GetIndexOfKey(eType);

    if (nIndex >= 0)
    {
        SuppService* pSuppService = m_objSuppService.GetValueAt(nIndex);
        delete pSuppService;
        m_objSuppService.RemoveAt(nIndex);
        IMS_TRACE_I("Delete : size[%d] Type[%d]", m_objSuppService.GetSize(), eType, 0);
        return;
    }

    IMS_TRACE_I("Delete : NoT Matched Size[%d]", m_objSuppService.GetSize(), 0, 0);
}

PUBLIC
void MtcSupplementaryService::DeleteServices()
{
    IMS_UINT32 nSize = m_objSuppService.GetSize();

    IMS_TRACE_I("DeleteAll : Size[%d]", nSize, 0, 0);

    for (IMS_UINT32 index = 0; index < nSize; index++)
    {
        SuppService* pService = m_objSuppService.GetValueAt(index);
        delete pService;
    }

    m_objSuppService.Clear();
}

PUBLIC
const SuppService* MtcSupplementaryService::Get(IN SuppType eType)
{
    IMS_SLONG nIndex = m_objSuppService.GetIndexOfKey(eType);

    if (nIndex >= 0)
    {
        SuppService* pSuppService = m_objSuppService.GetValueAt(nIndex);
        return pSuppService;
    }

    IMS_TRACE_I("Get : NoT Matched, Size[%d]", m_objSuppService.GetSize(), 0, 0);
    return IMS_NULL;
}

PUBLIC
const IMSMap<SuppType, SuppService*>& MtcSupplementaryService::GetServices()
{
    return m_objSuppService;
}

PUBLIC
void MtcSupplementaryService::Add(IN SuppType eSuppType, IN AString strValue)
{
    if (IsExist(eSuppType) == IMS_TRUE)
    {
        SuppService* pExistService = m_objSuppService.GetValue(eSuppType);
        pExistService->strValue = strValue;
    }
    else
    {
        SuppService* pUpdateService = new SuppService();
        pUpdateService->strValue = strValue;
        m_objSuppService.Add(eSuppType, pUpdateService);
    }
}

PUBLIC
void MtcSupplementaryService::Add(IN SuppType eSuppType, IN IMS_SINT32 nValue)
{
    if (IsExist(eSuppType) == IMS_TRUE)
    {
        SuppService* pExistService = m_objSuppService.GetValue(eSuppType);
        pExistService->nValue = nValue;
    }
    else
    {
        SuppService* pUpdateService = new SuppService();
        pUpdateService->nValue = nValue;
        m_objSuppService.Add(eSuppType, pUpdateService);
    }
}

PUBLIC
void MtcSupplementaryService::Add(IN SuppType eSuppType, IN IMS_BOOL bValue)
{
    if (IsExist(eSuppType) == IMS_TRUE)
    {
        SuppService* pExistService = m_objSuppService.GetValue(eSuppType);
        pExistService->bValue = bValue;
    }
    else
    {
        SuppService* pUpdateService = new SuppService();
        pUpdateService->bValue = bValue;
        m_objSuppService.Add(eSuppType, pUpdateService);
    }
}

PRIVATE
ISipHeader* MtcSupplementaryService::GetHistoryInfoHeader(IN IMessage* piMessage)
{
    IMSList<AString> lstHistoryInfos;
    MessageUtil::GetHeaders(piMessage, ISipHeader::HISTORY_INFO, lstHistoryInfos);
    if (lstHistoryInfos.IsEmpty())
    {
        return IMS_NULL;
    }

    const AString& strHistoryInfo = lstHistoryInfos.GetAt(lstHistoryInfos.GetSize() - 1);
    if (strHistoryInfo.GetLength() == 0)
    {
        return IMS_NULL;
    }

    return SipParsingHelper::CreateHeader(SipHeaderName::HISTORY_INFO, strHistoryInfo);
}

PRIVATE
IMS_BOOL MtcSupplementaryService::GetCdivCause(
        IN const SipAddress* pAddress, OUT IMS_SINT32& nCause)
{
    if (pAddress != IMS_NULL)
    {
        const SipParameter* pSIPParameter = pAddress->GetParameter("cause");

        if (pSIPParameter != IMS_NULL)
        {
            const AString strCause = pSIPParameter->GetValue();

            if (strCause.GetLength() > 0)
            {
                IMS_TRACE_D("GetCDIVCause : Cause=%s", strCause.GetStr(), 0, 0);

                nCause = strCause.ToInt32();
                return IMS_TRUE;
            }
        }
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL MtcSupplementaryService::GetCdivTarget(
        IN const SipAddress* pAddress, OUT AString& strTarget)
{
    if (pAddress == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetCDIVTarget : pAddress is NULL", 0, 0, 0);
        return IMS_FALSE;
    }

    if (pAddress->IsSchemeSip() || pAddress->IsSchemeSips())
    {
        strTarget = pAddress->GetUser();
    }
    else if (pAddress->IsSchemeTel())
    {
        strTarget = pAddress->GetHost();
    }
    else
    {
        IMS_TRACE_I("GetCDIVTarget : Getting the target failed", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_TRACE_D("GetCDIVTarget : Target=%s", strTarget.GetStr(), 0, 0);
    return IMS_TRUE;
}

PRIVATE
IMS_SINT32 MtcSupplementaryService::ConvertCdivCause(IN IMS_SINT32 nCause)
{
    IMS_SINT32 nCDIVCause;

    switch (nCause)
    {
        case 302:
            nCDIVCause = static_cast<IMS_SINT32>(CdivCause::UNCONDITION);
            break;
        case 404:
            nCDIVCause = static_cast<IMS_SINT32>(CdivCause::NOT_LOGGED_IN);
            break;
        case 408:
            nCDIVCause = static_cast<IMS_SINT32>(CdivCause::NO_REPLY);
            break;
        case 480:
            nCDIVCause = static_cast<IMS_SINT32>(CdivCause::DEFLECTION);
            break;
        case 486:
            nCDIVCause = static_cast<IMS_SINT32>(CdivCause::BUSY);
            break;
        case 487:
            nCDIVCause = static_cast<IMS_SINT32>(CdivCause::DEFLECTION_ALERTING);
            break;
        case 503:
            nCDIVCause = static_cast<IMS_SINT32>(CdivCause::NOT_REACHABLE);
            break;

        default:
            nCDIVCause = static_cast<IMS_SINT32>(CdivCause::NONE);
            break;
    }

    return nCDIVCause;
}

PRIVATE
IMS_SINT32 MtcSupplementaryService::GetCallingNumVerificationResult(IN AString& strValue)
{
    IMS_SINT32 nVerstatResult = CALLING_NUM_VERSTAT_NONE;

    if (strValue.GetLength() > 0)
    {
        if (strValue.EqualsIgnoreCase(STR_VERSTAT_TN_VALIDATION_PASSED))
        {
            nVerstatResult = CALLING_NUM_VERSTAT_VERIFIED;
        }
        else if (strValue.EqualsIgnoreCase(STR_VERSTAT_TN_VALIDATION_FAILED))
        {
            nVerstatResult = CALLING_NUM_VERSTAT_NOT_VERIFIED;
        }
    }

    IMS_TRACE_D("GetCallingNumVerificationResult : result is [%d]", nVerstatResult, 0, 0);
    return nVerstatResult;
}

PRIVATE
IMS_SINT32 MtcSupplementaryService::GetCnvHeaderType(IN IMessage* piMessage)
{
    if (m_nCnapType == CNAP_SCHEME_PAID ||
            (m_nCnapType == CNAP_SCHEME_PAID_FROM &&
                    MessageUtil::IsHeaderPresent(piMessage, ISipHeader::P_ASSERTED_IDENTITY)))
    {
        IMS_TRACE_D("GetCNVHeaderType - P_ASSERTED_IDENTITY", 0, 0, 0);
        return ISipHeader::P_ASSERTED_IDENTITY;
    }

    IMS_TRACE_D("GetCNVHeaderType - FROM", 0, 0, 0);
    return ISipHeader::FROM;
}

PRIVATE
void MtcSupplementaryService::LoadConfig()
{
    IMS_BOOL bOipFrom = m_objConfigurationProxy.Is(Feature::OIP_SOURCE_FROM_HEADER);

    if (bOipFrom)
    {
        m_nCnapType = CNAP_SCHEME_FROM;
    }

    IMS_TRACE_I("LoadConfig : Done", 0, 0, 0);
}

PRIVATE
IMS_BOOL MtcSupplementaryService::IsExist(IN SuppType suppType)
{
    IMS_SLONG nIndex = m_objSuppService.GetIndexOfKey(suppType);

    if (nIndex >= 0)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}
