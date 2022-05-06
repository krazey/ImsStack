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
#include "DeviceConfig.h"
#include "IMSStrLib.h"
#include "ServiceMemory.h"
#include "SystemConfig.h"

PRIVATE GLOBAL IMS_SINT32 SystemConfig::s_nGlobalConfigs = 0;

PRIVATE GLOBAL AString SystemConfig::s_strPackageName = AString::ConstNull();

PUBLIC
SystemConfig::SystemConfig() :
        m_nSlotId(0),
        m_strOperator(AString::ConstNull()),
        m_strCountry(AString::ConstNull()),
        m_strEnablerType(AString::ConstNull()),
        m_nExtraInfo(EXTRA_INFO_NONE),
        m_nFeatures(0),
        m_nServiceFeatures(0)
{
}

PUBLIC
SystemConfig::SystemConfig(IN const __SystemConfig* pConfig)
{
    if (pConfig != IMS_NULL)
    {
        m_nSlotId = pConfig->nSlotId;

        m_strOperator = static_cast<const IMS_CHAR*>(pConfig->acOperator);
        m_strCountry = static_cast<const IMS_CHAR*>(pConfig->acCountry);

        m_strEnablerType = static_cast<const IMS_CHAR*>(pConfig->acEnablerType);
        m_nExtraInfo = pConfig->nExtraInfo;

        m_nFeatures = pConfig->nFeatures;
        m_nServiceFeatures = pConfig->nServiceFeatures;
    }
    else
    {
        m_nSlotId = 0;

        m_strOperator = AString::ConstNull();
        m_strCountry = AString::ConstNull();

        m_strEnablerType = AString::ConstNull();
        m_nExtraInfo = EXTRA_INFO_NONE;

        m_nFeatures = 0;
        m_nServiceFeatures = 0;
    }
}

PUBLIC
SystemConfig::SystemConfig(IN const SystemConfig& other) :
        m_nSlotId(other.m_nSlotId),
        m_strOperator(other.m_strOperator),
        m_strCountry(other.m_strCountry),
        m_strEnablerType(other.m_strEnablerType),
        m_nExtraInfo(other.m_nExtraInfo),
        m_nFeatures(other.m_nFeatures),
        m_nServiceFeatures(other.m_nServiceFeatures)
{
}

PUBLIC
SystemConfig::~SystemConfig() {}

PUBLIC
SystemConfig& SystemConfig::operator=(IN const SystemConfig& other)
{
    if (this != &other)
    {
        m_nSlotId = other.m_nSlotId;

        m_strOperator = other.m_strOperator;
        m_strCountry = other.m_strCountry;

        m_strEnablerType = other.m_strEnablerType;
        m_nExtraInfo = other.m_nExtraInfo;

        m_nFeatures = other.m_nFeatures;
        m_nServiceFeatures = other.m_nServiceFeatures;
    }

    return (*this);
}

PUBLIC
IMS_BOOL SystemConfig::Equals(IN const SystemConfig& other) const
{
    return (m_nSlotId == other.m_nSlotId) && m_strOperator.EqualsIgnoreCase(other.m_strOperator) &&
            m_strCountry.EqualsIgnoreCase(other.m_strCountry) &&
            m_strEnablerType.EqualsIgnoreCase(other.m_strEnablerType) &&
            (m_nExtraInfo == other.m_nExtraInfo) && (m_nFeatures == other.m_nFeatures) &&
            (m_nServiceFeatures == other.m_nServiceFeatures);
}

PUBLIC
AString SystemConfig::ToString() const
{
    AString strSystemConfig;

    strSystemConfig.Sprintf("[ SystemConfig :: slotId=%d, op=%s, co=%s"
                            ", enablerType=%s, extraInfo=%08x"
                            ", features=%08x, serviceFeatures=%08x ]",
            m_nSlotId, m_strOperator.GetStr(), m_strCountry.GetStr(), m_strEnablerType.GetStr(),
            m_nExtraInfo, m_nFeatures, m_nServiceFeatures);

    return strSystemConfig;
}

/**
 * The caller MUTS free the memory of the pointer variable which returns from this method.
 */
PUBLIC
__SystemConfig* SystemConfig::ToSystemConfig() const
{
    __SystemConfig* pConfig = new __SystemConfig;

    pConfig->nSlotId = m_nSlotId;

    IMS_StrCpy(pConfig->acOperator, IMS_SC_SIZE_16 + 1, m_strOperator.GetStr());
    IMS_StrCpy(pConfig->acCountry, IMS_SC_SIZE_8 + 1, m_strCountry.GetStr());

    IMS_StrCpy(pConfig->acEnablerType, IMS_SC_SIZE_16 + 1, m_strEnablerType.GetStr());
    pConfig->nExtraInfo = m_nExtraInfo;

    pConfig->nFeatures = m_nFeatures;
    pConfig->nServiceFeatures = m_nServiceFeatures;

    return pConfig;
}

PUBLIC GLOBAL const AString& SystemConfig::GetPackageName()
{
    return s_strPackageName;
}

PUBLIC GLOBAL IMS_SINT32 SystemConfig::GetMaxSimSlot()
{
    return DeviceConfig::GetActiveModemCount();
}

PUBLIC GLOBAL IMS_BOOL SystemConfig::IsMultiImsEnabled()
{
    // As a default, single IMS is required on dual SIM environment.
    return (s_nGlobalConfigs & CONFIG_MULTI_IMS) != 0;
}

PUBLIC GLOBAL IMS_BOOL SystemConfig::IsMultiImsEnabledOnDssv()
{
    // DSSV-DV
    return IMS_FALSE;
}

PUBLIC GLOBAL IMS_BOOL SystemConfig::IsMultiLteEnabled()
{
    // LTE+LTE DSDS
    return (s_nGlobalConfigs & CONFIG_MULTI_LTE) != 0;
}

PUBLIC GLOBAL IMS_BOOL SystemConfig::IsMultiSimEnabled()
{
    return GetMaxSimSlot() > 1;
}

PUBLIC GLOBAL IMS_BOOL SystemConfig::IsOperatorChanged(
        IN const SystemConfig* pOldConfig, IN const SystemConfig* pNewConfig)
{
    if ((pOldConfig == IMS_NULL) && (pNewConfig == IMS_NULL))
    {
        return IMS_FALSE;
    }

    if (pOldConfig == IMS_NULL)
    {
        return pNewConfig->GetOperator().GetLength() > 0;
    }
    else if (pNewConfig == IMS_NULL)
    {
        return pOldConfig->GetOperator().GetLength() > 0;
    }

    if ((pNewConfig->GetOperator().GetLength() == 0) &&
            (pOldConfig->GetOperator().GetLength() == 0))
    {
        return IMS_FALSE;
    }

    if (pNewConfig->GetOperator().Equals(pOldConfig->GetOperator()))
    {
        if (pNewConfig->GetEnablerType().Equals("global"))
        {
            return !pNewConfig->GetCountry().Equals(pOldConfig->GetCountry());
        }

        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL SystemConfig::IsServiceFeatureChanged(
        IN const SystemConfig* pOldConfig, IN const SystemConfig* pNewConfig)
{
    if ((pOldConfig == IMS_NULL) || (pNewConfig == IMS_NULL))
    {
        return IMS_FALSE;
    }

    return pNewConfig->GetServiceFeatures() != pOldConfig->GetServiceFeatures();
}

PUBLIC GLOBAL IMS_BOOL SystemConfig::IsDdsChanged(
        IN const SystemConfig* pOldConfig, IN const SystemConfig* pNewConfig)
{
    if ((pOldConfig == IMS_NULL) || (pNewConfig == IMS_NULL))
    {
        return IMS_FALSE;
    }

    return (pNewConfig->IsDds() && !pOldConfig->IsDds()) ||
            (!pNewConfig->IsDds() && pOldConfig->IsDds());
}

PUBLIC GLOBAL IMS_BOOL SystemConfig::IsSimMobilityChanged(
        IN const SystemConfig* pOldConfig, IN const SystemConfig* pNewConfig)
{
    if ((pOldConfig == IMS_NULL) || (pNewConfig == IMS_NULL))
    {
        return IMS_FALSE;
    }

    return (pNewConfig->IsSimMobilityEnabled() && !pOldConfig->IsSimMobilityEnabled()) ||
            (!pNewConfig->IsSimMobilityEnabled() && pOldConfig->IsSimMobilityEnabled());
}

PRIVATE
IMS_BOOL SystemConfig::IsExtraInfoSet(IN IMS_SINT32 nExtraInfo) const
{
    return (m_nExtraInfo & nExtraInfo) != 0;
}

PRIVATE GLOBAL void SystemConfig::CacheGlobalConfigs()
{
    s_nGlobalConfigs = 0;

    s_strPackageName = "com.android.imsstack";
}

PRIVATE GLOBAL void SystemConfig::UpdateGlobalConfigsOnFeatureChanged()
{
    // DSDV-SV
    // s_nGlobalConfigs |= CONFIG_MULTI_IMS;
}
