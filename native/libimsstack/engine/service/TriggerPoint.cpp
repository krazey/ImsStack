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
#include "TextParser.h"

#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ISipMessageBodyPart.h"
#include "SipParameter.h"
#include "SipParsingHelper.h"
#include "TriggerPoint.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
TriggerPoint::TriggerPoint(
        IN const SipMethod& objMethod, IN IMS_BOOL bMethodNegated /*= IMS_FALSE*/) :
        m_nEvaluationRule(SPT_SIP_RULE_MATCH | SPT_SDP_RULE_CONTAIN),
        m_bMethodNegated(bMethodNegated),
        m_objMethod(objMethod)
{
}

PUBLIC
TriggerPoint::TriggerPoint(IN const TriggerPoint& other) :
        m_nEvaluationRule(other.m_nEvaluationRule),
        m_bMethodNegated(other.m_bMethodNegated),
        m_objMethod(other.m_objMethod),
        m_objSdpMLines(other.m_objSdpMLines),
        m_objSdpALines(other.m_objSdpALines)
{
    for (IMS_UINT32 i = 0; i < other.m_objHeaders.GetSize(); ++i)
    {
        const ISipHeader* piHeader = other.m_objHeaders.GetAt(i);

        if (piHeader == IMS_NULL)
        {
            continue;
        }

        ISipHeader* piNewHeader = piHeader->Clone();

        if (piNewHeader == IMS_NULL)
        {
            IMS_TRACE_E(0, "Cloning a SIP header failed", 0, 0, 0);
            continue;
        }

        if (!m_objHeaders.Append(piNewHeader))
        {
            piNewHeader->Destroy();

            IMS_TRACE_E(0, "Adding a new SIP header failed", 0, 0, 0);
        }
    }

    for (IMS_UINT32 i = 0; i < other.m_objNegatedHeaders.GetSize(); ++i)
    {
        const ISipHeader* piHeader = other.m_objNegatedHeaders.GetAt(i);

        if (piHeader == IMS_NULL)
        {
            continue;
        }

        ISipHeader* piNewHeader = piHeader->Clone();

        if (piNewHeader == IMS_NULL)
        {
            IMS_TRACE_E(0, "Cloning a SIP header failed", 0, 0, 0);
            continue;
        }

        if (!m_objNegatedHeaders.Append(piNewHeader))
        {
            piNewHeader->Destroy();

            IMS_TRACE_E(0, "Adding a new SIP header failed", 0, 0, 0);
        }
    }
}

PUBLIC
TriggerPoint::~TriggerPoint()
{
    RemoveAllHeaders();
}

PUBLIC
TriggerPoint& TriggerPoint::operator=(IN const TriggerPoint& other)
{
    if (this != &other)
    {
        RemoveAllHeaders();

        m_nEvaluationRule = other.m_nEvaluationRule;

        m_bMethodNegated = other.m_bMethodNegated;
        m_objMethod = other.m_objMethod;

        for (IMS_UINT32 i = 0; i < other.m_objHeaders.GetSize(); ++i)
        {
            const ISipHeader* piHeader = other.m_objHeaders.GetAt(i);

            if (piHeader == IMS_NULL)
            {
                continue;
            }

            ISipHeader* piNewHeader = piHeader->Clone();

            if (piNewHeader == IMS_NULL)
            {
                IMS_TRACE_E(0, "Cloning a SIP header failed", 0, 0, 0);
                continue;
            }

            if (!m_objHeaders.Append(piNewHeader))
            {
                piNewHeader->Destroy();

                IMS_TRACE_E(0, "Adding a new SIP header failed", 0, 0, 0);
            }
        }

        for (IMS_UINT32 i = 0; i < other.m_objNegatedHeaders.GetSize(); ++i)
        {
            const ISipHeader* piHeader = other.m_objNegatedHeaders.GetAt(i);

            if (piHeader == IMS_NULL)
            {
                continue;
            }

            ISipHeader* piNewHeader = piHeader->Clone();

            if (piNewHeader == IMS_NULL)
            {
                IMS_TRACE_E(0, "Cloning a SIP header failed", 0, 0, 0);
                continue;
            }

            if (!m_objNegatedHeaders.Append(piNewHeader))
            {
                piNewHeader->Destroy();

                IMS_TRACE_E(0, "Adding a new SIP header failed", 0, 0, 0);
            }
        }

        m_objSdpMLines = other.m_objSdpMLines;
        m_objSdpALines = other.m_objSdpALines;
    }

    return (*this);
}

PUBLIC
IMS_BOOL TriggerPoint::AddHeader(IN IMS_SINT32 nType, IN const AString& strValue,
        IN const AString& strName /*= AString::ConstNull()*/,
        IN IMS_BOOL bHeaderNegated /*= IMS_FALSE*/)
{
    ISipHeader* piHeader = IMS_NULL;

    if (nType != ISipHeader::UNKNOWN)
    {
        piHeader = SipParsingHelper::CreateHeader(nType, strValue);
    }
    else
    {
        piHeader = SipParsingHelper::CreateHeader(strName, strValue);
    }

    if (piHeader == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a SIP header (%d, %s, %s) failed", nType, strName.GetStr(),
                strValue.GetStr());
        return IMS_FALSE;
    }

    if (!bHeaderNegated)
    {
        if (!m_objHeaders.Append(piHeader))
        {
            piHeader->Destroy();

            IMS_TRACE_E(0, "Adding a SIP header (%d, %s, %s) failed", nType, strName.GetStr(),
                    strValue.GetStr());
            return IMS_FALSE;
        }
    }
    else
    {
        if (!m_objNegatedHeaders.Append(piHeader))
        {
            piHeader->Destroy();

            IMS_TRACE_E(0, "Adding a SIP header (%d, %s, %s) to failed", nType, strName.GetStr(),
                    strValue.GetStr());
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL TriggerPoint::AddSdpInfo(IN const IMS_CHAR cName, IN const AString& strValue)
{
    if (cName == 'm')
    {
        m_objSdpMLines.Append(strValue);
    }
    else if (cName == 'a')
    {
        m_objSdpALines.Append(strValue);
    }
    else
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
void TriggerPoint::RemoveAllHeaders()
{
    if (!m_objHeaders.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objHeaders.GetSize(); ++i)
        {
            ISipHeader* piHeader = m_objHeaders.GetAt(i);

            if (piHeader == IMS_NULL)
            {
                continue;
            }

            piHeader->Destroy();
        }

        m_objHeaders.Clear();
    }

    if (!m_objNegatedHeaders.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objNegatedHeaders.GetSize(); ++i)
        {
            ISipHeader* piHeader = m_objNegatedHeaders.GetAt(i);

            if (piHeader == IMS_NULL)
            {
                continue;
            }

            piHeader->Destroy();
        }

        m_objNegatedHeaders.Clear();
    }
}

PUBLIC
void TriggerPoint::RemoveAllSdpInfo()
{
    m_objSdpMLines.Clear();
    m_objSdpALines.Clear();
}

PRIVATE
IMS_BOOL TriggerPoint::Evaluate(IN const ISipMessage* piSipMsg) const
{
    if (piSipMsg == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!m_bMethodNegated)
    {
        if (!m_objMethod.Equals(piSipMsg->GetMethod()))
        {
            return IMS_FALSE;
        }
    }
    else
    {
        if (m_objMethod.Equals(piSipMsg->GetMethod()))
        {
            return IMS_FALSE;
        }
    }

    // In case the method is only matched ...
    if (m_objHeaders.IsEmpty() && m_objNegatedHeaders.IsEmpty())
    {
        if (CompareSdpInfo(m_objSdpMLines, m_objSdpALines, piSipMsg, m_nEvaluationRule) ==
                SPT_MATCH_NOK)
        {
            IMS_TRACE_D("TriggerPoint :: SDP is not matched", 0, 0, 0);
            return IMS_FALSE;
        }

        return IMS_TRUE;
    }

    IMS_SINT32 nMatchResult = SPT_MATCH_NOK;

    for (IMS_UINT32 i = 0; i < m_objNegatedHeaders.GetSize(); ++i)
    {
        const ISipHeader* piHeader = m_objNegatedHeaders.GetAt(i);

        if (piHeader == IMS_NULL)
        {
            continue;
        }

        nMatchResult = CompareHeaderInMessage(piHeader, piSipMsg, m_nEvaluationRule, IMS_TRUE);

        if (nMatchResult == SPT_MATCH_NOK)
        {
            IMS_TRACE_D("TriggerPoint :: NegatedHeader (%s) is not matched",
                    piHeader->ToString().GetStr(), 0, 0);
            return IMS_FALSE;
        }
    }

    for (IMS_UINT32 i = 0; i < m_objHeaders.GetSize(); ++i)
    {
        const ISipHeader* piHeader = m_objHeaders.GetAt(i);

        if (piHeader == IMS_NULL)
        {
            continue;
        }

        nMatchResult = CompareHeaderInMessage(piHeader, piSipMsg, m_nEvaluationRule);

        if (nMatchResult == SPT_MATCH_NOK)
        {
            IMS_TRACE_D("TriggerPoint :: Header (%s) is not matched", piHeader->ToString().GetStr(),
                    0, 0);
            return IMS_FALSE;
        }
    }

    if (CompareSdpInfo(m_objSdpMLines, m_objSdpALines, piSipMsg, m_nEvaluationRule) ==
            SPT_MATCH_NOK)
    {
        IMS_TRACE_D("TriggerPoint :: SDP is not matched", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
IMS_UINT32 TriggerPoint::GetCount() const
{
    // Method
    IMS_UINT32 nCount = 1;

    // Headers
    nCount += m_objHeaders.GetSize();

    // Negated Headers
    nCount += m_objNegatedHeaders.GetSize();

    // Additional fields

    return nCount;
}

PRIVATE GLOBAL IMS_SINT32 TriggerPoint::CompareHeader(IN const ISipHeader* piHeader,
        IN const ISipHeader* piOtherHeader, IN IMS_SINT32 nEvaluationRule,
        IN IMS_BOOL bConditionNegated /*= IMS_FALSE*/)
{
    if ((piHeader == IMS_NULL) || (piOtherHeader == IMS_NULL))
    {
        return SPT_MATCH_NOK;
    }

    // The below header fields will be evaluated for only SIP parameters ...
    if (IsParameterComparisonRequired(piHeader))
    {
        const ImsList<SipParameter*>& objParameters = piHeader->GetParameters();
        const ImsList<SipParameter*>& objOtherParameters = piOtherHeader->GetParameters();

        if (objParameters.IsEmpty())
        {
            if (objOtherParameters.IsEmpty())
            {
                return (bConditionNegated) ? SPT_MATCH_NOK : SPT_MATCH_OK;
            }
            else
            {
                return (bConditionNegated) ? SPT_MATCH_OK : SPT_MATCH_NOK;
            }
        }

        const SipParameter* pParameter = objParameters.GetAt(0);

        if (pParameter == IMS_NULL)
        {
            return SPT_MATCH_NOK;
        }

        for (IMS_UINT32 i = 0; i < objOtherParameters.GetSize(); ++i)
        {
            const SipParameter* pOtherParameter = objOtherParameters.GetAt(i);

            if (pOtherParameter == IMS_NULL)
            {
                continue;
            }

            if (pParameter->Equals(pOtherParameter))
            {
                if (bConditionNegated)
                {
                    return SPT_MATCH_NOK;
                }
                else
                {
                    return SPT_MATCH_OK;
                }
            }
        }

        if (bConditionNegated)
        {
            return SPT_MATCH_OK;
        }
    }
    else
    {
        IMS_BOOL bEvaluation = IMS_FALSE;

        if ((nEvaluationRule & SPT_SIP_RULE_MATCH) != 0)
        {
            bEvaluation = piHeader->GetValue().EqualsIgnoreCase(piOtherHeader->GetValue());
        }
        else if ((nEvaluationRule & SPT_SIP_RULE_CONTAIN) != 0)
        {
            AString strHeader = piHeader->GetValue().MakeLower();
            AString strOtherHeader = piOtherHeader->GetValue().MakeLower();

            bEvaluation = strOtherHeader.Contains(strHeader);
        }

        if (bEvaluation)
        {
            if (bConditionNegated)
            {
                return SPT_MATCH_NOK;
            }
            else
            {
                return SPT_MATCH_OK;
            }
        }
        else
        {
            if (bConditionNegated)
            {
                return SPT_MATCH_OK;
            }
        }
    }

    return SPT_MATCH_NOK;
}

PRIVATE GLOBAL IMS_SINT32 TriggerPoint::CompareHeaderInMessage(IN const ISipHeader* piHeader,
        IN const ISipMessage* piSipMsg, IN IMS_SINT32 nEvaluationRule,
        IN IMS_BOOL bConditionNegated /* = IMS_FALSE */)
{
    if ((piHeader == IMS_NULL) || (piSipMsg == IMS_NULL))
    {
        return SPT_MATCH_NOK;
    }

    ImsList<AString> objValues = piSipMsg->GetHeaders(piHeader->GetType(), piHeader->GetName());

    if (objValues.IsEmpty())
    {
        if (bConditionNegated)
        {
            return SPT_MATCH_OK;
        }
        else
        {
            return SPT_MATCH_NOK;
        }
    }
    else
    {
        // Header field's presentity
        if (bConditionNegated && (piHeader->GetValue().GetLength() == 0))
        {
            return SPT_MATCH_NOK;
        }
    }

    IMS_SINT32 nMatchResult = SPT_MATCH_NONE;

    for (IMS_UINT32 i = 0; i < objValues.GetSize(); ++i)
    {
        ISipHeader* piOtherHeader;
        const AString& strHeader = objValues.GetAt(i);

        if (piHeader->GetType() != ISipHeader::UNKNOWN)
        {
            piOtherHeader = SipParsingHelper::CreateHeader(piHeader->GetType(), strHeader);
        }
        else
        {
            piOtherHeader = SipParsingHelper::CreateHeader(piHeader->GetName(), strHeader);
        }

        if (piOtherHeader == IMS_NULL)
        {
            continue;
        }

        nMatchResult |= CompareHeader(piHeader, piOtherHeader, nEvaluationRule, bConditionNegated);

        piOtherHeader->Destroy();

        if (bConditionNegated)
        {
            if ((nMatchResult & SPT_MATCH_NOK) == SPT_MATCH_NOK)
            {
                nMatchResult = SPT_MATCH_NOK;
                break;
            }
        }
        else
        {
            if ((nMatchResult & SPT_MATCH_OK) == SPT_MATCH_OK)
            {
                nMatchResult = SPT_MATCH_OK;
                break;
            }
        }
    }

    if (nMatchResult == SPT_MATCH_NONE)
    {
        nMatchResult = SPT_MATCH_NOK;
    }

    return nMatchResult;
}

PRIVATE GLOBAL IMS_SINT32 TriggerPoint::CompareSdpInfo(IN const ImsList<AString>& objMLines,
        IN const ImsList<AString>& objALines, IN const ISipMessage* piSipMsg,
        IN IMS_SINT32 nEvaluationRule, IN IMS_BOOL bConditionNegated /*= IMS_FALSE*/)
{
    if (objMLines.IsEmpty() && objALines.IsEmpty())
    {
        IMS_TRACE_D("TriggerPoint :: No SDP info.", 0, 0, 0);
        return SPT_MATCH_OK;
    }

    const ISipMessageBodyPart* pBodyPart = piSipMsg->GetSdpBodyPart();

    if (pBodyPart == IMS_NULL)
    {
        if (bConditionNegated)
        {
            return SPT_MATCH_OK;
        }
        else
        {
            return SPT_MATCH_NOK;
        }
    }

    AString strBodyPart = pBodyPart->GetContent().ToString();
    AStringArray objBodyPart;
    AStringArray objSelectedLines;
    IMS_SINT32 nMatchResult = SPT_MATCH_NONE;
    IMS_BOOL bEvalRuleContain = ((nEvaluationRule & SPT_SDP_RULE_CONTAIN) != 0);

    SplitLines(strBodyPart, objBodyPart);

    // m-line
    for (IMS_SINT32 i = 0; i < objBodyPart.GetCount(); ++i)
    {
        const AString& strLine = objBodyPart.GetElementAt(i);

        if (!strLine.StartsWith("m=") && !strLine.StartsWith("M="))
        {
            continue;
        }

        objSelectedLines.AddElement(strLine.MakeLower());
    }

    for (IMS_UINT32 i = 0; i < objMLines.GetSize(); ++i)
    {
        IMS_SINT32 nIndex = GetIndexOf(objSelectedLines, objMLines.GetAt(i), bEvalRuleContain);

        if (nIndex != AString::NPOS)
        {
            nMatchResult |= SPT_MATCH_OK;
            objSelectedLines.RemoveElementAt(nIndex);
        }
        else
        {
            nMatchResult |= SPT_MATCH_NOK;
        }
    }

    if (bConditionNegated)
    {
        if ((nMatchResult & SPT_MATCH_OK) == SPT_MATCH_OK)
        {
            return SPT_MATCH_NOK;
        }
    }
    else
    {
        if ((nMatchResult & SPT_MATCH_NOK) == SPT_MATCH_NOK)
        {
            return SPT_MATCH_NOK;
        }
    }

    // a-line
    nMatchResult = SPT_MATCH_NONE;
    objSelectedLines.RemoveAllElements();

    for (IMS_SINT32 i = 0; i < objBodyPart.GetCount(); ++i)
    {
        const AString& strLine = objBodyPart.GetElementAt(i);

        if (!strLine.StartsWith("a=") && !strLine.StartsWith("A="))
        {
            continue;
        }

        objSelectedLines.AddElement(strLine.MakeLower());
    }

    for (IMS_UINT32 i = 0; i < objALines.GetSize(); ++i)
    {
        IMS_SINT32 nIndex = GetIndexOf(objSelectedLines, objALines.GetAt(i), bEvalRuleContain);

        if (nIndex != AString::NPOS)
        {
            nMatchResult |= SPT_MATCH_OK;
            objSelectedLines.RemoveElementAt(nIndex);
        }
        else
        {
            nMatchResult |= SPT_MATCH_NOK;
        }
    }

    if (bConditionNegated)
    {
        if ((nMatchResult & SPT_MATCH_OK) == SPT_MATCH_OK)
        {
            return SPT_MATCH_NOK;
        }
    }
    else
    {
        if ((nMatchResult & SPT_MATCH_NOK) == SPT_MATCH_NOK)
        {
            return SPT_MATCH_NOK;
        }
    }

    return SPT_MATCH_OK;
}

PRIVATE GLOBAL IMS_SINT32 TriggerPoint::GetIndexOf(IN const AStringArray& objSdpLines,
        IN const AString& strToken, IN IMS_BOOL bContain /*= IMS_TRUE*/)
{
    for (IMS_SINT32 i = 0; i < objSdpLines.GetCount(); ++i)
    {
        const AString& strLine = objSdpLines.GetElementAt(i);

        if (bContain)
        {
            if (strLine.Contains(strToken))
            {
                return i;
            }
        }
        else
        {
            if (strLine.EqualsIgnoreCase(strToken))
            {
                return i;
            }
        }
    }

    return AString::NPOS;
}

/**
 * @brief Checks if SIP header parameter needs to be compared for the specified header.
 */
PRIVATE GLOBAL IMS_BOOL TriggerPoint::IsParameterComparisonRequired(IN const ISipHeader* piHeader)
{
    if ((piHeader->GetType() == ISipHeader::ACCEPT_CONTACT) ||
            (piHeader->GetType() == ISipHeader::CONTACT_ANY) ||
            (piHeader->GetType() == ISipHeader::CONTACT_NORMAL) ||
            (piHeader->GetType() == ISipHeader::FEATURE_CAPS))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

/**
 * @brief Splits the message for each lines.
 */
PRIVATE GLOBAL void TriggerPoint::SplitLines(
        IN const AString& strSdp, OUT AStringArray& objSdpLines)
{
    IMS_SINT32 nLineStart = 0;

    while (nLineStart < strSdp.GetLength())
    {
        IMS_SINT32 nEndOfLine = strSdp.GetIndexOf(TextParser::CHAR_LF, nLineStart);

        if (nEndOfLine == AString::NPOS)
        {
            nEndOfLine = strSdp.GetLength();
        }

        IMS_SINT32 nNextLineStart = nEndOfLine + 1;

        if ((nEndOfLine > 0) && (strSdp[nEndOfLine - 1] == TextParser::CHAR_CR))
        {
            --nEndOfLine;
        }

        if (nEndOfLine > nLineStart)
        {
            objSdpLines.AddElement(strSdp.GetSubStr(nLineStart, nEndOfLine - nLineStart));
        }

        nLineStart = nNextLineStart;
    }
}
