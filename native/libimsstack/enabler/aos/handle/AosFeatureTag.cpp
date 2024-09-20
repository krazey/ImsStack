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
#include "ServiceTrace.h"

#include "ImsAosParameter.h"

#include "handle/AosFeatureTag.h"

__IMS_TRACE_TAG_AOS__;

PUBLIC
AosFeatureTag::AosFeatureTag(
        IN const AString& strName, IN const AString& strValue /* = AString::ConstNull() */,
        IN IMS_UINT32 nType /* = 0 */, IN IMS_UINT32 nOption /* = OPTION_HEADER_PARAMETER */
        ) :
        m_strName(strName),
        m_strValue(strValue),
        m_nType(nType),
        m_nOption(nOption)
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : AosFeatureTag = %" PFLS_u "/%" PFLS_x, sizeof(AosFeatureTag),
            this, 0);
}

PUBLIC VIRTUAL AosFeatureTag::~AosFeatureTag()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : AosFeatureTag = %" PFLS_u "/%" PFLS_x, sizeof(AosFeatureTag),
            this, 0);
}

PUBLIC
void AosFeatureTag::SetFeatureTag(
        IN const AString& strName, IN const AString& strValue /* = AString::ConstNull() */)
{
    m_strName = strName;
    m_strValue = strValue;
}

PUBLIC
IMS_BOOL AosFeatureTag::Equals(IN AosFeatureTag* pFeatureTag)
{
    IMS_BOOL bResult = IMS_FALSE;

    if (m_strName.EqualsIgnoreCase(pFeatureTag->GetName()) &&
            m_strValue.EqualsIgnoreCase(pFeatureTag->GetValue()))
    {
        bResult = IMS_TRUE;
    }

    return bResult;
}

PUBLIC
IMS_BOOL AosFeatureTag::Equals(
        IN const AString& strName, IN const AString& strValue /*= AString::ConstNull() */)
{
    IMS_BOOL bResult = IMS_FALSE;

    if (m_strName.EqualsIgnoreCase(strName) && m_strValue.EqualsIgnoreCase(strValue))
    {
        bResult = IMS_TRUE;
    }

    return bResult;
}

PUBLIC
AString& AosFeatureTag::GetName()
{
    return m_strName;
}

PUBLIC
AString& AosFeatureTag::GetValue()
{
    return m_strValue;
}

PUBLIC
IMS_UINT32 AosFeatureTag::GetType()
{
    return m_nType;
}

PUBLIC
IMS_UINT32 AosFeatureTag::GetOption()
{
    return m_nOption;
}

PUBLIC
AosFeatureTagList::AosFeatureTagList() :
        m_nFeatures(ImsAosFeature::NONE),
        m_nUnavailableFeatures(ImsAosFeature::NONE)
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : AosFeatureTagList = %" PFLS_u "/%" PFLS_x,
            sizeof(AosFeatureTagList), this, 0);

    m_objFeatureTagList.Clear();
}

PUBLIC VIRTUAL AosFeatureTagList::~AosFeatureTagList()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : AosFeatureTagList = %" PFLS_u "/%" PFLS_x,
            sizeof(AosFeatureTagList), this, 0);

    Clear();
}

PUBLIC
IMS_BOOL AosFeatureTagList::AddFeatureTag(IN const AString& strName,
        IN const AString& strValue /* = AString::ConstNull() */, IN IMS_UINT32 nType /*= 0*/,
        IN IMS_UINT32 nOption /* = AosFeatureTag::OPTION_HEADER_PARAMETER */)
{
    IMS_TRACE_I("AddFeatureTag :: name(%s) , value(%s)", strName.GetStr(), strValue.GetStr(), 0);

    for (IMS_UINT32 i = 0; i < m_objFeatureTagList.GetSize(); ++i)
    {
        AosFeatureTag* pFeatureTag = m_objFeatureTagList.GetAt(i);

        if (pFeatureTag->Equals(strName, strValue))
        {
            IMS_TRACE_D("AddFeatureTag :: already exist", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    AosFeatureTag* pFeatureTag = new AosFeatureTag(strName, strValue, nType, nOption);
    m_objFeatureTagList.Append(pFeatureTag);

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL AosFeatureTagList::RemoveFeatureTag(
        IN const AString& strName, IN const AString& strValue /* = AString::ConstNull()*/)
{
    IMS_TRACE_I("RemoveFeatureTag :: name(%s) , value(%s)", strName.GetStr(), strValue.GetStr(), 0);

    for (IMS_UINT32 i = 0; i < m_objFeatureTagList.GetSize(); ++i)
    {
        AosFeatureTag* pFeatureTag = m_objFeatureTagList.GetAt(i);

        if (pFeatureTag->Equals(strName, strValue))
        {
            delete pFeatureTag;

            m_objFeatureTagList.RemoveAt(i);

            return IMS_TRUE;
        }
    }

    IMS_TRACE_D("RemoveFeatureTag :: not exist", 0, 0, 0);
    return IMS_FALSE;
}

PUBLIC
void AosFeatureTagList::AddFeature(IN IMS_UINT32 nFeature)
{
    m_nFeatures |= nFeature;
}

PUBLIC
void AosFeatureTagList::RemoveFeature(IN IMS_UINT32 nFeature)
{
    m_nFeatures &= ~(nFeature);
}

PUBLIC
void AosFeatureTagList::AddUnavailableFeature(IN IMS_UINT32 nFeature)
{
    m_nUnavailableFeatures |= nFeature;
}

PUBLIC
void AosFeatureTagList::RemoveUnavailableFeature(IN IMS_UINT32 nFeature)
{
    m_nUnavailableFeatures &= ~(nFeature);
}

PUBLIC
IMS_UINT32 AosFeatureTagList::GetFeatures()
{
    return m_nFeatures;
}

PUBLIC
IMS_UINT32 AosFeatureTagList::GetUnavailableFeatures()
{
    return m_nUnavailableFeatures;
}

PUBLIC
void AosFeatureTagList::ClearFeatures()
{
    m_nFeatures = ImsAosFeature::NONE;
    m_nUnavailableFeatures = ImsAosFeature::NONE;
}

PUBLIC
void AosFeatureTagList::ClearFeatureTags()
{
    if (!m_objFeatureTagList.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objFeatureTagList.GetSize(); ++i)
        {
            AosFeatureTag* pFeatureTag = m_objFeatureTagList.GetAt(i);

            if (pFeatureTag != IMS_NULL)
            {
                delete pFeatureTag;
            }
        }

        m_objFeatureTagList.Clear();
    }
}

PUBLIC
void AosFeatureTagList::Clear()
{
    IMS_TRACE_I("Clear", 0, 0, 0);

    ClearFeatureTags();
    ClearFeatures();
}

PUBLIC
IMS_BOOL AosFeatureTagList::Equals(IN AosFeatureTagList& objTargetList)
{
    if (GetFeatures() != objTargetList.GetFeatures())
    {
        IMS_TRACE_I("Equals :: Feature is different", 0, 0, 0);
        return IMS_FALSE;
    }

    if (m_objFeatureTagList.GetSize() != objTargetList.GetSize())
    {
        IMS_TRACE_I("Equals :: size is different", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_BOOL bResult = IMS_TRUE;

    for (IMS_UINT32 i = 0; i < m_objFeatureTagList.GetSize(); ++i)
    {
        AosFeatureTag* pFeatureTag = m_objFeatureTagList.GetAt(i);

        IMS_BOOL bEqual = IMS_FALSE;

        for (IMS_UINT32 j = 0; j < objTargetList.GetSize(); ++j)
        {
            AosFeatureTag* pTargetFeatureTag = objTargetList.GetAt(j);

            if (pFeatureTag->Equals(pTargetFeatureTag))
            {
                bEqual = IMS_TRUE;
                break;
            }
        }

        if (!bEqual)
        {
            bResult = IMS_FALSE;
            break;
        }
    }

    IMS_TRACE_I("Equals :: (%s)", _TRACE_B_(bResult), 0, 0);

    return bResult;
}

PUBLIC
void AosFeatureTagList::Copy(IN AosFeatureTagList& objSourceList)
{
    IMS_TRACE_I("Copy", 0, 0, 0);

    Clear();

    for (IMS_UINT32 i = 0; i < objSourceList.GetSize(); ++i)
    {
        AosFeatureTag* pFeatureTag = objSourceList.GetAt(i);
        AddFeatureTag(pFeatureTag->GetName(), pFeatureTag->GetValue(), pFeatureTag->GetType(),
                pFeatureTag->GetOption());
    }

    CopyFeatures(objSourceList);
}

PUBLIC
void AosFeatureTagList::CopyFeatures(IN const AosFeatureTagList& objSourceList)
{
    m_nFeatures = objSourceList.m_nFeatures;
    m_nUnavailableFeatures = objSourceList.m_nUnavailableFeatures;
}

PUBLIC
void AosFeatureTagList::CopyFeatureTags(IN ImsList<ImsAosFeatureTag*>& objFeatureTag)
{
    ClearFeatureTags();

    for (IMS_UINT32 i = 0; i < objFeatureTag.GetSize(); ++i)
    {
        AosFeatureTag* pFeatureTag = new AosFeatureTag(
                objFeatureTag.GetAt(i)->GetName(), objFeatureTag.GetAt(i)->GetValue());
        m_objFeatureTagList.Append(pFeatureTag);
    }
}

PUBLIC
IMS_UINT32 AosFeatureTagList::GetSize()
{
    return m_objFeatureTagList.GetSize();
}

PUBLIC
AosFeatureTag* AosFeatureTagList::GetAt(IN IMS_UINT32 nIndex)
{
    return m_objFeatureTagList.GetAt(nIndex);
}

PUBLIC
IMS_BOOL AosFeatureTagList::HasUnavailableFeature(IN IMS_UINT32 nFeature) const
{
    return (m_nUnavailableFeatures & nFeature) > 0;
}

PUBLIC
IMS_BOOL AosFeatureTagList::HasFeature(IN IMS_UINT32 nFeature) const
{
    return (m_nFeatures & nFeature) > 0;
}

PUBLIC
IMS_BOOL AosFeatureTagList::HasFeatureTag(
        IN const AString& strName, IN const AString& strValue /* = AString::ConstNull() */) const
{
    IMS_TRACE_I("HasFeatureTag :: name(%s) , value(%s)", strName.GetStr(), strValue.GetStr(), 0);

    for (IMS_UINT32 i = 0; i < m_objFeatureTagList.GetSize(); ++i)
    {
        AosFeatureTag* pFeatureTag = m_objFeatureTagList.GetAt(i);

        if (pFeatureTag->Equals(strName, strValue))
        {
            return IMS_TRUE;
        }
    }

    IMS_TRACE_D("HasFeatureTag :: not exist", 0, 0, 0);

    return IMS_FALSE;
}

PUBLIC
void AosFeatureTagList::PrintFeatureTagList()
{
    AString strLog;
    for (IMS_UINT32 i = 0; i < m_objFeatureTagList.GetSize(); ++i)
    {
        AosFeatureTag* pFeatureTag = m_objFeatureTagList.GetAt(i);

        strLog.Append("[");
        strLog.Append(pFeatureTag->GetName().GetStr());
        strLog.Append("/");
        strLog.Append(pFeatureTag->GetValue().GetStr());
        strLog.Append("]");
    }

    IMS_TRACE_I("PrintFeatureTagList :: (%s)", strLog.GetStr(), 0, 0);
}