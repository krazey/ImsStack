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

#include "IDocument.h"
#include "IElement.h"
#include "INodeList.h"

#include "IRegInfoListener.h"
#include "RegInfo.h"
#include "RegInfoConst.h"
#include "RegInfoRegistration.h"

__IMS_TRACE_TAG_REG__;

PUBLIC
RegInfo::RegInfo() :
        m_bIsCreated(IMS_FALSE),
        m_nVersion(0),
        m_objListeners(ImsList<IRegInfoListener*>()),
        m_objRegistrations(ImsList<RegInfoRegistration*>())
{
}

PUBLIC VIRTUAL RegInfo::~RegInfo()
{
    RemoveAllRegistrations();
}

PUBLIC VIRTUAL IRegInfoRegistration* RegInfo::GetRegistration(IN const AString& strAor) const
{
    SipAddress objAor;

    if (!objAor.Create(strAor))
    {
        return IMS_NULL;
    }

    return GetRegistration(objAor);
}

PUBLIC VIRTUAL IRegInfoRegistration* RegInfo::GetRegistration(IN const SipAddress& objAor) const
{
    for (IMS_UINT32 i = 0; i < m_objRegistrations.GetSize(); ++i)
    {
        RegInfoRegistration* pRegistration = m_objRegistrations.GetAt(i);

        if (objAor.Equals(pRegistration->GetAor()))
        {
            return pRegistration;
        }
    }

    return IMS_NULL;
}

PUBLIC VIRTUAL ImsList<IRegInfoRegistration*> RegInfo::GetRegistrations() const
{
    ImsList<IRegInfoRegistration*> objRegInfoRegistrations;

    for (IMS_UINT32 i = 0; i < m_objRegistrations.GetSize(); ++i)
    {
        RegInfoRegistration* pRegistration = m_objRegistrations.GetAt(i);

        if (pRegistration != IMS_NULL)
        {
            objRegInfoRegistrations.Append(pRegistration);
        }
    }

    return objRegInfoRegistrations;
}

PUBLIC
void RegInfo::AddListener(IN IRegInfoListener* piListener)
{
    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        const IRegInfoListener* piTmpListener = m_objListeners.GetAt(i);

        if (piTmpListener == piListener)
        {
            return;
        }
    }

    m_objListeners.Append(piListener);
}

PUBLIC
void RegInfo::RemoveListener(IN const IRegInfoListener* piListener)
{
    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        const IRegInfoListener* piTmpListener = m_objListeners.GetAt(i);

        if (piTmpListener == piListener)
        {
            m_objListeners.RemoveAt(i);
            return;
        }
    }
}

PUBLIC
IMS_BOOL RegInfo::Update(IN const IDocument* piDocument)
{
    if (piDocument == IMS_NULL)
    {
        IMS_TRACE_E(0, "Document is null", 0, 0, 0);

        CallListener(STATUS_UPDATE_FAILED);
        return IMS_FALSE;
    }

    IElement* piElement = piDocument->GetDocumentElement();

    if (piElement == IMS_NULL)
    {
        CallListener(STATUS_UPDATE_FAILED);
        return IMS_FALSE;
    }

    const AString& strRegInfo = piElement->GetTagName();

    if (!strRegInfo.EqualsIgnoreCase(RegInfoConst::ELEMENT_REGINFO))
    {
        IMS_TRACE_E(0, "Root element (%s) is not matched in 'reginfo'", strRegInfo.GetStr(), 0, 0);

        CallListener(STATUS_UPDATE_FAILED);
        return IMS_FALSE;
    }

    // "version" attribute
    const AString& strVersion = piElement->GetAttribute(RegInfoConst::ATTR_VERSION);
    IMS_BOOL bOk = IMS_FALSE;
    IMS_UINT32 nNewVersion = strVersion.ToUInt32(&bOk);

    if (!bOk)
    {
        IMS_TRACE_E(0, "Invalid version attribute", 0, 0, 0);

        CallListener(STATUS_UPDATE_FAILED);
        return IMS_FALSE;
    }

    // "state" attribute
    const AString& strState = piElement->GetAttribute(RegInfoConst::ATTR_STATE);

    // If the version is 0, it means that an initial reginfo is received
    if (m_bIsCreated && (m_nVersion >= nNewVersion))
    {
        if ((m_nVersion == nNewVersion) && (m_nVersion == 0) &&
                strState.EqualsIgnoreCase(RegInfoConst::ATTR_STATE_FULL))
        {
            IMS_TRACE_I("RegInfo: Same version & full - being processed", 0, 0, 0);
        }
        else
        {
            IMS_TRACE_I("RegInfo: Equal or less (%d) than the local version (%d) - discarded",
                    nNewVersion, m_nVersion, 0);

            CallListener(STATUS_UPDATE_FAILED);
            return IMS_TRUE;
        }
    }

    IMS_BOOL bSubscriptionRefreshRequired = IMS_FALSE;

    // more than one higher than the local version number
    if (nNewVersion > (m_nVersion + 1))
    {
        // refreshed SUBSCRIBE needs to be sent
        bSubscriptionRefreshRequired = IMS_TRUE;

        IMS_TRACE_I("RegInfo :: Subscription refresh is required - "
                    "New version (%d), Old version (%d)",
                nNewVersion, m_nVersion, 0);
    }

    if (!m_bIsCreated)
    {
        m_bIsCreated = IMS_TRUE;
    }

    m_nVersion = nNewVersion;

    if (strState.EqualsIgnoreCase(RegInfoConst::ATTR_STATE_FULL))
    {
        IMS_TRACE_D("RegInfo :: 'full' state received", 0, 0, 0);

        // Remove all the registrations & updates all...
        RemoveAllRegistrations();
    }

    // "registration" element
    INodeList* piNodeList = piElement->GetElementsByTagName(RegInfoConst::ELEMENT_REGISTRATION);

    if (piNodeList != IMS_NULL)
    {
        for (IMS_SINT32 i = 0; i < piNodeList->GetLength(); ++i)
        {
            INode* piNode = piNodeList->Item(i);
            RegInfoRegistration* pRegistration = CheckNCreateRegistration(piNode);

            if (pRegistration != IMS_NULL)
            {
                pRegistration->Update(piNode);
            }
        }

        piElement->DestroyNodeList(piNodeList);
    }

    CallListener(STATUS_UPDATED);

    if (bSubscriptionRefreshRequired)
    {
        CallListener(STATUS_REFRESH_REQUIRED);
    }

    return IMS_TRUE;
}

void RegInfo::DisplayRegInfo()
{
    IMS_TRACE_I("RegInfo :: Version (%d)", m_nVersion, 0, 0);

    for (IMS_UINT32 i = 0; i < m_objRegistrations.GetSize(); ++i)
    {
        IMS_TRACE_D("", 0, 0, 0);

        RegInfoRegistration* pRegistration = m_objRegistrations.GetAt(i);

        pRegistration->DisplayRegInfo();
    }
}

PRIVATE
void RegInfo::CallListener(IN IMS_SINT32 nStatus)
{
    if (m_objListeners.IsEmpty())
    {
        return;
    }

    switch (nStatus)
    {
        case STATUS_REFRESH_REQUIRED:
            for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
            {
                IRegInfoListener* piListener = m_objListeners.GetAt(i);

                if (piListener != IMS_NULL)
                {
                    piListener->RegInfo_Updated(IMS_TRUE);
                }
            }
            break;
        case STATUS_UPDATED:
            for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
            {
                IRegInfoListener* piListener = m_objListeners.GetAt(i);

                if (piListener != IMS_NULL)
                {
                    piListener->RegInfo_Updated();
                }
            }
            break;
        case STATUS_UPDATE_FAILED:
            for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
            {
                IRegInfoListener* piListener = m_objListeners.GetAt(i);

                if (piListener != IMS_NULL)
                {
                    piListener->RegInfo_UpdateFailed();
                }
            }
            break;
        default:
            break;
    }
}

PRIVATE
RegInfoRegistration* RegInfo::CheckNCreateRegistration(IN INode* piNode)
{
    for (IMS_UINT32 i = 0; i < m_objRegistrations.GetSize(); ++i)
    {
        RegInfoRegistration* pRegistration = m_objRegistrations.GetAt(i);

        if (pRegistration->Equals(piNode))
        {
            return pRegistration;
        }
    }

    // New registration updated...
    RegInfoRegistration* pRegistration = new RegInfoRegistration();

    if (pRegistration == IMS_NULL)
    {
        return IMS_NULL;
    }

    if (!m_objRegistrations.Append(pRegistration))
    {
        IMS_TRACE_E(0, "Adding a new RegInfoRegistration failed", 0, 0, 0);

        delete pRegistration;
        return IMS_NULL;
    }

    return pRegistration;
}

PRIVATE
void RegInfo::RemoveAllRegistrations()
{
    if (m_objRegistrations.IsEmpty())
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objRegistrations.GetSize(); ++i)
    {
        RegInfoRegistration* pRegistration = m_objRegistrations.GetAt(i);

        if (pRegistration != IMS_NULL)
        {
            delete pRegistration;
        }
    }

    m_objRegistrations.Clear();
}
