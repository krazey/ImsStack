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
#include "ServiceUtil.h"

#include "INamedNodeMap.h"
#include "INode.h"

#include "RegInfoConst.h"
#include "RegInfoRegistration.h"
#include "SipDebug.h"

__IMS_TRACE_TAG_REG__;

PUBLIC
RegInfoRegistration::RegInfoRegistration() :
        m_strId(AString::ConstNull()),
        m_nState(STATE_CREATED),
        m_objContacts(ImsList<RegInfoContact*>())
{
}

PUBLIC VIRTUAL RegInfoRegistration::~RegInfoRegistration()
{
    for (IMS_UINT32 i = 0; i < m_objContacts.GetSize(); ++i)
    {
        RegInfoContact* pContact = m_objContacts.GetAt(i);

        if (pContact != IMS_NULL)
        {
            delete pContact;
        }
    }

    IMS_TRACE_D("Destructor :: aor=%s, id=%s", SipDebug::GetUri1(m_objAor.ToString()).GetStr(),
            m_strId.GetStr(), 0);
}

PUBLIC VIRTUAL IRegInfoContact* RegInfoRegistration::GetContact(
        IN const SipAddress& objContactUri) const
{
    for (IMS_UINT32 i = 0; i < m_objContacts.GetSize(); ++i)
    {
        RegInfoContact* pContact = m_objContacts.GetAt(i);

        if (objContactUri.Equals(pContact->GetUri()))
        {
            return pContact;
        }
    }

    return IMS_NULL;
}

PUBLIC VIRTUAL ImsList<IRegInfoContact*> RegInfoRegistration::GetContacts() const
{
    ImsList<IRegInfoContact*> objRegInfoContacts;

    for (IMS_UINT32 i = 0; i < m_objContacts.GetSize(); ++i)
    {
        objRegInfoContacts.Append(m_objContacts.GetAt(i));
    }

    return objRegInfoContacts;
}

PUBLIC VIRTUAL RegInfoContact* RegInfoRegistration::GetPriorContact() const
{
    if (m_objContacts.IsEmpty())
    {
        return IMS_NULL;
    }

    RegInfoContact* pContact = m_objContacts.GetAt(0);

    for (IMS_UINT32 i = 1; i < m_objContacts.GetSize(); ++i)
    {
        RegInfoContact* pTmpContact = m_objContacts.GetAt(i);

        if (pContact->GetQValue() < pTmpContact->GetQValue())
        {
            pContact = pTmpContact;
        }
    }

    return pContact;
}

PUBLIC
IMS_BOOL RegInfoRegistration::Equals(IN INode* piNode) const
{
    if (piNode == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!piNode->GetLocalName().EqualsIgnoreCase(RegInfoConst::ELEMENT_REGISTRATION))
    {
        return IMS_FALSE;
    }

    // Find "id" attribute & compares it
    INamedNodeMap* piNodeMap = piNode->GetAttributes();

    if (piNodeMap == IMS_NULL)
    {
        return IMS_FALSE;
    }

    const INode* piNodeId = piNodeMap->GetNamedItem(RegInfoConst::ATTR_ID);

    if (piNodeId == IMS_NULL)
    {
        piNode->DestroyNamedNodeMap(piNodeMap);
        return IMS_FALSE;
    }

    if (!m_strId.EqualsIgnoreCase(piNodeId->GetNodeValue()))
    {
        piNode->DestroyNamedNodeMap(piNodeMap);
        return IMS_FALSE;
    }

    piNode->DestroyNamedNodeMap(piNodeMap);

    return IMS_TRUE;
}

void RegInfoRegistration::DisplayRegInfo()
{
    // clang-format off
    static const IMS_CHAR* pszState[] = {
            "CREATED",
            "INIT",
            "ACTIVE",
            "TERMINATED"
    };
    // clang-format on

    if (IMS_UTIL_SYS_PROP_IS_SERVER_INFO_HIDDEN_IN_LOG())
    {
        IMS_TRACE_I("REG :: id=%s, state=%s", m_strId.GetStr(), pszState[m_nState], 0);
    }
    else
    {
        IMS_TRACE_I("REG :: id=%s, aor=%s, state=%s", m_strId.GetStr(),
                SipDebug::GetUri1(m_objAor.ToString()).GetStr(), pszState[m_nState]);
    }

    AString strTag;

    for (IMS_UINT32 i = 0; i < m_objContacts.GetSize(); ++i)
    {
        RegInfoContact* pContact = m_objContacts.GetAt(i);

        strTag.SetNumber(i);

        pContact->DisplayRegInfo(strTag);
    }

    RegInfoContact* pContact = GetPriorContact();

    pContact->DisplayRegInfo("Prior");
}

PUBLIC
IMS_BOOL RegInfoRegistration::Update(IN INode* piNode)
{
    if (piNode == IMS_NULL)
    {
        return IMS_FALSE;
    }

    INamedNodeMap* piNodeMap = piNode->GetAttributes();

    if (piNodeMap == IMS_NULL)
    {
        IMS_TRACE_E(0, "No attributes in 'registration'", 0, 0, 0);
        return IMS_FALSE;
    }

    if (m_nState == STATE_CREATED)
    {
        // Updates all the fields

        // "aor" attribute
        if (!SetAor(piNodeMap))
        {
            piNode->DestroyNamedNodeMap(piNodeMap);
            return IMS_FALSE;
        }

        // "id" attribute
        if (!SetId(piNodeMap))
        {
            piNode->DestroyNamedNodeMap(piNodeMap);
            return IMS_FALSE;
        }

        if (IMS_UTIL_SYS_PROP_IS_SERVER_INFO_HIDDEN_IN_LOG())
        {
            IMS_TRACE_I("REG :: id=%s", m_strId.GetStr(), 0, 0);
        }
        else
        {
            IMS_TRACE_I("REG :: aor=%s, id=%s", SipDebug::GetUri1(m_objAor.ToString()).GetStr(),
                    m_strId.GetStr(), 0);
        }
    }
    else
    {
        // Updates only the state & Contacts
    }

    // "state" attribute
    if (!SetState(piNodeMap))
    {
        piNode->DestroyNamedNodeMap(piNodeMap);
        return IMS_FALSE;
    }

    piNode->DestroyNamedNodeMap(piNodeMap);

    // "contact" elements
    INode* piNode_Contact = piNode->GetFirstChild();

    if (!SetContacts(piNode_Contact))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
RegInfoContact* RegInfoRegistration::CheckNCreateContact(IN INode* piNode)
{
    for (IMS_UINT32 i = 0; i < m_objContacts.GetSize(); ++i)
    {
        RegInfoContact* pContact = m_objContacts.GetAt(i);

        if (pContact->Equals(piNode))
        {
            return pContact;
        }
    }

    // New contact updated...
    RegInfoContact* pContact = new RegInfoContact();

    if (pContact == IMS_NULL)
    {
        return IMS_NULL;
    }

    if (!m_objContacts.Append(pContact))
    {
        delete pContact;
        return IMS_NULL;
    }

    return pContact;
}

PRIVATE
IMS_BOOL RegInfoRegistration::SetAor(IN const INamedNodeMap* piNodeMap)
{
    const INode* piNode = piNodeMap->GetNamedItem(RegInfoConst::ATTR_AOR);

    if (piNode == IMS_NULL)
    {
        IMS_TRACE_E(0, "Can't find 'aor' attribute", 0, 0, 0);
        return IMS_FALSE;
    }

    if (!m_objAor.Create(piNode->GetNodeValue()))
    {
        IMS_TRACE_E(0, "Creating an AOR (%s) failed",
                SipDebug::GetUri1(piNode->GetNodeValue()).GetStr(), 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL RegInfoRegistration::SetContacts(IN INode* piNode)
{
    while (piNode != IMS_NULL)
    {
        RegInfoContact* pContact = CheckNCreateContact(piNode);

        if (pContact != IMS_NULL)
        {
            pContact->Update(piNode);
        }

        piNode = piNode->GetNextSibling();
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL RegInfoRegistration::SetId(IN const INamedNodeMap* piNodeMap)
{
    const INode* piNode = piNodeMap->GetNamedItem(RegInfoConst::ATTR_ID);

    if (piNode == IMS_NULL)
    {
        IMS_TRACE_E(0, "Can't find 'id' attribute", 0, 0, 0);
        return IMS_FALSE;
    }

    m_strId = piNode->GetNodeValue();

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL RegInfoRegistration::SetState(IN const INamedNodeMap* piNodeMap)
{
    const INode* piNode = piNodeMap->GetNamedItem(RegInfoConst::ATTR_STATE);

    if (piNode == IMS_NULL)
    {
        IMS_TRACE_E(0, "Can't find 'state' attribute", 0, 0, 0);
        return IMS_FALSE;
    }

    const AString& strState = piNode->GetNodeValue();

    if (strState.EqualsIgnoreCase(RegInfoConst::ATTR_STATE_INIT))
    {
        m_nState = STATE_INIT;
    }
    else if (strState.EqualsIgnoreCase(RegInfoConst::ATTR_STATE_ACTIVE))
    {
        m_nState = STATE_ACTIVE;
    }
    else if (strState.EqualsIgnoreCase(RegInfoConst::ATTR_STATE_TERMINATED))
    {
        m_nState = STATE_TERMINATED;
    }

    return IMS_TRUE;
}
