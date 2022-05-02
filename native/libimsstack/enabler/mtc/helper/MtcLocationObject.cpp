/*
    Author
    <table>
    date          author                        description
    --------      --------------                ----------
    20150415    il.won@                   Created
    </table>

    Description

*/
#include "CarrierConfig.h"
#include "ServiceConfig.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "Configuration.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "SipHeaderName.h"
#include "IMessage.h"
#include "IMessageBodyPart.h"
#include "GeolocationHelper.h"

#include "define/MtcStringDef.h"
#include "utility/MessageUtil.h"
#include "IMtcService.h"
#include "helper/MtcLocationObject.h"
#include "helper/MtcSupplementaryService.h"

__IMS_TRACE_TAG_COM_UC__;


PUBLIC GLOBAL
const IMS_CHAR UCLocationObject::STR_APPLICATION_PIDF_XML[] = "application/pidf+xml";
PUBLIC GLOBAL
const IMS_CHAR UCLocationObject::STR_GEOLOCATION[] = "Geolocation";
PUBLIC GLOBAL
const IMS_CHAR UCLocationObject::STR_GEOLOCATION_ROUTING[] = "Geolocation-Routing";
PUBLIC GLOBAL
const IMS_CHAR UCLocationObject::STR_NO[] = "no";
PUBLIC GLOBAL
const IMS_CHAR UCLocationObject::STR_YES[] = "yes";

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC
UCLocationObject::UCLocationObject()
{
    IMS_TRACE_MEM("uc", "uc_M : UCLocationObject[%" PFLS_u "][%" PFLS_x "]",
            sizeof(UCLocationObject), this, 0);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL
UCLocationObject::~UCLocationObject()
{
    IMS_TRACE_MEM("uc", "uc_F : UCLocationObject[%" PFLS_u "][%" PFLS_x "]",
            sizeof(UCLocationObject), this, 0);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL
IMS_BOOL UCLocationObject::IsGeolocationInfoRequired(IN IMtcCall *pSession)
{
    // TODO, MTC BUILD
    IMS_SINT32 nSlotID = 0;
    IMtcService *pService = IMS_NULL;
    MtcSupplementaryService* pMtcSuppService = IMS_NULL;
    UNUSED_PARAM(pSession);
    // IMS_SINT32 nSlotID = pSession->GetSlotID();
    // IMtcService *pService = pSession->GetService();
    // IMtcSupplementaryService *pMtcSuppService = pSession->GetSuppService();

    if (!pService->IsEmergency())
    {
        return IMS_FALSE;
    }

    if (!(IsGeolocationPidfSupported(nSlotID,
            CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_EMERGENCY_ON_CELLULAR)
        || IsGeolocationPidfSupported(nSlotID,
            CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_EMERGENCY_ON_WIFI)))
    {
        return IMS_FALSE;
    }

    const SuppService *pSuppService = pMtcSuppService->Get(SuppType::GEOLOCATION);
    if (pSuppService == IMS_NULL || !pSuppService->bValue)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_I("IsGeolocationInfoRequired : TRUE", 0, 0, 0);
    return IMS_TRUE;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC
void UCLocationObject::SetLocation(IN IMtcCall *pSession, IN_OUT IMessage *piMessage,
        IN IMS_BOOL bGeolocationRouting /* = IMS_FALSE */)
{
    if (!IsGeolocationInfoRequired(pSession))
    {
        return;
    }

    SetLocation(pSession, piMessage->GetMessage(), bGeolocationRouting);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC
void UCLocationObject::SetLocation(IN IMtcCall *pSession, IN_OUT ISIPMessage *piSIPMessage,
        IN IMS_BOOL bGeolocationRouting /* = IMS_FALSE */)
{
    // TODO, MTC BUILD
    IMS_SINT32 nSlotID = 0;
    // IMS_SINT32 nSlotID = pSession->GetSlotID();

    if (!IsGeolocationInfoRequired(pSession))
    {
        return;
    }

    GeolocationPidfCreator *pPidfCreator = GeolocationHelper::GetInstance()->GetPidfCreator(
            nSlotID);
    if (pPidfCreator == IMS_NULL)
    {
        IMS_TRACE_D("SetLocation : GeolocationPidfCreator is null", 0, 0, 0);
        return;
    }

    ByteArray objContent;
    if (!pPidfCreator->CreateWithPosition(AString::ConstNull(), objContent))
    {
        IMS_TRACE_D("SetLocation : Creating a location information failed", 0, 0, 0);
        return;
    }

    AString strContentLength;
    strContentLength.SetNumber(objContent.GetLength());

    const ISubscriberConfig *pISubscriberConfig =
            Configuration::GetInstance()->GetSubscriberConfig(nSlotID);

    if (pISubscriberConfig == IMS_NULL)
    {
        IMS_TRACE_I("SetLocation : pISubscriberConfig is null", 0, 0, 0);
        return;
    }

    AString strCID;
    MessageUtil::GenerateContentId(pISubscriberConfig->GetHomeDomainName(), strCID);

    ISIPMessageBodyPart *piBodyPart = piSIPMessage->CreateSDPBodyPart();
    piBodyPart->SetContent(objContent);
    piBodyPart->SetHeader(ISIPMessageBodyPart::CONTENT_UNKNOWN, strContentLength,
            SIPHeaderName::CONTENT_LENGTH);

    AString strContentID;
    strContentID.Sprintf("<%s>", strCID.GetStr());
    piBodyPart->SetHeader(ISIPMessageBodyPart::CONTENT_ID, strContentID);

    piBodyPart->SetHeader(ISIPMessageBodyPart::CONTENT_TYPE, STR_APPLICATION_PIDF_XML);
    piBodyPart->SetHeader(ISIPMessageBodyPart::CONTENT_DISPOSITION, "render; handling=optional");

    piSIPMessage->SetHeader(ISIPHeader::UNKNOWN, strCID.Sprintf("<cid:%s>", strCID.GetStr()),
            STR_GEOLOCATION);
    piSIPMessage->SetHeader(ISIPHeader::UNKNOWN, bGeolocationRouting ? STR_YES : STR_NO,
            STR_GEOLOCATION_ROUTING);

    IMS_TRACE_I("SetLocation : Done", 0, 0, 0);
}

PRIVATE
IMS_BOOL UCLocationObject::IsGeolocationPidfSupported(IN IMS_SINT32 nSlotId,
        IN IMS_SINT32 nGeolocationPidfType) const
{
    ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(nSlotId);
    IMSVector<IMS_SINT32> objGeolocationPidfTypes = piCc->GetIntArray(
            CarrierConfig::Ims::KEY_GEOLOCATION_PIDF_IN_SIP_INVITE_SUPPORT_INT_ARRAY);

    for (IMS_UINT32 i = 0; i < objGeolocationPidfTypes.GetSize(); ++i)
    {
        if (nGeolocationPidfType == objGeolocationPidfTypes.GetAt(i))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}
