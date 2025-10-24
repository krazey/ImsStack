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
#include "ConfigUce.h"
#include "ImsServiceConfig.h"

static const ImsServiceIdentifier SERVICE_IDENTIFIERS[] = {
        {ImsAppId::MTC, ImsServiceId::MTC},
        {ImsAppId::MTS, ImsServiceId::MTS},
        {ImsAppId::UCE, ImsServiceId::UCE}
};

static const ImsServiceIdentifier EMERGENCY_SERVICE_IDENTIFIERS[] = {
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
        default:
            return IMS_NULL;
    }
}

PUBLIC GLOBAL ImsServiceProfile ImsServiceConfig::GetServiceProfile()
{
    ImsServiceProfile objProfile = {0, IMS_NULL};

    objProfile.nCount = sizeof(SERVICE_IDENTIFIERS) / sizeof(SERVICE_IDENTIFIERS[0]);
    objProfile.pServiceIds = SERVICE_IDENTIFIERS;

    return objProfile;
}

PUBLIC GLOBAL ImsServiceProfile ImsServiceConfig::GetEmergencyServiceProfile()
{
    ImsServiceProfile objProfile = {0, IMS_NULL};

    objProfile.nCount =
            sizeof(EMERGENCY_SERVICE_IDENTIFIERS) / sizeof(EMERGENCY_SERVICE_IDENTIFIERS[0]);
    objProfile.pServiceIds = EMERGENCY_SERVICE_IDENTIFIERS;

    return objProfile;
}

PUBLIC GLOBAL ImsList<ImsServiceName> ImsServiceConfig::GetServiceNames(
        IN const ImsServiceProfile& objProfile)
{
    ImsList<ImsServiceName> objServiceNames;

    for (IMS_SINT32 i = 0; i < objProfile.nCount; i++)
    {
        const ImsServiceIdentifier* pId = &(objProfile.pServiceIds[i]);
        ImsServiceName objName(GetAppName(pId->eAppId), GetServiceName(pId->eServiceId));
        objServiceNames.Append(objName);
    }

    return objServiceNames;
}
