/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20110719  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "TextParser.h"
#include "SipHeaderName.h"
#include "ISipMessage.h"
#include "ISipHeader.h"
#include "SipParameter.h"
#include "SipParsingHelper.h"
#include "TriggerPoint.h"

__IMS_TRACE_TAG_IMS__;



PUBLIC
TriggerPoint::TriggerPoint(IN CONST SIPMethod &objMethod_,
        IN IMS_BOOL bMethodNegated_ /* = IMS_FALSE */)
    : nEvaluationRule(SPT_SIP_RULE_MATCH | SPT_SDP_RULE_CONTAIN)
    , bMethodNegated(bMethodNegated_)
    , objMethod(objMethod_)
{
}

PUBLIC
TriggerPoint::TriggerPoint(IN CONST TriggerPoint &objRHS)
    : nEvaluationRule(objRHS.nEvaluationRule)
    , bMethodNegated(objRHS.bMethodNegated)
    , objMethod(objRHS.objMethod)
    , objSDPMLines(objRHS.objSDPMLines)
    , objSDPALines(objRHS.objSDPALines)
{
    for (IMS_UINT32 i = 0; i < objRHS.objHeaders.GetSize(); ++i)
    {
        const ISIPHeader *piHeader = objRHS.objHeaders.GetAt(i);

        if (piHeader == IMS_NULL)
            continue;

        ISIPHeader *piNewHeader = piHeader->Clone();

        if (piNewHeader == IMS_NULL)
        {
            IMS_TRACE_E(0, "Cloning a SIP header failed", 0, 0, 0);
            continue;
        }

        if (!objHeaders.Append(piNewHeader))
        {
            piNewHeader->Destroy();

            IMS_TRACE_E(0, "Adding a new SIP header failed", 0, 0, 0);
        }
    }

    for (IMS_UINT32 i = 0; i < objRHS.objNegatedHeaders.GetSize(); ++i)
    {
        const ISIPHeader *piHeader = objRHS.objNegatedHeaders.GetAt(i);

        if (piHeader == IMS_NULL)
            continue;

        ISIPHeader *piNewHeader = piHeader->Clone();

        if (piNewHeader == IMS_NULL)
        {
            IMS_TRACE_E(0, "Cloning a SIP header failed", 0, 0, 0);
            continue;
        }

        if (!objNegatedHeaders.Append(piNewHeader))
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
TriggerPoint& TriggerPoint::operator=(IN CONST TriggerPoint &objRHS)
{
    //---------------------------------------------------------------------------------------------

    if (this != &objRHS)
    {
        RemoveAllHeaders();

        nEvaluationRule = objRHS.nEvaluationRule;

        bMethodNegated = objRHS.bMethodNegated;
        objMethod = objRHS.objMethod;

        for (IMS_UINT32 i = 0; i < objRHS.objHeaders.GetSize(); ++i)
        {
            const ISIPHeader *piHeader = objRHS.objHeaders.GetAt(i);

            if (piHeader == IMS_NULL)
                continue;

            ISIPHeader *piNewHeader = piHeader->Clone();

            if (piNewHeader == IMS_NULL)
            {
                IMS_TRACE_E(0, "Cloning a SIP header failed", 0, 0, 0);
                continue;
            }

            if (!objHeaders.Append(piNewHeader))
            {
                piNewHeader->Destroy();

                IMS_TRACE_E(0, "Adding a new SIP header failed", 0, 0, 0);
            }
        }

        for (IMS_UINT32 i = 0; i < objRHS.objNegatedHeaders.GetSize(); ++i)
        {
            const ISIPHeader *piHeader = objRHS.objNegatedHeaders.GetAt(i);

            if (piHeader == IMS_NULL)
                continue;

            ISIPHeader *piNewHeader = piHeader->Clone();

            if (piNewHeader == IMS_NULL)
            {
                IMS_TRACE_E(0, "Cloning a SIP header failed", 0, 0, 0);
                continue;
            }

            if (!objNegatedHeaders.Append(piNewHeader))
            {
                piNewHeader->Destroy();

                IMS_TRACE_E(0, "Adding a new SIP header failed", 0, 0, 0);
            }
        }

        objSDPMLines = objRHS.objSDPMLines;
        objSDPALines = objRHS.objSDPALines;
    }

    return (*this);
}

/*

Remarks

*/
PUBLIC
IMS_BOOL TriggerPoint::AddHeader(IN IMS_SINT32 nType, IN CONST AString &strValue,
        IN CONST AString &strName /* = AString::ConstNull() */,
        IN IMS_BOOL bHeaderNegated /* = IMS_FALSE */)
{
    ISIPHeader *piHeader = IMS_NULL;

    //---------------------------------------------------------------------------------------------

    if (nType != ISIPHeader::UNKNOWN)
    {
        piHeader = SIPParsingHelper::CreateHeader(nType, strValue);
    }
    else
    {
        piHeader = SIPParsingHelper::CreateHeader(strName, strValue);
    }

    if (piHeader == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a SIP header (%d, %s, %s) failed",
                nType, strName.GetStr(), strValue.GetStr());
        return IMS_FALSE;
    }

    if (!bHeaderNegated)
    {
        if (!objHeaders.Append(piHeader))
        {
            piHeader->Destroy();

            IMS_TRACE_E(0, "Adding a SIP header (%d, %s, %s) failed",
                    nType, strName.GetStr(), strValue.GetStr());
            return IMS_FALSE;
        }
    }
    else
    {
        if (!objNegatedHeaders.Append(piHeader))
        {
            piHeader->Destroy();

            IMS_TRACE_E(0, "Adding a SIP header (%d, %s, %s) to failed",
                    nType, strName.GetStr(), strValue.GetStr());
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL TriggerPoint::AddSDPInfo(IN CONST IMS_CHAR cName, IN CONST AString &strValue)
{
    //---------------------------------------------------------------------------------------------

    if (cName == 'm')
    {
        objSDPMLines.Append(strValue);
    }
    else if (cName == 'a')
    {
        objSDPALines.Append(strValue);
    }
    else
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
void TriggerPoint::RemoveAllHeaders()
{
    //---------------------------------------------------------------------------------------------

    if (!objHeaders.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objHeaders.GetSize(); ++i)
        {
            ISIPHeader *piHeader = objHeaders.GetAt(i);

            if (piHeader == IMS_NULL)
                continue;

            piHeader->Destroy();
        }

        objHeaders.Clear();
    }

    if (!objNegatedHeaders.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objNegatedHeaders.GetSize(); ++i)
        {
            ISIPHeader *piHeader = objNegatedHeaders.GetAt(i);

            if (piHeader == IMS_NULL)
                continue;

            piHeader->Destroy();
        }

        objNegatedHeaders.Clear();
    }
}

/*

Remarks

*/
PUBLIC
void TriggerPoint::RemoveAllSDPInfo()
{
    //---------------------------------------------------------------------------------------------

    objSDPMLines.Clear();
    objSDPALines.Clear();
}

/*

Remarks

*/
PUBLIC
void TriggerPoint::SetEvaluationRule(IN IMS_SINT32 nRule)
{
    //---------------------------------------------------------------------------------------------

    nEvaluationRule = nRule;
}

/*

Remarks

*/
PRIVATE
IMS_BOOL TriggerPoint::Evaluate(IN CONST ISIPMessage *piSIPMsg) const
{
    //---------------------------------------------------------------------------------------------

    if (piSIPMsg == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!bMethodNegated)
    {
        if (!objMethod.Equals(piSIPMsg->GetMethod()))
        {
            return IMS_FALSE;
        }
    }
    else
    {
        if (objMethod.Equals(piSIPMsg->GetMethod()))
        {
            return IMS_FALSE;
        }
    }

    // In case the method is only matched ...
    if (objHeaders.IsEmpty() && objNegatedHeaders.IsEmpty())
    {
        if (CompareSDPInfo(objSDPMLines, objSDPALines,
                piSIPMsg, nEvaluationRule) == SPT_MATCH_NOK)
        {
            IMS_TRACE_D("TriggerPoint :: SDP is not matched", 0, 0, 0);
            return IMS_FALSE;
        }

        return IMS_TRUE;
    }

    IMS_SINT32 nMatchResult = SPT_MATCH_NOK;

    for (IMS_UINT32 i = 0; i < objNegatedHeaders.GetSize(); ++i)
    {
        const ISIPHeader *piHeader = objNegatedHeaders.GetAt(i);

        if (piHeader == IMS_NULL)
            continue;

        nMatchResult = CompareHeaderInMessage(piHeader, piSIPMsg, nEvaluationRule, IMS_TRUE);

        if (nMatchResult == SPT_MATCH_NOK)
        {
            IMS_TRACE_D("TriggerPoint :: NegatedHeader (%s) is not matched",
                    piHeader->ToString().GetStr(), 0, 0);
            return IMS_FALSE;
        }
    }

    for (IMS_UINT32 i = 0; i < objHeaders.GetSize(); ++i)
    {
        const ISIPHeader *piHeader = objHeaders.GetAt(i);

        if (piHeader == IMS_NULL)
            continue;

        nMatchResult = CompareHeaderInMessage(piHeader, piSIPMsg, nEvaluationRule);

        if (nMatchResult == SPT_MATCH_NOK)
        {
            IMS_TRACE_D("TriggerPoint :: Header (%s) is not matched",
                    piHeader->ToString().GetStr(), 0, 0);
            return IMS_FALSE;
        }
    }

    if (CompareSDPInfo(objSDPMLines, objSDPALines,
            piSIPMsg, nEvaluationRule) == SPT_MATCH_NOK)
    {
        IMS_TRACE_D("TriggerPoint :: SDP is not matched", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
IMS_UINT32 TriggerPoint::GetCount() const
{
    // Method
    IMS_UINT32 nCount = 1;

    //---------------------------------------------------------------------------------------------

    // Headers
    nCount += objHeaders.GetSize();

    // Negated Headers
    nCount += objNegatedHeaders.GetSize();

    // Additional fields

    return nCount;
}

/*

Remarks

*/
PRIVATE GLOBAL
IMS_SINT32 TriggerPoint::CompareHeader(IN CONST ISIPHeader *piHeader,
        IN CONST ISIPHeader *piOtherHeader, IN IMS_SINT32 nEvaluationRule,
        IN IMS_BOOL bConditionNegated /* = IMS_FALSE */)
{
    //---------------------------------------------------------------------------------------------

    if ((piHeader == IMS_NULL) || (piOtherHeader == IMS_NULL))
        return SPT_MATCH_NOK;

    // The below header fields will be evaluated for only SIP parameters ...
    if (IsParameterComparisonRequired(piHeader))
    {
        const IMSList<SIPParameter*> &objParameters = piHeader->GetParameters();
        const IMSList<SIPParameter*> &objOtherParameters = piOtherHeader->GetParameters();

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

        const SIPParameter *pParameter = objParameters.GetAt(0);

        if (pParameter == IMS_NULL)
        {
            return SPT_MATCH_NOK;
        }

        for (IMS_UINT32 i = 0; i < objOtherParameters.GetSize(); ++i)
        {
            const SIPParameter *pOtherParameter = objOtherParameters.GetAt(i);

            if (pOtherParameter == IMS_NULL)
                continue;

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

/*

Remarks

*/
PRIVATE GLOBAL
IMS_SINT32 TriggerPoint::CompareHeaderInMessage(IN CONST ISIPHeader *piHeader,
        IN CONST ISIPMessage *piSIPMsg, IN IMS_SINT32 nEvaluationRule,
        IN IMS_BOOL bConditionNegated /* = IMS_FALSE */)
{
    //---------------------------------------------------------------------------------------------

    if ((piHeader == IMS_NULL) || (piSIPMsg == IMS_NULL))
    {
        return SPT_MATCH_NOK;
    }

    IMSList<AString> objValues = piSIPMsg->GetHeaders(piHeader->GetType(), piHeader->GetName());

    if (objValues.IsEmpty())
    {
        if (bConditionNegated)
            return SPT_MATCH_OK;
        else
            return SPT_MATCH_NOK;
    }
    else
    {
        // Header field's presentity
        if (bConditionNegated && (piHeader->GetValue().GetLength() == 0))
        {
            return SPT_MATCH_NOK;
        }
    }

    ISIPHeader *piOtherHeader = IMS_NULL;
    IMS_SINT32 nMatchResult = SPT_MATCH_NONE;

    for (IMS_UINT32 i = 0; i < objValues.GetSize(); ++i)
    {
        const AString &strHeader = objValues.GetAt(i);

        if (piHeader->GetType() != ISIPHeader::UNKNOWN)
        {
            piOtherHeader = SIPParsingHelper::CreateHeader(piHeader->GetType(), strHeader);
        }
        else
        {
            piOtherHeader = SIPParsingHelper::CreateHeader(piHeader->GetName(), strHeader);
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

/*

Remarks

*/
PRIVATE GLOBAL
IMS_SINT32 TriggerPoint::CompareSDPInfo(IN CONST IMSList<AString>& objMLines,
        IN CONST IMSList<AString>& objALines, IN CONST ISIPMessage *piSIPMsg,
        IN IMS_SINT32 nEvaluationRule, IN IMS_BOOL bConditionNegated /* = IMS_FALSE */)
{
    if (objMLines.IsEmpty() && objALines.IsEmpty())
    {
        IMS_TRACE_D("TriggerPoint :: No SDP info.", 0, 0, 0);
        return SPT_MATCH_OK;
    }

    ISIPMessageBodyPart *pBodyPart = piSIPMsg->GetSDPBodyPart();

    if (pBodyPart == IMS_NULL)
    {
        if (bConditionNegated)
            return SPT_MATCH_OK;
        else
            return SPT_MATCH_NOK;
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
            return SPT_MATCH_NOK;
    }
    else
    {
        if ((nMatchResult & SPT_MATCH_NOK) == SPT_MATCH_NOK)
            return SPT_MATCH_NOK;
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
            return SPT_MATCH_NOK;
    }
    else
    {
        if ((nMatchResult & SPT_MATCH_NOK) == SPT_MATCH_NOK)
            return SPT_MATCH_NOK;
    }

    return SPT_MATCH_OK;
}

/*

Remarks

*/
PRIVATE GLOBAL
IMS_SINT32 TriggerPoint::GetIndexOf(IN CONST AStringArray &objSDPLines, IN CONST AString &strToken,
        IN IMS_BOOL bContain /* = IMS_TRUE */)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_SINT32 i = 0; i < objSDPLines.GetCount(); ++i)
    {
        const AString &strLine = objSDPLines.GetElementAt(i);

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

/*
 Checks if SIP header parameter needs to be compared for the specified header.

Remarks

*/
PRIVATE GLOBAL
IMS_BOOL TriggerPoint::IsParameterComparisonRequired(IN CONST ISIPHeader *piHeader)
{
    if ((piHeader->GetType() == ISIPHeader::ACCEPT_CONTACT)
            || (piHeader->GetType() == ISIPHeader::CONTACT_ANY)
            || (piHeader->GetType() == ISIPHeader::CONTACT_NORMAL)
            || piHeader->GetName().Equals(SIPHeaderName::FEATURE_CAPS))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

/*
 Splits the message for each lines.

Remarks

*/
PRIVATE GLOBAL
void TriggerPoint::SplitLines(IN CONST AString &strSDP, OUT AStringArray &objSDPLines)
{
    IMS_SINT32 nLineStart = 0;
    IMS_SINT32 nEOL = 0;
    IMS_SINT32 nNextLineStart = 0;

    //---------------------------------------------------------------------------------------------

    while (nLineStart < strSDP.GetLength())
    {
        nEOL = strSDP.GetIndexOf(TextParser::CHAR_LF, nLineStart);

        if (nEOL == AString::NPOS)
        {
            nEOL = strSDP.GetLength();
        }

        nNextLineStart = nEOL + 1;

        if ((nEOL > 0) && (strSDP[nEOL - 1] == TextParser::CHAR_CR))
        {
            --nEOL;
        }

        if (nEOL > nLineStart)
        {
            objSDPLines.AddElement(strSDP.GetSubStr(nLineStart, nEOL - nLineStart));
        }

        nLineStart = nNextLineStart;
    }
}
