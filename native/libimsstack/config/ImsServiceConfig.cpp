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
#include "ConfigMtc.h"
#include "ConfigMts.h"
#include "ConfigSipDelegate.h"
#include "ConfigUce.h"
#include "ImsServiceConfig.h"

LOCAL ImsServiceIdentifier s_objServiceIdentifiers[] = {
        {ImsAppId::MTC,          ImsServiceId::MTC         },
        {ImsAppId::MTS,          ImsServiceId::MTS         },
        {ImsAppId::UCE,          ImsServiceId::UCE         },
        {ImsAppId::SIP_DELEGATE, ImsServiceId::SIP_DELEGATE}
};

LOCAL ImsServiceIdentifier s_objEmergencyServiceIdentifiers[] = {
        {ImsAppId::MTC, ImsServiceId::MTC_EMERGENCY},
        {ImsAppId::MTS, ImsServiceId::MTS_EMERGENCY},
};

PUBLIC GLOBAL ImsAppId ImsServiceConfig::GetAppId(IN ImsServiceId eServiceId)
{
    switch (eServiceId)
    {
        case ImsServiceId::MTC:  // FALL-THROUGH
        case ImsServiceId::MTC_EMERGENCY:
            return ImsAppId::MTC;
        case ImsServiceId::MTS:  // FALL-THROUGH
        case ImsServiceId::MTS_EMERGENCY:
            return ImsAppId::MTS;
        case ImsServiceId::UCE:
            return ImsAppId::UCE;
        case ImsServiceId::SIP_DELEGATE:
            return ImsAppId::SIP_DELEGATE;
        default:
            return ImsAppId::UNKNOWN;
    }
}

PUBLIC GLOBAL AString ImsServiceConfig::GetAppName(IN ImsAppId eAppId)
{
    switch (eAppId)
    {
        case ImsAppId::MTC:
            return AString(ConfigMtc::APP_NAME);
        case ImsAppId::MTS:
            return AString(ConfigMts::APP_NAME);
        case ImsAppId::UCE:
            return AString(ConfigUce::APP_NAME);
        case ImsAppId::SIP_DELEGATE:
            return AString(ConfigSipDelegate::APP_NAME);
        default:
            return AString::ConstNull();
    }
}

PUBLIC GLOBAL AString ImsServiceConfig::GetServiceName(IN ImsServiceId eServiceId)
{
    switch (eServiceId)
    {
        case ImsServiceId::MTC:
            return AString(ConfigMtc::SERVICE_NAME);
        case ImsServiceId::MTC_EMERGENCY:
            return AString(ConfigMtc::EMERGENCY_SERVICE_NAME);
        case ImsServiceId::MTS:
            return AString(ConfigMts::SERVICE_NAME);
        case ImsServiceId::MTS_EMERGENCY:
            return AString(ConfigMts::EMERGENCY_SERVICE_NAME);
        case ImsServiceId::UCE:
            return AString(ConfigUce::SERVICE_NAME);
        case ImsServiceId::SIP_DELEGATE:
            return AString(ConfigSipDelegate::SERVICE_NAME);
        default:
            return AString::ConstNull();
    }
}

PUBLIC GLOBAL const IMS_CHAR* ImsServiceConfig::GetAppNameC(IN ImsAppId eAppId)
{
    switch (eAppId)
    {
        case ImsAppId::MTC:
            return ConfigMtc::APP_NAME;
        case ImsAppId::MTS:
            return ConfigMts::APP_NAME;
        case ImsAppId::UCE:
            return ConfigUce::APP_NAME;
        case ImsAppId::SIP_DELEGATE:
            return ConfigSipDelegate::APP_NAME;
        default:
            return IMS_NULL;
    }
}

PUBLIC GLOBAL const IMS_CHAR* ImsServiceConfig::GetServiceNameC(IN ImsServiceId eServiceId)
{
    switch (eServiceId)
    {
        case ImsServiceId::MTC:
            return ConfigMtc::SERVICE_NAME;
        case ImsServiceId::MTC_EMERGENCY:
            return ConfigMtc::EMERGENCY_SERVICE_NAME;
        case ImsServiceId::MTS:
            return ConfigMts::SERVICE_NAME;
        case ImsServiceId::MTS_EMERGENCY:
            return ConfigMts::EMERGENCY_SERVICE_NAME;
        case ImsServiceId::UCE:
            return ConfigUce::SERVICE_NAME;
        case ImsServiceId::SIP_DELEGATE:
            return ConfigSipDelegate::SERVICE_NAME;
        default:
            return IMS_NULL;
    }
}

PUBLIC GLOBAL ImsServiceProfile ImsServiceConfig::GetServiceProfile()
{
    ImsServiceProfile objProfile = {0, IMS_NULL};

    objProfile.nCount = sizeof(s_objServiceIdentifiers) / sizeof(s_objServiceIdentifiers[0]);
    objProfile.pServiceIds = s_objServiceIdentifiers;

    return objProfile;
}

PUBLIC GLOBAL ImsServiceProfile ImsServiceConfig::GetEmergencyServiceProfile()
{
    ImsServiceProfile objProfile = {0, IMS_NULL};

    objProfile.nCount =
            sizeof(s_objEmergencyServiceIdentifiers) / sizeof(s_objEmergencyServiceIdentifiers[0]);
    objProfile.pServiceIds = s_objEmergencyServiceIdentifiers;

    return objProfile;
}

PUBLIC GLOBAL IMSList<ImsServiceName> ImsServiceConfig::GetServiceNames(
        IN const ImsServiceProfile& objProfile)
{
    IMSList<ImsServiceName> objServiceNames;

    for (IMS_SINT32 i = 0; i < objProfile.nCount; i++)
    {
        const ImsServiceIdentifier* pId = &(objProfile.pServiceIds[i]);
        ImsServiceName objName(GetAppName(pId->eAppId), GetServiceName(pId->eServiceId));
        objServiceNames.Append(objName);
    }

    return objServiceNames;
}
