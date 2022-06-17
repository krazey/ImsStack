#include "ImsTrace.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "SipParsingHelper.h"
#include "TextParser.h"
#include "call/IMtcCall.h"
#include "call/IMtcCallContext.h"
#include "call/MtcSession.h"
#include "call/message/MtcMessageMediator.h"
#include "configuration/MtcConfigurationProxy.h"
#include "utility/MessageUtil.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcMessageMediator::MtcMessageMediator(IN IMtcCallContext& objContext) :
        m_objContext(objContext),
        m_strOriginalContactHeader(AString::ConstEmpty())
{
}

PUBLIC
MtcMessageMediator::~MtcMessageMediator() {}

PUBLIC IMS_RESULT MtcMessageMediator::MessageMediator_AdjustMessage(
        IN_OUT ISipMessage* piSipMessage, IN IMS_SINT32 /* nMessage */)
{
    if (m_strOriginalContactHeader.GetLength() <= 0)
    {
        m_strOriginalContactHeader = piSipMessage->GetHeader(ISipHeader::CONTACT_NORMAL);
    }

    if (piSipMessage->IsHeaderPresent(ISipHeader::CONTACT_NORMAL) &&
            m_objContext.GetConfigurationProxy().Is(
                    Feature::SET_VIDEO_TEXT_FEATURE_EXCLUSIVELY_IN_CONTACT_HEADER_BY_SESSION_TYPE))
    {
        switch (GetCallTypeOfCurrentMessage())
        {
            case CallType::VT:
                piSipMessage->SetHeader(ISipHeader::CONTACT_NORMAL,
                        GetContactHeaderWithoutFeatureTag(MessageUtil::STR_TEXT));
                break;
            case CallType::RTT:
                piSipMessage->SetHeader(ISipHeader::CONTACT_NORMAL,
                        GetContactHeaderWithoutFeatureTag(MessageUtil::STR_VIDEO));
                break;
            default:
                piSipMessage->SetHeader(ISipHeader::CONTACT_NORMAL, m_strOriginalContactHeader);
                break;
        }
    }

    return IMS_SUCCESS;
}

PRIVATE
AString MtcMessageMediator::GetContactHeaderWithoutFeatureTag(IN const AString& strFeatureTag)
{
    IMS_TRACE_D(
            "GetContactHeaderWithoutFeatureTag : Feature tag[%s]", strFeatureTag.GetStr(), 0, 0);

    ISipHeader* piHeader =
            SipParsingHelper::CreateHeader(ISipHeader::CONTACT_NORMAL, m_strOriginalContactHeader);
    piHeader->RemoveParameter(strFeatureTag);
    AString strModifiedHeader = piHeader->GetHeaderValue();
    piHeader->Destroy();

    return strModifiedHeader;
}

PRIVATE
CallType MtcMessageMediator::GetCallTypeOfCurrentMessage()
{
    // VZ_REQ_5GNRSAVOICEVIDEO_4105999311948863
    // The device shall treat a "downgraded video call" as a video call, ...
    CallType eCallType = MessageUtil::GetCallTypeFromSdp(
            &m_objContext.GetSession()->GetISession(), IMS_FALSE, IMS_TRUE, IMS_FALSE);
    if (eCallType != CallType::UNKNOWN)
    {
        return eCallType;
    }

    return m_objContext.GetSession()->GetCallType();
}
