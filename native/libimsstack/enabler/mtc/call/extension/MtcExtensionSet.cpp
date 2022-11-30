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

#include "ISipHeader.h"
#include "ImsList.h"
#include "ServiceTrace.h"
#include "call/extension/IMtcExtension.h"
#include "call/extension/MtcExtensionSet.h"
#include "utility/MessageUtil.h"

__IMS_TRACE_TAG_COM_MTC__;

const AString MtcExtensionSet::OPTION_TAG_EARLY_DIALOG_TERMINATED = "199";
const AString MtcExtensionSet::OPTION_TAG_FROM_CHANGE = "from-change";
const AString MtcExtensionSet::OPTION_TAG_HISTORY_INFO = "histinfo";
const AString MtcExtensionSet::OPTION_TAG_PRECONDITION = "precondition";
const AString MtcExtensionSet::OPTION_TAG_REPLACES = "replaces";
const AString MtcExtensionSet::OPTION_TAG_RPR = "100rel";
const AString MtcExtensionSet::OPTION_TAG_SESSION_TIMER = "timer";
const AString MtcExtensionSet::OPTION_TAG_TARGET_DIALOG = "tdialog";

PUBLIC
MtcExtensionSet::MtcExtensionSet(IN const ImsList<IMtcExtension*>& lstExtensions)
{
    for (IMS_UINT32 nIndex = 0; nIndex < lstExtensions.GetSize(); nIndex++)
    {
        IMtcExtension* pExtension = lstExtensions.GetAt(nIndex);
        if (pExtension)
        {
            m_objExtensions.Add(pExtension->GetOptionTag(), pExtension);
        }
    }
}

PUBLIC
MtcExtensionSet::MtcExtensionSet(IN const MtcExtensionSet& objRhs)
{
    CopyFrom(objRhs);
}

PUBLIC VIRTUAL MtcExtensionSet::~MtcExtensionSet()
{
    Clear();
}

PUBLIC
MtcExtensionSet& MtcExtensionSet::operator=(IN const MtcExtensionSet& objRhs)
{
    if (this != &objRhs)
    {
        CopyFrom(objRhs);
    }

    return *this;
}

PUBLIC
IMS_BOOL MtcExtensionSet::IsAvailableOnBoth(IN const AString& strOptionTag) const
{
    IMS_SLONG nIndex = m_objExtensions.GetIndexOfKey(strOptionTag);
    return nIndex >= 0 && m_objExtensions.GetValueAt(nIndex)->IsAvailableOnRemote();
}

PUBLIC
IMS_BOOL MtcExtensionSet::IsAvailableOnLocal(IN const AString& strOptionTag) const
{
    return m_objExtensions.GetIndexOfKey(strOptionTag) >= 0;
}

PUBLIC
IMS_BOOL MtcExtensionSet::IsRequiredOnRemote(IN const AString& strOptionTag) const
{
    IMS_SLONG nIndex = m_objExtensions.GetIndexOfKey(strOptionTag);
    return nIndex >= 0 && m_objExtensions.GetValueAt(nIndex)->IsRequiredOnRemote();
}

PUBLIC
IMS_BOOL MtcExtensionSet::IsSupportRequiredExtensions(IN const IMessage& objMessage) const
{
    ImsList<AString> lstRequiredExtensions;
    MessageUtil::GetHeaders(&objMessage, ISipHeader::REQUIRE, lstRequiredExtensions);

    for (IMS_UINT32 nIndex = 0; nIndex < lstRequiredExtensions.GetSize(); nIndex++)
    {
        const AString& strOptionTag = lstRequiredExtensions.GetAt(nIndex);
        if (!IsAvailableOnLocal(strOptionTag))
        {
            IMS_TRACE_I(
                    "IsSupportRequiredExtensions : Not support [%s]", strOptionTag.GetStr(), 0, 0);
            return IMS_FALSE;
        }
    }
    return IMS_TRUE;
}

PUBLIC VIRTUAL void MtcExtensionSet::FormatRequest(
        IN RequestType eType, IN_OUT IMessage& objRequest)
{
    for (IMS_UINT32 nIndex = 0; nIndex < m_objExtensions.GetSize(); nIndex++)
    {
        m_objExtensions.GetValueAt(nIndex)->FormatRequest(eType, objRequest);
    }
}

PUBLIC VIRTUAL void MtcExtensionSet::FormatResponse(
        IN ResponseType eType, IN_OUT IMessage& objResponse)
{
    for (IMS_UINT32 nIndex = 0; nIndex < m_objExtensions.GetSize(); nIndex++)
    {
        m_objExtensions.GetValueAt(nIndex)->FormatResponse(eType, objResponse);
    }
}

PUBLIC VIRTUAL void MtcExtensionSet::HandleRequest(
        IN RequestType eType, IN const IMessage& objRequest)
{
    for (IMS_UINT32 nIndex = 0; nIndex < m_objExtensions.GetSize(); nIndex++)
    {
        m_objExtensions.GetValueAt(nIndex)->HandleRequest(eType, objRequest);
    }
}

PUBLIC VIRTUAL void MtcExtensionSet::HandleResponse(
        IN ResponseType eType, IN const IMessage& objResponse)
{
    for (IMS_UINT32 nIndex = 0; nIndex < m_objExtensions.GetSize(); nIndex++)
    {
        m_objExtensions.GetValueAt(nIndex)->HandleResponse(eType, objResponse);
    }
}

PRIVATE
void MtcExtensionSet::CopyFrom(IN const MtcExtensionSet& objRhs)
{
    Clear();

    for (IMS_UINT32 nIndex = 0; nIndex < objRhs.m_objExtensions.GetSize(); nIndex++)
    {
        m_objExtensions.Add(objRhs.m_objExtensions.GetKeyAt(nIndex),
                objRhs.m_objExtensions.GetValueAt(nIndex)->Clone());
    }
}

PRIVATE
void MtcExtensionSet::Clear()
{
    for (IMS_UINT32 nIndex = 0; nIndex < m_objExtensions.GetSize(); nIndex++)
    {
        delete m_objExtensions.GetValueAt(nIndex);
    }
    m_objExtensions.Clear();
}
