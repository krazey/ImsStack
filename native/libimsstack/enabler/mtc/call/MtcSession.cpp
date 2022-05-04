#include "ServiceTrace.h"
#include "call/MtcSession.h"
#include "helper/sipinterfaceholder/MtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/SessionInterfaceHolder.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcSession::MtcSession(IN IMtcCallContext& objContext, IN ISession& objSession) :
        m_objContext(objContext),
        m_objSession(objSession),
        m_objMessageSender(MessageSender(*this)),
        m_objExtensionSet(GetSupportedOptionTags())
{
    IMS_TRACE_I("+MtcSession", 0, 0, 0);
    if (m_objContext.GetCallInfo().ePeerType == PeerType::MT)
    {
        GetSipInterfaceFactory().GetISessionHolder()->AddISession(&m_objSession);
    }
}

PUBLIC VIRTUAL MtcSession::~MtcSession()
{
    IMS_TRACE_I("~MtcSession", 0, 0, 0);
    GetSipInterfaceFactory().GetISessionHolder()->ReleaseISession(&m_objSession);
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
    if (/* TODO: Support precondition */ IMS_TRUE)
    {
        lstOptionTags.Append(MtcExtensionSet::OPTION_TAG_PRECONDITION);
    }

    return lstOptionTags;
}
