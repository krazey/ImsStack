// TODO: this class will be deprecated.

#include "ServiceMemory.h"
#include "ServiceThread.h"
#include "ServiceTrace.h"

#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"

#include "conferencecall/ConferenceConfigurationWrapper.h"
#include "MtcDef.h"
#include "MtcContextRepository.h"
#include "IMtcContext.h"


__IMS_TRACE_TAG_COM_MTC__;

#ifdef _PUBLIC_METHOD_
#endif

PUBLIC GLOBAL
IMS_BOOL ConferenceConfigurationWrapper::IsSupported()
{
    return IsConferenceFeatureFlagged(FLAG_CONFERENCE);
}

PUBLIC GLOBAL
IMS_BOOL ConferenceConfigurationWrapper::IsConferenceSubscriptionRequired()
{
    return IsConferenceFeatureFlagged(FLAG_CONFERENCE_SUBSCRIPTION);
}

PUBLIC GLOBAL
IMS_BOOL ConferenceConfigurationWrapper::IsReferSubscriptionRequired()
{
    return IsConferenceFeatureFlagged(FLAG_REFER_SUBSCRIPTION);
}

PUBLIC GLOBAL
IMS_BOOL ConferenceConfigurationWrapper::IsSubscriptionOutDialog()
{
    return IsConferenceFeatureFlagged(FLAG_SUBSCRIPTION_OUTDIALOG);
}

PUBLIC GLOBAL
IMS_BOOL ConferenceConfigurationWrapper::IsReferredByRequired()
{
    return IsConferenceFeatureFlagged(FLAG_REFERRED_BY);
}

PUBLIC GLOBAL
IMS_BOOL ConferenceConfigurationWrapper::IsAddUserParameter()
{
    return IsConferenceFeatureFlagged(FLAG_ADD_USER_PARAMETER);
}

PUBLIC GLOBAL
IMS_BOOL ConferenceConfigurationWrapper::IsPackageVersionCheckRequired()
{
    return IMS_FALSE;
    //return IsConferenceFeatureFlagged(FLAG_PACKAGE_VERSION_CHECK);
}

PUBLIC GLOBAL
IMS_BOOL ConferenceConfigurationWrapper::IsImplicitRoutingRequired()
{
    return IsConferenceFeatureFlagged(FLAG_IMPLICIT_ROUTING);
}

PUBLIC GLOBAL
IMS_BOOL ConferenceConfigurationWrapper::IsSubscriptionFirst()
{
    return IsConferenceFeatureFlagged(FLAG_SUBSCRIPTION_FIRST);
}

PUBLIC GLOBAL
IMS_BOOL ConferenceConfigurationWrapper::IsPaidPreferred()
{
    return IsConferenceFeatureFlagged(FLAG_PAID_PREFERRED);
}

PUBLIC GLOBAL
IMS_BOOL ConferenceConfigurationWrapper::IsReUseReferToUri()
{
    return IsConferenceFeatureFlagged(FLAG_REUSE_REFER_TO_URI_IN_INVITE);
}

PUBLIC GLOBAL
IMS_BOOL ConferenceConfigurationWrapper::IsReferUsed()
{
    return IsConferenceFeatureFlagged(FLAG_USE_REFER);
}

PUBLIC GLOBAL
IMS_BOOL ConferenceConfigurationWrapper::IsDisconnectingStatusUsed()
{
    return IsConferenceFeatureFlagged(FLAG_USE_DISCONNECTING_STATUS);
}

PUBLIC GLOBAL
IMS_BOOL ConferenceConfigurationWrapper::IsReferToExHeaderUsed()
{
    return IsConferenceFeatureFlagged(FLAG_USE_REFER_TO_EX_HDR);
}

PUBLIC GLOBAL
IMS_BOOL ConferenceConfigurationWrapper::IsSubscriptionForParticipantRequired()
{
    return IsConferenceFeatureFlagged(FLAG_SUBSCRIPTION_FOR_PARTICIPANT);
}

PUBLIC GLOBAL
IMS_SINT32 ConferenceConfigurationWrapper::GetWaitTimeInitiation()
{
    return 2000;
}

PUBLIC GLOBAL
IMS_SINT32 ConferenceConfigurationWrapper::GetWaitTimeNotifyActive()
{
    return 2000;
}

PUBLIC GLOBAL
IMS_SINT32 ConferenceConfigurationWrapper::GetWaitTimeNotifyTerminated()
{
    // if this value is less than 0, no Un-Subscription
    return 3000;
}

PUBLIC GLOBAL
IMS_SINT32 ConferenceConfigurationWrapper::GetWaitTimeSipFrag()
{
    return 2000;
}

PUBLIC GLOBAL
IMS_SINT32 ConferenceConfigurationWrapper::GetReferTypeForInvite()
{
    return MtcContextRepository::GetContext(ThreadService::GetCurrentSlotId())
            ->GetConfigurationProxy().GetInt(Feature::CONFERENCE_INVITING_REFER_TYPE);
}

#ifdef _PRIVATE_METHOD_
#endif

PRIVATE GLOBAL
IMS_BOOL ConferenceConfigurationWrapper::IsConferenceFeatureFlagged(IN IMS_UINT32 nFeature)
{
    IMS_SINT32 nSlotId = ThreadService::GetCurrentSlotId();
    // TODO, MTC BUILD
    UNUSED_PARAM(nSlotId);
    const AString& strFeatures = "0x2911000F";
    IMS_UINT32 nConferenceFeatures = strFeatures.ToUInt32(IMS_NULL, 16);

    return ((nConferenceFeatures&  nFeature) != 0);
}
