#include "ServiceTrace.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "conferencecall/ConferenceConfigurationWrapper.h"
#include "MtcDef.h"
#include "MtcContextRepository.h"
#include "IMtcContext.h"


__IMS_TRACE_TAG_COM_MTC__;

PUBLIC GLOBAL
IMS_BOOL ConferenceConfigurationWrapper::IsSupported()
{
    return IMS_TRUE;
}

PUBLIC GLOBAL
IMS_BOOL ConferenceConfigurationWrapper::IsConferenceSubscriptionRequired()
{
    return MtcContextRepository::GetContext()
            ->GetConfigurationProxy().GetInt(Feature::CONFERENCE_SUBSCRIBE_TYPE) > -1 ;
}

PUBLIC GLOBAL
IMS_BOOL ConferenceConfigurationWrapper::IsReferSubscriptionRequired()
{
    return MtcContextRepository::GetContext()
            ->GetConfigurationProxy().Is(Feature::SUPPORT_CONFERENCE_REFER_SUBSCRIBE);
}

PUBLIC GLOBAL
IMS_BOOL ConferenceConfigurationWrapper::IsSubscriptionOutDialog()
{
    return MtcContextRepository::GetContext()
            ->GetConfigurationProxy().GetInt(Feature::CONFERENCE_SUBSCRIBE_TYPE) > 0;
}

PUBLIC GLOBAL
IMS_BOOL ConferenceConfigurationWrapper::IsPackageVersionCheckRequired()
{
    return MtcContextRepository::GetContext()
            ->GetConfigurationProxy().Is(Feature::CHECK_CONFERENCE_EVENT_PACKAGE_VERSION);
}

PUBLIC GLOBAL
IMS_BOOL ConferenceConfigurationWrapper::IsSubscriptionFirst()
{
    return MtcContextRepository::GetContext()
            ->GetConfigurationProxy().GetInt(Feature::CONFERENCE_SIP_FLOW_ORDER) == 0;
}

PUBLIC GLOBAL
IMS_BOOL ConferenceConfigurationWrapper::IsPaidPreferred()
{
    return MtcContextRepository::GetContext()
            ->GetConfigurationProxy().Is(Feature::CONFERENCE_REFER_TO_URI_SOURCE_PAID);
}

PUBLIC GLOBAL
IMS_BOOL ConferenceConfigurationWrapper::IsReUseReferToUri()
{
    return MtcContextRepository::GetContext()
            ->GetConfigurationProxy().Is(Feature::CONFERENCE_DROP_REFER_TO_URI_SOURCE_TYPE);
}

PUBLIC GLOBAL
IMS_BOOL ConferenceConfigurationWrapper::IsReferUsed()
{
    return MtcContextRepository::GetContext()
            ->GetConfigurationProxy().GetInt(Feature::CONFERENCE_INVITING_REFER_TYPE) == 1;
}

PUBLIC GLOBAL
IMS_BOOL ConferenceConfigurationWrapper::IsDisconnectingStatusUsed()
{
    // TODO: it's really not necessary?
    return IMS_TRUE;
}

PUBLIC GLOBAL
IMS_BOOL ConferenceConfigurationWrapper::IsReferToExHeaderUsed()
{
    return MtcContextRepository::GetContext()
            ->GetConfigurationProxy().Is(Feature::ADD_REPLACE_HEADER_FOR_CONFERENCE);
}

PUBLIC GLOBAL
IMS_BOOL ConferenceConfigurationWrapper::IsSubscriptionForParticipantRequired()
{
    return MtcContextRepository::GetContext()
            ->GetConfigurationProxy().Is(Feature::ENABLE_CONFERENCE_SUBSCRIBE_BY_PARTICIPANT);
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
    return MtcContextRepository::GetContext()
            ->GetConfigurationProxy().GetInt(Feature::CONFERENCE_INVITING_REFER_TYPE);
}
