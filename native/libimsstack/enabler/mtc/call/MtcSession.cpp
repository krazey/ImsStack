#include "IMessage.h"
#include "ImsAosParameter.h"
#include "ISession.h"
#include "ISipHeader.h"
#include "ServiceSystemTime.h"
#include "ServiceTrace.h"
#include "call/MtcSession.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MtcAosConnector.h"
#include "helper/sipinterfaceholder/MtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/SessionInterfaceHolder.h"
#include "media/IMtcMediaManager.h"
#include "precondition/IMtcPreconditionManager.h"
#include "utility/MessageUtil.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcSession::MtcSession(
        IN IMtcCallContext& objContext, IN ISession& objSession, IN CallType eCallType) :
        m_objContext(objContext),
        m_objSession(objSession),
        m_objMessageSender(MessageSender(*this)),
        m_objExtensionSet(GetSupportedOptionTags()),
        m_eCallType(eCallType),
        m_bVideoCapable(IMS_FALSE),
        m_bRttCapable(IMS_FALSE),
        m_strSessionIdHeader(AString::ConstNull())
{
    IMS_TRACE_I("+MtcSession", 0, 0, 0);
    if (m_objContext.GetCallInfo().ePeerType == PeerType::MT)
    {
        GetSipInterfaceFactory().GetISessionHolder()->AddISession(&m_objSession);
    }

    UpdateSessionProperty();

    m_bVideoCapable = HasAosFeature(ImsAosFeature::MMTEL) && HasAosFeature(ImsAosFeature::VIDEO);
    m_bRttCapable = HasAosFeature(ImsAosFeature::MMTEL) && HasAosFeature(ImsAosFeature::TEXT);

    if (GetConfigurationProxy().Is(Feature::SUPPORT_SIP_SESSION_ID_HEADER))
    {
        m_strSessionIdHeader = GenerateSessionId();
    }
}

PUBLIC VIRTUAL MtcSession::~MtcSession()
{
    IMS_TRACE_I("~MtcSession", 0, 0, 0);
    GetSipInterfaceFactory().GetISessionHolder()->ReleaseISession(&m_objSession);
}

PUBLIC IMS_RESULT MtcSession::SendStart()
{
    if (m_objContext.GetMediaManager().FormSdp(&m_objSession, CallType::VOIP) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    m_objContext.GetPreconditionManager().FormPreconditionSdp(&m_objSession, IMS_FALSE);

    return m_objMessageSender.Start();
}

PUBLIC
void MtcSession::HandleRequest(IN IMS_UINT32 nMethod, IN const IMessage& objRequest)
{
    m_objExtensionSet.HandleRequest(nMethod, objRequest);

    if (nMethod == IMessage::SESSION_START || nMethod == IMessage::SESSION_EARLY_UPDATE)
    {
        UpdateFromRemoteMessage(objRequest);
    }

    if (m_eCallType == CallType::UNKNOWN)
    {
        // UE must send full media list for the incoming INVITE w/o SDP
        // TODO: but, let us optimize.
        m_eCallType = CallType::VOIP;
        m_objContext.GetMediaManager().UpdateMediaDirection(
                MEDIATYPE_AUDIO, DIRECTION_SEND_RECEIVE);
    }
}

PUBLIC
void MtcSession::HandleResponse(IN IMS_UINT32 nMethod, IN const IMessage& objResponse)
{
    m_objExtensionSet.HandleResponse(nMethod, objResponse);

    if (nMethod == IMessage::SESSION_START || nMethod == IMessage::SESSION_EARLY_UPDATE)
    {
        UpdateFromRemoteMessage(objResponse);
    }
}

PRIVATE
IMSList<AString> MtcSession::GetSupportedOptionTags() const
{
    IMSList<AString> lstOptionTags;

    lstOptionTags.Append(MtcExtensionSet::OPTION_TAG_EARLY_DIALOG_TERMINATED);
    lstOptionTags.Append(MtcExtensionSet::OPTION_TAG_FROM_CHANGE);
    lstOptionTags.Append(MtcExtensionSet::OPTION_TAG_HISTORY_INFO);
    lstOptionTags.Append(MtcExtensionSet::OPTION_TAG_REPLACES);
    lstOptionTags.Append(MtcExtensionSet::OPTION_TAG_RPR);
    lstOptionTags.Append(MtcExtensionSet::OPTION_TAG_TARGET_DIALOG);
    lstOptionTags.Append(MtcExtensionSet::OPTION_TAG_TIMER);

    // TODO: check CallType.
    if (m_objContext.GetConfigurationProxy().Is(Feature::VOICE_QOS_PRECONDITION_SUPPORTED))
    {
        lstOptionTags.Append(MtcExtensionSet::OPTION_TAG_PRECONDITION);
    }

    return lstOptionTags;
}

PRIVATE
void MtcSession::UpdateSessionProperty()
{
    IMS_SINT32 nInterval =
            m_objContext.GetConfigurationProxy().GetInt(Feature::SESSION_REFRESH_TRIGGER_INTERVAL);
    if (nInterval > 0)
    {
        m_objSession.SetRefreshPolicy(ISession::REFRESH_POLICY_REMAIN_TIME, 0, 0, nInterval);
    }

    m_objSession.SetImplicitRoutingRequired(IMS_TRUE);
}

void MtcSession::UpdateFromRemoteMessage(IN const IMessage& objMessage)
{
    CallType eNewCallType = MessageUtil::GetCallType(&objMessage, &m_objSession, IMS_TRUE);
    if (eNewCallType != CallType::UNKNOWN)
    {
        m_eCallType = eNewCallType;
    }

    m_bVideoCapable = HasAosFeature(ImsAosFeature::MMTEL) && HasAosFeature(ImsAosFeature::VIDEO) &&
            MessageUtil::IsVideoFeatureIncluded(&objMessage);
    m_bRttCapable = HasAosFeature(ImsAosFeature::MMTEL) && HasAosFeature(ImsAosFeature::TEXT) &&
            MessageUtil::IsTextFeatureIncluded(&objMessage);

    MessageUtil::GetHeader(&objMessage, ISipHeader::UNKNOWN, m_strSessionIdHeader, "Session-ID");

    IMS_TRACE_D("UpdateFromRemoteMessage : CallType[%d] Video[%s] Rtt[%s]", m_eCallType,
            _TRACE_B_(m_bVideoCapable), _TRACE_B_(m_bRttCapable));
}

PRIVATE
AString MtcSession::GenerateSessionId() const
{
    // Pseudo-random 128-bit system secret key
    AString strSessionId;
    strSessionId.Sprintf("%08x%08x%08x%08x", IMS_SYS_GetTimeInMicroSeconds(), IMS_SYS_GetRandom0(),
            IMS_SYS_GetRandom0(), IMS_SYS_GetRandom0());

    return strSessionId;
}

PRIVATE
IMS_BOOL MtcSession::HasAosFeature(IMS_UINT32 nFeature)
{
    MtcAosConnector* pAosConnector = GetAosConnector(GetService().GetServiceType());
    if (pAosConnector == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return pAosConnector->GetFeatures() & nFeature;
}
