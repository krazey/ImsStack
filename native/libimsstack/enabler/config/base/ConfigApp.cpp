/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "ServiceMemory.h"
#include "ServiceTrace.h"

#include "Engine.h"
#include "IConfigurable.h"
#include "IConfiguration.h"
#include "ImsConfig.h"

#include "base/ConfigApp.h"

__IMS_TRACE_TAG_USER_DECL__("ConfigApp");

PUBLIC
ConfigApp::ConfigApp(IN const AString& strAppName) :
        ImsApp(strAppName)
{
}

PUBLIC VIRTUAL ConfigApp::~ConfigApp() {}

PUBLIC
void ConfigApp::Start()
{
    PostMessage(AMSG_START, 0, 0);
}

PROTECTED VIRTUAL IMS_BOOL ConfigApp::OnMessage(IN ImsMessage& objMsg)
{
    switch (objMsg.GetName())
    {
        case AMSG_START:
            // no-op
            break;
        default:
            return IMS_FALSE;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL void ConfigApp::Event_NotifyEvent(
        IN IMS_SINT32 /*nEvent*/, IN IMS_UINT32 /*nWparam*/, IN IMS_UINT32 /*nLparam*/)
{
    // Subclass MAY implement this method to handle the external events.
}

PROTECTED VIRTUAL void ConfigApp::UpdateAllForHidden(IN IMS_SINT32 nItem, IN IMS_SINT32 nParam)
{
    (void)nParam;

    IMS_BOOL bSubscriberChanged = ((nItem & ImsConfig::FLAG_IMS_SUBSCRIBER) != 0);
    IMS_BOOL bSipConfigChanged = ((nItem & ImsConfig::FLAG_IMS_SIP) != 0);
    IMS_BOOL bSipConfigVChanged = ((nItem & ImsConfig::FLAG_IMS_COM_SIP) != 0);

    IMS_TRACE_I("UpdateAllForHidden :: subscriber=%s, sip=%s, sip-v=%s",
            _TRACE_B_(bSubscriberChanged), _TRACE_B_(bSipConfigChanged),
            _TRACE_B_(bSipConfigVChanged));

    IConfiguration* piConfiguration = Engine::GetConfiguration();

    // SUBSCRIBER
    if (bSubscriberChanged)
    {
        const ISubscriberConfig* piSubConfig = piConfiguration->GetSubscriberConfig(GetSlotId());

        if (piSubConfig != IMS_NULL)
        {
            IConfigurable* piConfigurable = piSubConfig->GetConfigurable();

            if (piConfigurable != IMS_NULL)
            {
                piConfigurable->Update(IConfigurable::CP_I_SUBSCRIBER_ALL);
            }
        }
    }

    // SIP configuration
    if (bSipConfigChanged)
    {
        const ISipConfig* piSipConfig = piConfiguration->GetSipConfig(GetSlotId());

        if (piSipConfig != IMS_NULL)
        {
            IConfigurable* piConfigurable = piSipConfig->GetConfigurable();

            if (piConfigurable != IMS_NULL)
            {
                piConfigurable->Update(IConfigurable::CP_I_SIP_FEATURES);
                piConfigurable->Update(IConfigurable::CP_I_TCP_CRITERION_LENGTH);
                piConfigurable->Update(IConfigurable::CP_I_REG_EXPIRES);
                piConfigurable->Update(IConfigurable::CP_I_REG_SUB);
                piConfigurable->Update(IConfigurable::CP_I_REG_SUB_EXPIRES);
            }
        }
    }

    // SIP configuration V
    if (bSipConfigVChanged)
    {
        const ISipConfig* piSipConfig = piConfiguration->GetSipConfig(GetSlotId());

        if (piSipConfig != IMS_NULL)
        {
            const ISipConfigV* piSipConfigV = piSipConfig->GetSipConfigV();

            if (piSipConfigV != IMS_NULL)
            {
                IConfigurable* piConfigurable = piSipConfigV->GetConfigurable();

                if (piConfigurable != IMS_NULL)
                {
                    piConfigurable->Update(IConfigurable::CP_I_SIP_ALL);
                }
            }
        }
    }
}

PROTECTED VIRTUAL void ConfigApp::UpdateAllForDm(IN IMS_SINT32 nItem, IN IMS_SINT32 nParam)
{
    (void)nParam;

    IMS_BOOL bSipConfigChanged = ((nItem & ImsConfig::FLAG_IMS_SIP) != 0);
    IMS_BOOL bSipConfigVChanged = ((nItem & ImsConfig::FLAG_IMS_COM_SIP) != 0);

    IMS_TRACE_I("UpdateAllForDm :: sip=%s, sip-v=%s", _TRACE_B_(bSipConfigChanged),
            _TRACE_B_(bSipConfigVChanged), 0);

    IConfiguration* piConfiguration = Engine::GetConfiguration();

    // SIP configuration
    if (bSipConfigChanged)
    {
        const ISipConfig* piSipConfig = piConfiguration->GetSipConfig(GetSlotId());

        if (piSipConfig != IMS_NULL)
        {
            IConfigurable* piConfigurable = piSipConfig->GetConfigurable();

            if (piConfigurable != IMS_NULL)
            {
                piConfigurable->Update(IConfigurable::CP_I_SIP_FEATURES);
                piConfigurable->Update(IConfigurable::CP_I_TCP_CRITERION_LENGTH);
                piConfigurable->Update(IConfigurable::CP_I_REG_EXPIRES);
                piConfigurable->Update(IConfigurable::CP_I_REG_SUB);
                piConfigurable->Update(IConfigurable::CP_I_REG_SUB_EXPIRES);
            }
        }
    }

    // SIP configuration V
    if (bSipConfigVChanged)
    {
        const ISipConfig* piSipConfig = piConfiguration->GetSipConfig(GetSlotId());

        if (piSipConfig != IMS_NULL)
        {
            const ISipConfigV* piSipConfigV = piSipConfig->GetSipConfigV();

            if (piSipConfigV != IMS_NULL)
            {
                IConfigurable* piConfigurable = piSipConfigV->GetConfigurable();

                if (piConfigurable != IMS_NULL)
                {
                    piConfigurable->Update(IConfigurable::CP_I_SIP_ALL);
                }
            }
        }
    }
}

PROTECTED VIRTUAL void ConfigApp::UpdateItemForPst(IN IMS_UINT32 nItem, IN IMS_UINT32 nValue)
{
    IMS_TRACE_I("UpdateItemForPst :: item=%d, value=%d", nItem, nValue, 0);

    switch (nItem)
    {
        case ImsConfig::PST_I_PCSCF_ADDRESS:
            UpdateSubscriberConfig(IConfigurable::CP_I_PCSCF_ADDRESS_0);
            break;
        case ImsConfig::PST_I_PCSCF_PORT:
            UpdateSubscriberConfig(IConfigurable::CP_I_PCSCF_PORT_0);
            break;
        case ImsConfig::PST_I_TV_T1:
            UpdateSipConifgV(IConfigurable::CP_I_TIMER_T1);
            break;
        case ImsConfig::PST_I_TV_T2:
            UpdateSipConifgV(IConfigurable::CP_I_TIMER_T2);
            break;
        case ImsConfig::PST_I_TV_TF:
            UpdateSipConifgV(IConfigurable::CP_I_TIMER_F);
            break;
        case ImsConfig::PST_I_SESSION_TIMER:
            UpdateSipConifgV(IConfigurable::CP_I_SESSION_EXPIRES);
            break;
        case ImsConfig::PST_I_MIN_SE:
            UpdateSipConifgV(IConfigurable::CP_I_SESSION_MINSE);
            break;
        default:
            break;
    }
}

PROTECTED VIRTUAL void ConfigApp::UpdateItemForSdm(IN IMS_UINT32 nItem, IN IMS_UINT32 nValue)
{
    IMS_TRACE_I("UpdateItemForSdm :: item=%d, value=%d", nItem, nValue, 0);

    switch (nItem)
    {
        case ImsConfig::SDM_I_HOME_DOMAIN_NAME:
            UpdateSubscriberConfig(IConfigurable::CP_I_HOME_DOMAIN_NAME);
            break;
        case ImsConfig::SDM_I_TV_T1:
            UpdateSipConifgV(IConfigurable::CP_I_TIMER_T1);
            break;
        case ImsConfig::SDM_I_TV_T2:
            UpdateSipConifgV(IConfigurable::CP_I_TIMER_T2);
            break;
        case ImsConfig::SDM_I_TV_TF:
            UpdateSipConifgV(IConfigurable::CP_I_TIMER_F);
            break;
        case ImsConfig::SDM_I_SIP_SESSION_TIMER:
            UpdateSipConifgV(IConfigurable::CP_I_SESSION_EXPIRES);
            break;
        case ImsConfig::SDM_I_MIN_SE:
            UpdateSipConifgV(IConfigurable::CP_I_SESSION_MINSE);
            break;
        default:
            break;
    }
}

PROTECTED
IMS_BOOL ConfigApp::UpdateSipConifgV(
        IN IMS_SINT32 nCpi, IN const AString& /*strServiceId = AString::ConstNull()*/)
{
    const ISipConfig* piSipConfig = Engine::GetConfiguration()->GetSipConfig(GetSlotId());

    if (piSipConfig != IMS_NULL)
    {
        const ISipConfigV* piSipConfigV = piSipConfig->GetSipConfigV();

        if (piSipConfigV != IMS_NULL)
        {
            IConfigurable* piConfigurable = piSipConfigV->GetConfigurable();

            if (piConfigurable != IMS_NULL)
            {
                if (!piConfigurable->Update(nCpi))
                {
                    IMS_TRACE_E(0, "UpdateSipConifgV :: [%d] failed", nCpi, 0, 0);
                    return IMS_FALSE;
                }

                return IMS_TRUE;
            }
        }
    }

    return IMS_FALSE;
}

PROTECTED
IMS_BOOL ConfigApp::UpdateSubscriberConfig(IN IMS_SINT32 nCpi)
{
    const ISubscriberConfig* piSubsConfig =
            Engine::GetConfiguration()->GetSubscriberConfig(GetSlotId());

    if (piSubsConfig != IMS_NULL)
    {
        IConfigurable* piConfigurable = piSubsConfig->GetConfigurable();

        if (piConfigurable != IMS_NULL)
        {
            if (!piConfigurable->Update(nCpi))
            {
                IMS_TRACE_E(0, "UpdateSubscriberConfig :: [%d] failed", nCpi, 0, 0);
                return IMS_FALSE;
            }

            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}
