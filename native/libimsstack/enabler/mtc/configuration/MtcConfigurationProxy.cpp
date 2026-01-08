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

#include "AString.h"
#include "ICarrierConfig.h"
#include "ImsVector.h"
#include "ServiceConfig.h"
#include "ServiceThread.h"
#include "ServiceTrace.h"
#include "configuration/ConfigCache.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include <memory>
#include <optional>

__IMS_TRACE_TAG_COM_MTC__;

LOCAL const IMS_CHAR* SUFFIX_BOOL = "_bool";
LOCAL const IMS_CHAR* SUFFIX_INT = "_int";
LOCAL const IMS_CHAR* SUFFIX_STRING = "_string";
LOCAL const IMS_CHAR* SUFFIX_INT_ARRAY = "_int_array";
LOCAL const IMS_CHAR* SUFFIX_STRING_ARRAY = "_string_array";

PUBLIC
MtcConfigurationProxy::MtcConfigurationProxy() :
        m_pCache(IMS_NULL),
        m_piCc(ConfigService::GetConfigService()->GetCarrierConfig(
                ThreadService::GetCurrentSlotId(IMS_SLOT_0)))
{
    IMS_ASSERT(m_piCc != IMS_NULL);
}

PUBLIC VIRTUAL MtcConfigurationProxy::~MtcConfigurationProxy() {}

PUBLIC VIRTUAL IMS_BOOL MtcConfigurationProxy::GetBoolean(IN const IMS_CHAR* pszKey) const
{
    IMS_ASSERT(ValidateSuffix(pszKey, SUFFIX_BOOL));
    if (GetCachedValue(pszKey))
    {
        return std::get<IMS_BOOL>(*GetCachedValue(pszKey));
    }

    return m_piCc->GetBoolean(pszKey);
}

PUBLIC VIRTUAL IMS_SINT32 MtcConfigurationProxy::GetInt(IN const IMS_CHAR* pszKey) const
{
    IMS_ASSERT(ValidateSuffix(pszKey, SUFFIX_INT));
    if (GetCachedValue(pszKey))
    {
        return std::get<IMS_SINT32>(*GetCachedValue(pszKey));
    }

    return m_piCc->GetInt(pszKey);
}

PUBLIC VIRTUAL AString MtcConfigurationProxy::GetString(IN const IMS_CHAR* pszKey) const
{
    IMS_ASSERT(ValidateSuffix(pszKey, SUFFIX_STRING));
    if (GetCachedValue(pszKey))
    {
        return std::get<AString>(*GetCachedValue(pszKey));
    }

    return m_piCc->GetString(pszKey);
}

PUBLIC VIRTUAL ImsVector<IMS_SINT32> MtcConfigurationProxy::GetIntArray(
        IN const IMS_CHAR* pszKey) const
{
    IMS_ASSERT(ValidateSuffix(pszKey, SUFFIX_INT_ARRAY));
    return m_piCc->GetIntArray(pszKey);
}

PUBLIC VIRTUAL ImsVector<AString> MtcConfigurationProxy::GetStringArray(
        IN const IMS_CHAR* pszKey) const
{
    IMS_ASSERT(ValidateSuffix(pszKey, SUFFIX_STRING_ARRAY));
    return m_piCc->GetStringArray(pszKey);
}

PUBLIC VIRTUAL IMS_BOOL MtcConfigurationProxy::Contains(
        IN const IMS_CHAR* pszKey, IN IMS_SINT32 nValue) const
{
    return m_piCc->GetIntArray(pszKey).Contains(nValue);
}

PUBLIC VIRTUAL IMS_BOOL MtcConfigurationProxy::Contains(
        IN const IMS_CHAR* pszKey, IN const IMS_CHAR* pszValue) const
{
    return m_piCc->GetStringArray(pszKey).Contains(pszValue);
}

PUBLIC VIRTUAL IMS_SINT32 MtcConfigurationProxy::GetIntFromArray(
        IN const IMS_CHAR* pszKey, IN const IMS_UINT32 nIndex) const
{
    IMS_ASSERT(ValidateSuffix(pszKey, SUFFIX_INT_ARRAY));
    return m_piCc->GetIntArray(pszKey).GetAt(nIndex);
}

PUBLIC VIRTUAL AString MtcConfigurationProxy::GetStringFromArray(
        IN const IMS_CHAR* pszKey, IN const IMS_UINT32 nIndex) const
{
    IMS_ASSERT(ValidateSuffix(pszKey, SUFFIX_STRING_ARRAY));
    return m_piCc->GetStringArray(pszKey).GetAt(nIndex);
}

PUBLIC VIRTUAL void MtcConfigurationProxy::PutCache(IN const IMS_CHAR* pszKey, IN IMS_BOOL bValue)
{
    PutCache(pszKey, ConfigValue(bValue));
}

PUBLIC VIRTUAL void MtcConfigurationProxy::PutCache(IN const IMS_CHAR* pszKey, IN IMS_SINT32 nValue)
{
    PutCache(pszKey, ConfigValue(nValue));
}

PUBLIC VIRTUAL void MtcConfigurationProxy::PutCache(
        IN const IMS_CHAR* pszKey, IN const IMS_CHAR* pszValue)
{
    PutCache(pszKey, ConfigValue(pszValue));
}

PUBLIC VIRTUAL void MtcConfigurationProxy::OnRegistrationRefreshed()
{
    IMS_TRACE_I("OnRegistrationRefreshed", 0, 0, 0);
    m_pCache.reset();
}

PRIVATE
std::optional<ConfigValue> MtcConfigurationProxy::GetCachedValue(const IMS_CHAR* pszKey) const
{
    if (m_pCache)
    {
        return m_pCache->GetCache(pszKey);
    }
    return std::nullopt;
}

PRIVATE
void MtcConfigurationProxy::PutCache(IN const IMS_CHAR* pszKey, IN const ConfigValue& objValue)
{
    if (m_pCache == IMS_NULL)
    {
        m_pCache = std::make_unique<ConfigCache>();
    }
    m_pCache->PutCache(pszKey, objValue);
}

PRIVATE
IMS_BOOL MtcConfigurationProxy::ValidateSuffix(
        const IMS_CHAR* pszKey, const IMS_CHAR* pszSuffix) const
{
    IMS_SIZE_T nKeyLen = std::strlen(pszKey);
    IMS_SIZE_T nSuffixLen = std::strlen(pszSuffix);

    if (nKeyLen < nSuffixLen)
    {
        return IMS_FALSE;
    }

    return std::strcmp(pszKey + nKeyLen - nSuffixLen, pszSuffix) == 0;
}
