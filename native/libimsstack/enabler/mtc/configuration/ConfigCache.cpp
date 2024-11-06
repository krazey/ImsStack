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
#include "ImsMap.h"
#include "ImsTypeDef.h"
#include "ServiceTrace.h"
#include "configuration/ConfigCache.h"
#include "configuration/ConfigDef.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
ConfigCache::ConfigCache() :
        m_objBooleanCache(ImsMap<Feature, IMS_BOOL>()),
        m_objIntegerCache(ImsMap<Feature, IMS_SINT32>()),
        m_objStringCache(ImsMap<Feature, AString>())
{
    IMS_TRACE_I("+ConfigCache", 0, 0, 0);
}

PUBLIC VIRTUAL ConfigCache::~ConfigCache()
{
    IMS_TRACE_I("~ConfigCache", 0, 0, 0);
    m_objBooleanCache.Clear();
    m_objIntegerCache.Clear();
    m_objStringCache.Clear();
}

PUBLIC
void ConfigCache::PutCache(IN Feature eFeature, IN IMS_BOOL bValue)
{
    IMS_TRACE_I("PutCache. Feature = [%d] value=[%s]", eFeature, _TRACE_B_(bValue), 0);

    m_objBooleanCache.SetValue(eFeature, bValue);
}

PUBLIC
void ConfigCache::PutCache(IN Feature eFeature, IN IMS_SINT32 nValue)
{
    IMS_TRACE_I("PutCache. Feature = [%d] value=[%d]", eFeature, nValue, 0);

    m_objIntegerCache.SetValue(eFeature, nValue);
}

PUBLIC
void ConfigCache::PutCache(IN Feature eFeature, const IN AString& strValue)
{
    IMS_TRACE_I("PutCache. Feature = [%d] value=[%s]", eFeature, strValue.GetStr(), 0);

    m_objStringCache.SetValue(eFeature, strValue);
}

PUBLIC
IMS_BOOL ConfigCache::ResetCache(IN Feature eFeature)
{
    IMS_TRACE_I("ResetCache. feature type = [%d]", eFeature, 0, 0);

    IMS_SLONG nTempIndex = m_objIntegerCache.GetIndexOfKey(eFeature);
    if (nTempIndex > -1)
    {
        m_objIntegerCache.RemoveAt(nTempIndex);
        return IMS_TRUE;
    }

    nTempIndex = m_objBooleanCache.GetIndexOfKey(eFeature);
    if (nTempIndex > -1)
    {
        m_objBooleanCache.RemoveAt(nTempIndex);
        return IMS_TRUE;
    }

    nTempIndex = m_objStringCache.GetIndexOfKey(eFeature);
    if (nTempIndex > -1)
    {
        m_objStringCache.RemoveAt(nTempIndex);
        return IMS_TRUE;
    }
    return IMS_FALSE;
}

PUBLIC
IMS_BOOL ConfigCache::GetBooleanCache(IN Feature eFeature) const
{
    IMS_BOOL bValue = m_objBooleanCache.GetValue(eFeature);

    IMS_TRACE_I("GetBooleanCache. Feature[%d] value[%s]", eFeature, _TRACE_B_(bValue), 0);
    return bValue;
}

PUBLIC
IMS_SINT32 ConfigCache::GetIntegerCache(IN Feature eFeature) const
{
    IMS_SINT32 nValue = m_objIntegerCache.GetValue(eFeature);

    IMS_TRACE_I("GetIntegerCache. Feature[%d] value[%d]", eFeature, nValue, 0);
    return nValue;
}

PUBLIC
const AString ConfigCache::GetStringCache(IN Feature eFeature) const
{
    AString strValue = m_objStringCache.GetValue(eFeature);

    IMS_TRACE_I("GetStringCache. Feature[%d] value[%s]", eFeature, strValue.GetStr(), 0);
    return strValue;
}

PUBLIC
IMS_BOOL ConfigCache::HasBooleanCache(IN Feature eFeature) const
{
    if (m_objBooleanCache.GetIndexOfKey(eFeature) > -1)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL ConfigCache::HasIntegerCache(IN Feature eFeature) const
{
    if (m_objIntegerCache.GetIndexOfKey(eFeature) > -1)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL ConfigCache::HasStringCache(IN Feature eFeature) const
{
    if (m_objStringCache.GetIndexOfKey(eFeature) > -1)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL ConfigCache::IsEmpty() const
{
    if (m_objIntegerCache.IsEmpty() && m_objBooleanCache.IsEmpty() && m_objStringCache.IsEmpty())
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}
