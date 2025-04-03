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
#include "AStringBuffer.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceUtil.h"

#include "INamedNodeMap.h"
#include "INode.h"

#include "RegInfoConst.h"
#include "RegInfoContact.h"
#include "SipDebug.h"

__IMS_TRACE_TAG_REG__;

PUBLIC
RegInfoContact::RegInfoContact() :
        m_strId(AString::ConstNull()),
        m_nState(STATE_CREATED),
        m_nEvent(EVENT_UNREGISTERED),
        m_nDurationRegistered(0),
        m_nExpires(0),
        m_nRetryAfter(0),
        m_strDisplayName(AString::ConstNull()),
        m_strQValue(AString::ConstNull()),
        m_strCallId(AString::ConstNull()),
        m_nCSeq(0),
        m_strPubGruu(AString::ConstNull()),
        m_objTempGruu(TempGruu()),
        m_objUnknownParameters(ImsMap<AString, AString>())
{
}

PUBLIC VIRTUAL RegInfoContact::~RegInfoContact()
{
    IMS_TRACE_D("Destructor :: uri=%s, id=%s", SipDebug::GetUri1(m_objUri.ToString()).GetStr(),
            m_strId.GetStr(), 0);
}

PUBLIC VIRTUAL const AString& RegInfoContact::GetUnknownParameter(IN const AString& strName) const
{
    IMS_SLONG nIndex = m_objUnknownParameters.GetIndexOfKey(strName);

    if (nIndex < 0)
    {
        IMS_TRACE_D("Unknown-param (%s) does not exist", strName.GetStr(), 0, 0);
        return AString::ConstNull();
    }

    const AString& strValue = m_objUnknownParameters.GetValueAt(nIndex);

    if (strValue.GetLength() == 0)
    {
        return AString::ConstEmpty();
    }

    return strValue;
}

PUBLIC
IMS_BOOL RegInfoContact::Equals(IN INode* piNode) const
{
    if (piNode == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!piNode->GetLocalName().EqualsIgnoreCase(RegInfoConst::ELEMENT_CONTACT))
    {
        return IMS_FALSE;
    }

    // Find "id" attribute & compares it
    INamedNodeMap* piNodeMap = piNode->GetAttributes();

    if (piNodeMap == IMS_NULL)
    {
        return IMS_FALSE;
    }

    INode* piNodeId = piNodeMap->GetNamedItem(RegInfoConst::ATTR_ID);

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

PUBLIC
IMS_BOOL RegInfoContact::Update(IN INode* piNode)
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

        // "id" attribute
        if (!SetId(piNodeMap))
        {
            piNode->DestroyNamedNodeMap(piNodeMap);
            return IMS_FALSE;
        }

        // "callid" attribute
        SetCallId(piNodeMap);

        // "uri" & "display-name" element
        INode* piChildNode = piNode->GetFirstChild();

        while (piChildNode != IMS_NULL)
        {
            const AString& strNodeName = piChildNode->GetLocalName();

            if (strNodeName.EqualsIgnoreCase(RegInfoConst::ELEMENT_URI))
            {
                // The reason we don't perform error handling here is to allow the current parsing
                // process to continue even if the "uri" element of "contact" element is malformed
                // and leave related exception handling to the enabler.
                SetUri(piChildNode);
            }
            else if (strNodeName.EqualsIgnoreCase(RegInfoConst::ELEMENT_DISPLAY_NAME))
            {
                SetDisplayName(piChildNode);
            }
            else if (strNodeName.EqualsIgnoreCase(RegInfoConst::ELEMENT_UNKNOWN_PARAM))
            {
                SetUnknownParameter(piChildNode);
            }
            else if (strNodeName.EqualsIgnoreCase(RegInfoConst::ELEMENT_PUB_GRUU))
            {
                SetPublicGruu(piChildNode);
            }
            else if (strNodeName.EqualsIgnoreCase(RegInfoConst::ELEMENT_TEMP_GRUU))
            {
                SetTemporaryGruu(piChildNode);
            }
            else
            {
                IMS_TRACE_D("Unknown element (%s)", strNodeName.GetStr(), 0, 0);
            }

            piChildNode = piChildNode->GetNextSibling();
        }

        if (!IMS_UTIL_SYS_PROP_IS_SERVER_INFO_HIDDEN_IN_LOG())
        {
            IMS_TRACE_I("Contact :: uri=%s, id=%s", SipDebug::GetUri1(m_objUri.ToString()).GetStr(),
                    m_strId.GetStr(), 0);
        }
    }
    else
    {
        // Updates only the state & Contacts

        // "display-name" element
        INode* piChildNode = piNode->GetFirstChild();

        while (piChildNode != IMS_NULL)
        {
            const AString& strNodeName = piChildNode->GetLocalName();

            if (strNodeName.EqualsIgnoreCase(RegInfoConst::ELEMENT_DISPLAY_NAME))
            {
                SetDisplayName(piChildNode);
            }
            else if (strNodeName.EqualsIgnoreCase(RegInfoConst::ELEMENT_PUB_GRUU))
            {
                SetPublicGruu(piChildNode);
            }
            else if (strNodeName.EqualsIgnoreCase(RegInfoConst::ELEMENT_TEMP_GRUU))
            {
                SetTemporaryGruu(piChildNode);
            }
            else
            {
                IMS_TRACE_D("Skip or unknown element (%s)", strNodeName.GetStr(), 0, 0);
            }

            piChildNode = piChildNode->GetNextSibling();
        }
    }

    // "state" attribute
    if (!SetState(piNodeMap))
    {
        piNode->DestroyNamedNodeMap(piNodeMap);
        return IMS_FALSE;
    }

    // "event" attribute
    if (!SetEvent(piNodeMap))
    {
        piNode->DestroyNamedNodeMap(piNodeMap);
        return IMS_FALSE;
    }

    // "cseq" attribute
    SetCSeq(piNodeMap);

    // "duration-registered" attribute
    SetDurationRegistered(piNodeMap);

    // "expires" attribute
    SetExpiresValue(piNodeMap);

    // "q" attribute
    SetQValue(piNodeMap);

    // "retry-after" attribute
    SetRetryAfterValue(piNodeMap);

    piNode->DestroyNamedNodeMap(piNodeMap);

    return IMS_TRUE;
}

void RegInfoContact::DisplayRegInfo(IN const AString& strTag /*= AString::ConstNull()*/)
{
    // clang-format off
    static const IMS_CHAR* pszState[] = {
            "CREATED",
            "ACTIVE",
            "TERMINATED"
    };

    static const IMS_CHAR* pszEvent[] = {
            "REGISTERED",
            "CREATED",
            "REFRESHED",
            "SHORTENED",
            "EXPIRED",
            "DEACTIVATED",
            "PROBATION",
            "UNREGISTERED",
            "REJECTED"
    };
    // clang-format on

    AStringBuffer objLog(512);

    objLog.Append("id=").Append(m_strId);
    objLog.Append(", state=").Append(pszState[m_nState]);
    objLog.Append(", event=").Append(pszEvent[m_nEvent]);
    objLog.Append(", duration-registered=").Append(m_nDurationRegistered);
    objLog.Append(", expires=").Append(m_nExpires);
    objLog.Append(", callid=").Append(SipDebug::GetCharA1(m_strCallId.GetStr(), 8, '@'));
    objLog.Append(", cseq=").Append(m_nCSeq);
    objLog.Append(", retry-after=").Append(m_nRetryAfter);
    objLog.Append(", q-value=").Append(m_strQValue);
    objLog.Append(", first-cseq=").Append(m_objTempGruu.m_nFirstCSeq);

    if (!IMS_UTIL_SYS_PROP_IS_SERVER_INFO_HIDDEN_IN_LOG())
    {
        objLog.Append(", display-name=").Append(SipDebug::GetCharA1(m_strDisplayName.GetStr(), 6));
        objLog.Append(", uri=").Append(SipDebug::GetUri1(m_objUri.ToString()));
        objLog.Append(", pub-gruu=").Append(SipDebug::GetUri1(m_strPubGruu));
        objLog.Append(", temp-gruu=").Append(SipDebug::GetUri1(m_objTempGruu.m_strGruu));
    }

    objLog.Append(", unknown-param-count=").Append(m_objUnknownParameters.GetSize());

    IMS_TRACE_I("CON(%s) :: %s", strTag.GetStr(), objLog.GetCharString(), 0);

    for (IMS_UINT32 i = 0; i < m_objUnknownParameters.GetSize(); ++i)
    {
        const AString& strName = m_objUnknownParameters.GetKeyAt(i);
        const AString& strValue = m_objUnknownParameters.GetValueAt(i);

        IMS_TRACE_D("(%s) unknown-param :: %s=%s", strTag.GetStr(), strName.GetStr(),
                SipDebug::GetCharA1(strValue.GetStr(), 6));
    }
}

PRIVATE
void RegInfoContact::SetCallId(IN INamedNodeMap* piNodeMap)
{
    INode* piNode = piNodeMap->GetNamedItem(RegInfoConst::ATTR_CALLID);

    if (piNode == IMS_NULL)
    {
        IMS_TRACE_D("No 'callid' attribute", 0, 0, 0);
        return;
    }

    m_strCallId = piNode->GetNodeValue();
}

PRIVATE
void RegInfoContact::SetCSeq(IN INamedNodeMap* piNodeMap)
{
    INode* piNode = piNodeMap->GetNamedItem(RegInfoConst::ATTR_CSEQ);

    if (piNode == IMS_NULL)
    {
        IMS_TRACE_D("No 'cseq' attribute", 0, 0, 0);
        return;
    }

    const AString& strCSeq = piNode->GetNodeValue();
    IMS_BOOL bOk = IMS_FALSE;
    IMS_UINT32 nNewCSeq = strCSeq.ToUInt32(&bOk);

    if (!bOk)
    {
        return;
    }

    m_nCSeq = nNewCSeq;
}

PRIVATE
void RegInfoContact::SetDurationRegistered(IN INamedNodeMap* piNodeMap)
{
    INode* piNode = piNodeMap->GetNamedItem(RegInfoConst::ATTR_DURATION_REGISTERED);

    if (piNode == IMS_NULL)
    {
        IMS_TRACE_D("No 'duration-registered' attribute", 0, 0, 0);
        return;
    }

    const AString& strDurationRegistered = piNode->GetNodeValue();
    IMS_BOOL bOk = IMS_FALSE;
    IMS_UINT32 nNewDurationRegistered = strDurationRegistered.ToUInt32(&bOk);

    if (!bOk)
    {
        return;
    }

    m_nDurationRegistered = nNewDurationRegistered;
}

PRIVATE
IMS_BOOL RegInfoContact::SetEvent(IN INamedNodeMap* piNodeMap)
{
    INode* piNode = piNodeMap->GetNamedItem(RegInfoConst::ATTR_EVENT);

    if (piNode == IMS_NULL)
    {
        IMS_TRACE_E(0, "Can't find 'event' attribute", 0, 0, 0);
        return IMS_FALSE;
    }

    const AString& strEvent = piNode->GetNodeValue();

    if (strEvent.EqualsIgnoreCase(RegInfoConst::ATTR_EVENT_REGISTERED))
    {
        m_nEvent = EVENT_REGISTERED;
    }
    else if (strEvent.EqualsIgnoreCase(RegInfoConst::ATTR_EVENT_CREATED))
    {
        m_nEvent = EVENT_CREATED;
    }
    else if (strEvent.EqualsIgnoreCase(RegInfoConst::ATTR_EVENT_REFRESHED))
    {
        m_nEvent = EVENT_REFRESHED;
    }
    else if (strEvent.EqualsIgnoreCase(RegInfoConst::ATTR_EVENT_SHORTENED))
    {
        m_nEvent = EVENT_SHORTENED;
    }
    else if (strEvent.EqualsIgnoreCase(RegInfoConst::ATTR_EVENT_EXPIRED))
    {
        m_nEvent = EVENT_EXPIRED;
    }
    else if (strEvent.EqualsIgnoreCase(RegInfoConst::ATTR_EVENT_DEACTIVATED))
    {
        m_nEvent = EVENT_DEACTIVATED;
    }
    else if (strEvent.EqualsIgnoreCase(RegInfoConst::ATTR_EVENT_PROBATION))
    {
        m_nEvent = EVENT_PROBATION;
    }
    else if (strEvent.EqualsIgnoreCase(RegInfoConst::ATTR_EVENT_UNREGISTERED))
    {
        m_nEvent = EVENT_UNREGISTERED;
    }
    else if (strEvent.EqualsIgnoreCase(RegInfoConst::ATTR_EVENT_REJECTED))
    {
        m_nEvent = EVENT_REJECTED;
    }

    return IMS_TRUE;
}

PRIVATE
void RegInfoContact::SetExpiresValue(IN INamedNodeMap* piNodeMap)
{
    // If 'event' is "shortened", then it MUST be present
    INode* piNode = piNodeMap->GetNamedItem(RegInfoConst::ATTR_EXPIRES);

    if (piNode == IMS_NULL)
    {
        IMS_TRACE_D("No 'expires' attribute", 0, 0, 0);
        return;
    }

    const AString& strExpires = piNode->GetNodeValue();
    IMS_BOOL bOk = IMS_FALSE;
    IMS_UINT32 nNewExpires = strExpires.ToUInt32(&bOk);

    if (!bOk)
    {
        return;
    }

    m_nExpires = nNewExpires;
}

PRIVATE
IMS_BOOL RegInfoContact::SetId(IN INamedNodeMap* piNodeMap)
{
    INode* piNode = piNodeMap->GetNamedItem(RegInfoConst::ATTR_ID);

    if (piNode == IMS_NULL)
    {
        IMS_TRACE_E(0, "Can't find 'id' attribute", 0, 0, 0);
        return IMS_FALSE;
    }

    m_strId = piNode->GetNodeValue();

    return IMS_TRUE;
}

PRIVATE
void RegInfoContact::SetQValue(IN INamedNodeMap* piNodeMap)
{
    INode* piNode = piNodeMap->GetNamedItem(RegInfoConst::ATTR_Q);

    if (piNode == IMS_NULL)
    {
        IMS_TRACE_D("No 'q' attribute", 0, 0, 0);
        return;
    }

    m_strQValue = piNode->GetNodeValue();
}

PRIVATE
void RegInfoContact::SetRetryAfterValue(IN INamedNodeMap* piNodeMap)
{
    // If 'event' is "probation", then it MUST be present.
    INode* piNode = piNodeMap->GetNamedItem(RegInfoConst::ATTR_RETRY_AFTER);

    if (piNode == IMS_NULL)
    {
        IMS_TRACE_D("No 'retry-after' attribute", 0, 0, 0);
        return;
    }

    const AString& strRetryAfter = piNode->GetNodeValue();
    IMS_BOOL bOk = IMS_FALSE;
    IMS_UINT32 nNewRetryAfter = strRetryAfter.ToUInt32(&bOk);

    if (!bOk)
    {
        return;
    }

    m_nRetryAfter = nNewRetryAfter;
}

PRIVATE
IMS_BOOL RegInfoContact::SetState(IN INamedNodeMap* piNodeMap)
{
    INode* piNode = piNodeMap->GetNamedItem(RegInfoConst::ATTR_STATE);

    if (piNode == IMS_NULL)
    {
        IMS_TRACE_E(0, "Can't find 'state' attribute", 0, 0, 0);
        return IMS_FALSE;
    }

    const AString& strState = piNode->GetNodeValue();

    if (strState.EqualsIgnoreCase(RegInfoConst::ATTR_STATE_ACTIVE))
    {
        m_nState = STATE_ACTIVE;
    }
    else if (strState.EqualsIgnoreCase(RegInfoConst::ATTR_STATE_TERMINATED))
    {
        m_nState = STATE_TERMINATED;
    }

    return IMS_TRUE;
}

PRIVATE
void RegInfoContact::SetDisplayName(IN INode* piNode)
{
    if (piNode == IMS_NULL)
    {
        return;
    }

    INode* piContent = piNode->GetFirstChild();

    m_strDisplayName = piContent->GetNodeValue();
}

PRIVATE
void RegInfoContact::SetPublicGruu(IN INode* piNode)
{
    INamedNodeMap* piNodeMap = piNode->GetAttributes();

    if (piNodeMap == IMS_NULL)
    {
        return;
    }

    INode* piAttrNode = piNodeMap->GetNamedItem(RegInfoConst::ATTR_URI);

    if (piAttrNode == IMS_NULL)
    {
        piNode->DestroyNamedNodeMap(piNodeMap);
        IMS_TRACE_D("Pub-GRUU :: Can't find 'uri' attribute", 0, 0, 0);
        return;
    }

    m_strPubGruu = piAttrNode->GetNodeValue();

    piNode->DestroyNamedNodeMap(piNodeMap);
}

PRIVATE
void RegInfoContact::SetTemporaryGruu(IN INode* piNode)
{
    INamedNodeMap* piNodeMap = piNode->GetAttributes();

    if (piNodeMap == IMS_NULL)
    {
        return;
    }

    INode* piAttrNode = piNodeMap->GetNamedItem(RegInfoConst::ATTR_URI);

    if (piAttrNode == IMS_NULL)
    {
        piNode->DestroyNamedNodeMap(piNodeMap);
        IMS_TRACE_D("Temp-GRUU :: Can't find 'uri' attribute", 0, 0, 0);
        return;
    }

    m_objTempGruu.m_strGruu = piAttrNode->GetNodeValue();

    piAttrNode = piNodeMap->GetNamedItem(RegInfoConst::ATTR_FIRST_CSEQ);

    if (piAttrNode == IMS_NULL)
    {
        m_objTempGruu.m_nFirstCSeq = 0;
        piNode->DestroyNamedNodeMap(piNodeMap);

        IMS_TRACE_D("Temp-GRUU :: Can't find 'first-cseq' attribute", 0, 0, 0);
        return;
    }

    const AString& strFirstCSeq = piAttrNode->GetNodeValue();
    IMS_BOOL bOk = IMS_FALSE;
    IMS_UINT32 nNewFirstCSeq = strFirstCSeq.ToUInt32(&bOk);

    if (!bOk)
    {
        m_objTempGruu.m_nFirstCSeq = 0;
        piNode->DestroyNamedNodeMap(piNodeMap);
        return;
    }

    m_objTempGruu.m_nFirstCSeq = nNewFirstCSeq;

    piNode->DestroyNamedNodeMap(piNodeMap);
}

PRIVATE
void RegInfoContact::SetUnknownParameter(IN INode* piNode)
{
    INamedNodeMap* piNodeMap = piNode->GetAttributes();

    if (piNodeMap == IMS_NULL)
    {
        return;
    }

    INode* piNameNode = piNodeMap->GetNamedItem(RegInfoConst::ATTR_NAME);

    if (piNameNode == IMS_NULL)
    {
        piNode->DestroyNamedNodeMap(piNodeMap);
        IMS_TRACE_D("No 'name' attribute in unknown-param element", 0, 0, 0);
        return;
    }

    const AString& strName = piNameNode->GetNodeValue();

    if (strName.GetLength() == 0)
    {
        piNode->DestroyNamedNodeMap(piNodeMap);
        IMS_TRACE_D("No 'name' attribute value in unknown-param element", 0, 0, 0);
        return;
    }

    piNode->DestroyNamedNodeMap(piNodeMap);

    AString strValue = AString::ConstEmpty();
    INode* piContent = piNode->GetFirstChild();

    if (piContent != IMS_NULL)
    {
        strValue = piContent->GetNodeValue();

        if (strValue.Contains("&lt;"))
        {
            strValue.Replace("&lt;", "<");
            strValue.Replace("&gt;", ">");
        }

        if (strValue.GetLength() == 0)
        {
            strValue = AString::ConstEmpty();
        }
    }

    if (!IMS_UTIL_SYS_PROP_IS_SERVER_INFO_HIDDEN_IN_LOG())
    {
        if (strValue.IsEmpty())
        {
            IMS_TRACE_D("unknown-param :: %s", strName.GetStr(), 0, 0);
        }
        else
        {
            IMS_TRACE_D("unknown-param :: %s=%s", strName.GetStr(),
                    SipDebug::GetCharA1(strValue.GetStr(), 6), 0);
        }
    }

    m_objUnknownParameters.SetValue(strName, strValue);
}

PRIVATE
IMS_BOOL RegInfoContact::SetUri(IN INode* piNode)
{
    if (piNode == IMS_NULL)
    {
        IMS_TRACE_E(0, "No uri element", 0, 0, 0);
        return IMS_FALSE;
    }

    INode* piContent = piNode->GetFirstChild();

    if (piContent == IMS_NULL)
    {
        IMS_TRACE_E(0, "No content for uri", 0, 0, 0);
        return IMS_FALSE;
    }

    const AString& strUri = piContent->GetNodeValue();

    if (!m_objUri.Create(strUri))
    {
        IMS_TRACE_E(
                0, "Creating a Contact URI (%s) failed", SipDebug::GetUri1(strUri).GetStr(), 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}
