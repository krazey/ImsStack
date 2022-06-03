#include "call/message/EmergencyMessageFormatter.h"
#include "ICoreService.h"
#include "IFeatureCaps.h"
#include "ISipHeader.h"
#include "IMtcApp.h"
#include "IMtcService.h"
#include "utility/MessageUtil.h"
#include "ServicePhoneInfo.h"
#include "Sip.h"
#include "SipParameter.h"
#include "TextParser.h"
#include "helper/MtcAosConnector.h"
#include "call/IMtcSessionContext.h"

__IMS_TRACE_TAG_COM_MTC__;

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
EmergencyMessageFormatter::EmergencyMessageFormatter(IN IMtcSessionContext& objContext) :
        MessageFormatter(objContext)
{
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC VIRTUAL EmergencyMessageFormatter::~EmergencyMessageFormatter() {}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC VIRTUAL IMS_RESULT EmergencyMessageFormatter::FormStartMessage()
{
    if (MessageFormatter::FormStartMessage() == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    m_eNormalAosRegMode = GetAoSRegMode(ServiceType::NORMAL);
    m_eEmergencyAosRegMode = GetAoSRegMode(ServiceType::EMERGENCY);
    if ((m_eNormalAosRegMode == IImsAosInfo::REG_MODE_UNKNOWN) ||
            (m_eEmergencyAosRegMode == IImsAosInfo::REG_MODE_UNKNOWN))
    {
        return IMS_FAILURE;
    }

    SetPPreferredIdentityHeader();
    SetSipInstanceFeature();

    return IMS_SUCCESS;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
void EmergencyMessageFormatter::SetPPreferredIdentityHeader()
{
    if (MessageUtil::IsHeaderPresent(m_piNextMessage, ISipHeader::P_PREFERRED_IDENTITY))
    {
        return;
    }

    if ((m_eNormalAosRegMode != IImsAosInfo::REG_MODE_NORMAL) &&
            (m_eNormalAosRegMode != IImsAosInfo::REG_MODE_ADMIN) &&
            (m_eEmergencyAosRegMode != IImsAosInfo::REG_MODE_NORMAL) &&
            (m_eEmergencyAosRegMode != IImsAosInfo::REG_MODE_ADMIN))
    {
        return SetPPreferredIdentityHeaderByDeviceId();
    }

    SetPPreferredIdentityHeaderByUserId();
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
void EmergencyMessageFormatter::SetPPreferredIdentityHeaderByUserId()
{
    ICoreService* piCoreService = GetICoreService();
    if (piCoreService == IMS_NULL)
    {
        return;
    }

    CONST AStringArray& objUserIds = piCoreService->GetUserIdentities();
    for (IMS_SINT32 i = 0; i < objUserIds.GetCount(); i++)
    {
        SipAddress objSipAddress;
        AString strAddress = objUserIds.GetElementAt(i);
        if (!objSipAddress.Create(strAddress))
        {
            continue;
        }

        MessageUtil::SetHeader(m_piNextMessage, strAddress, ISipHeader::P_PREFERRED_IDENTITY);
        return;
    }
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
void EmergencyMessageFormatter::SetPPreferredIdentityHeaderByDeviceId()
{
    AString strIpAddress;
    if (GetLocalIpAddress(strIpAddress) != IMS_SUCCESS)
    {
        return;
    }

    IMS_UINT32 nPort = GetLocalPort();
    if (nPort == 0)
    {
        return;
    }

    IMS_SINT32 nSlotId = 0;  // TODO
    AString strImei;
    PhoneInfoService::GetPhoneInfoService()->GetDeviceInfo()->GetDeviceId(nSlotId, strImei);

    LOCAL CONST IMS_UINT32 LEN_IMEI_TAC = 8;
    LOCAL CONST IMS_UINT32 LEN_IMEI_SNR = 6;
    strImei = strImei.AlignLeft(LEN_IMEI_TAC + LEN_IMEI_SNR, '0');

    AString strValue;
    strValue.Append(Sip::STR_SIP);
    strValue.Append(TextParser::CHAR_COLON);
    strValue.Append(strImei.GetSubStr(0, LEN_IMEI_TAC));
    strValue.Append(strImei.GetSubStr(LEN_IMEI_TAC, LEN_IMEI_SNR));
    strValue.Append('0');
    strValue.Append(TextParser::STR_AT);

    IPAddress objIpAddress;
    if (!objIpAddress.Parse(strIpAddress) || objIpAddress.IsIPv4Address())
    {
        strValue.Append(strIpAddress);
    }
    else
    {
        strValue.Append(TextParser::STR_LSBRACKET);
        strValue.Append(strIpAddress);
        strValue.Append(TextParser::STR_RSBRACKET);
    }
    strValue.Append(TextParser::CHAR_COLON);

    AString strPort;
    strPort.Sprintf("%d", nPort);
    strValue.Append(strPort);

    MessageUtil::SetHeader(m_piNextMessage, strValue, ISipHeader::P_PREFERRED_IDENTITY);
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
void EmergencyMessageFormatter::SetSipInstanceFeature()
{
    if ((m_eNormalAosRegMode == IImsAosInfo::REG_MODE_NORMAL) ||
            (m_eNormalAosRegMode == IImsAosInfo::REG_MODE_ADMIN))
    {
        return;
    }

    ICoreService* piCoreService = GetICoreService();
    if (piCoreService == IMS_NULL)
    {
        return;
    }

    CONST SipParameter* pParameter = piCoreService->GetInstanceParameter();
    if (pParameter == IMS_NULL)
    {
        return;
    }

    IFeatureCaps* piFeatureCaps = GetIFeatureCaps();
    if (piFeatureCaps == IMS_NULL)
    {
        return;
    }

    piFeatureCaps->AddFeature(pParameter->GetName(), pParameter->GetValue());
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
IMS_UINT32 EmergencyMessageFormatter::GetAoSRegMode(IN ServiceType eServiceType)
{
    MtcAosConnector* pAosConnector = m_objContext.GetAosConnector(eServiceType);
    if (pAosConnector == IMS_NULL)
    {
        return IImsAosInfo::REG_MODE_UNKNOWN;
    }

    return pAosConnector->GetRegistrationMode();
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
IMS_RESULT EmergencyMessageFormatter::GetLocalIpAddress(OUT AString& strIpAddress)
{
    // TODO: emerency??
    MtcAosConnector* pAosConnector = m_objContext.GetAosConnector(ServiceType::EMERGENCY);
    if (pAosConnector == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    strIpAddress = pAosConnector->GetLocalAddress();
    if (strIpAddress.GetLength() < 1)
    {
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
IMS_UINT32 EmergencyMessageFormatter::GetLocalPort()
{
    MtcAosConnector* pAosConnector = m_objContext.GetAosConnector(ServiceType::EMERGENCY);
    if (pAosConnector == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    return pAosConnector->GetLocalPort();
}
