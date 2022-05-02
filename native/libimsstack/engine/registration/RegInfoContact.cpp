/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100720  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceUtil.h"
#include "AStringBuffer.h"
#include "INamedNodeMap.h"
#include "INode.h"
#include "SipDebug.h"
#include "RegInfoConst.h"
#include "RegInfoContact.h"

__IMS_TRACE_TAG_REG__;



PUBLIC
RegInfoContact::RegInfoContact()
    : strId(AString::ConstNull())
    , nState(STATE_CREATED)
    , nEvent(EVENT_UNREGISTERED)
    , nDurationRegistered(0)
    , nExpires(0)
    , nRetryAfter(0)
    , strDisplayName(AString::ConstNull())
    , strQValue(AString::ConstNull())
    , strCallId(AString::ConstNull())
    , nCSeq(0)
    , strPubGRUU(AString::ConstNull())
    , objTempGRUU(TempGRUU())
    , objUnknownParameters(IMSMap<AString, AString>())
{
}

PUBLIC VIRTUAL
RegInfoContact::~RegInfoContact()
{
    IMS_TRACE_D("Destructor :: uri=%s, id=%s",
            SIPDebug::GetUri1(objURI.ToString()).GetStr(), strId.GetStr(), 0);
}

PUBLIC VIRTUAL
IMS_UINT32 RegInfoContact::GetCSeq() const
{
    //---------------------------------------------------------------------------------------------

    return nCSeq;
}

PUBLIC VIRTUAL
const AString& RegInfoContact::GetDisplayName() const
{
    //---------------------------------------------------------------------------------------------

    return strDisplayName;
}

PUBLIC VIRTUAL
IMS_SINT32 RegInfoContact::GetEvent() const
{
    //---------------------------------------------------------------------------------------------

    return nEvent;
}

PUBLIC VIRTUAL
IMS_UINT32 RegInfoContact::GetExpiresValue() const
{
    //---------------------------------------------------------------------------------------------

    return nExpires;
}

PUBLIC VIRTUAL
IMS_UINT32 RegInfoContact::GetFirstCSeq() const
{
    //---------------------------------------------------------------------------------------------

    return objTempGRUU.nFirstCSeq;
}

PUBLIC VIRTUAL
const AString& RegInfoContact::GetPublicGRUU() const
{
    //---------------------------------------------------------------------------------------------

    return strPubGRUU;
}

PUBLIC VIRTUAL
const AString& RegInfoContact::GetTemporaryGRUU() const
{
    //---------------------------------------------------------------------------------------------

    return objTempGRUU.strGRUU;
}

PUBLIC VIRTUAL
const AString& RegInfoContact::GetQValue() const
{
    //---------------------------------------------------------------------------------------------

    return strQValue;
}

PUBLIC VIRTUAL
IMS_UINT32 RegInfoContact::GetRetryAfterValue() const
{
    //---------------------------------------------------------------------------------------------

    return nRetryAfter;
}

PUBLIC VIRTUAL
IMS_SINT32 RegInfoContact::GetState() const
{
    //---------------------------------------------------------------------------------------------

    return nState;
}

PUBLIC VIRTUAL
const AString& RegInfoContact::GetUnknownParameter(IN CONST AString &strName) const
{
    IMS_SLONG nIndex = objUnknownParameters.GetIndexOfKey(strName);

    //---------------------------------------------------------------------------------------------

    if (nIndex < 0)
    {
        IMS_TRACE_D("Unknown-param (%s) does not exist",
                strName.GetStr(), 0, 0);
        return AString::ConstNull();
    }

    const AString &strValue = objUnknownParameters.GetValueAt(nIndex);

    if (strValue.GetLength() == 0)
    {
        return AString::ConstEmpty();
    }

    return strValue;
}

PUBLIC VIRTUAL
const IMSMap<AString, AString>& RegInfoContact::GetUnknownParameters() const
{
    //---------------------------------------------------------------------------------------------

    return objUnknownParameters;
}

PUBLIC VIRTUAL
const SIPAddress& RegInfoContact::GetURI() const
{
    //---------------------------------------------------------------------------------------------

    return objURI;
}

PUBLIC
IMS_BOOL RegInfoContact::Equals(IN INode *piNode) const
{
    //---------------------------------------------------------------------------------------------

    if (piNode == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!piNode->GetLocalName().EqualsIgnoreCase(RegInfoConst::ELEMENT_CONTACT))
    {
        return IMS_FALSE;
    }

    // Find "id" attribute & compares it
    INamedNodeMap *piNodeMap = piNode->GetAttributes();

    if (piNodeMap == IMS_NULL)
    {
        return IMS_FALSE;
    }

    INode *piNode_Id = piNodeMap->GetNamedItem(RegInfoConst::ATTR_ID);

    if (piNode_Id == IMS_NULL)
    {
        piNode->DestroyNamedNodeMap(piNodeMap);
        return IMS_FALSE;
    }

    if (!strId.EqualsIgnoreCase(piNode_Id->GetNodeValue()))
    {
        piNode->DestroyNamedNodeMap(piNodeMap);
        return IMS_FALSE;
    }

    piNode->DestroyNamedNodeMap(piNodeMap);

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL RegInfoContact::Update(IN INode *piNode)
{
    //---------------------------------------------------------------------------------------------

    if (piNode == IMS_NULL)
    {
        return IMS_FALSE;
    }

    INamedNodeMap *piNodeMap = piNode->GetAttributes();

    if (piNodeMap == IMS_NULL)
    {
        IMS_TRACE_E(0, "No attributes in 'registration'", 0, 0, 0);
        return IMS_FALSE;
    }

    if (nState == STATE_CREATED)
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
        INode *piChildNode = piNode->GetFirstChild();

        while (piChildNode != IMS_NULL)
        {
            const AString& strNodeName = piChildNode->GetLocalName();

            if (strNodeName.EqualsIgnoreCase(RegInfoConst::ELEMENT_URI))
            {
                if (!SetURI(piChildNode))
                {
                    piNode->DestroyNamedNodeMap(piNodeMap);
                    return IMS_FALSE;
                }
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
                SetPublicGRUU(piChildNode);
            }
            else if (strNodeName.EqualsIgnoreCase(RegInfoConst::ELEMENT_TEMP_GRUU))
            {
                SetTemporaryGRUU(piChildNode);
            }
            else
            {
                IMS_TRACE_D("Unknown element (%s)", strNodeName.GetStr(), 0, 0);
            }

            piChildNode = piChildNode->GetNextSibling();
        }

        if (!IMS_UTIL_SYS_PROP_IS_SERVER_INFO_HIDDEN_IN_LOG())
        {
            IMS_TRACE_I("Contact :: uri=%s, id=%s",
                    SIPDebug::GetUri1(objURI.ToString()).GetStr(), strId.GetStr(), 0);
        }
    }
    else
    {
        // Updates only the state & Contacts

        // "display-name" element
        INode *piChildNode = piNode->GetFirstChild();

        while (piChildNode != IMS_NULL)
        {
            const AString& strNodeName = piChildNode->GetLocalName();

            if (strNodeName.EqualsIgnoreCase(RegInfoConst::ELEMENT_DISPLAY_NAME))
            {
                SetDisplayName(piChildNode);
            }
            else if (strNodeName.EqualsIgnoreCase(RegInfoConst::ELEMENT_PUB_GRUU))
            {
                SetPublicGRUU(piChildNode);
            }
            else if (strNodeName.EqualsIgnoreCase(RegInfoConst::ELEMENT_TEMP_GRUU))
            {
                SetTemporaryGRUU(piChildNode);
            }
            else
            {
                IMS_TRACE_D("Unknown element (%s)", strNodeName.GetStr(), 0, 0);
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

void RegInfoContact::DisplayRegInfo(IN const AString& strTag/* = AString::ConstNull()*/)
{
    static const IMS_CHAR *pszState[] =
    {
        "CREATED",
        "ACTIVE",
        "TERMINATED",
    };

    static const IMS_CHAR *pszEvent[] =
    {
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

    //---------------------------------------------------------------------------------------------

    AStringBuffer objLog(512);

    objLog.Append("id=").Append(strId);
    objLog.Append(", state=").Append(pszState[nState]);
    objLog.Append(", event=").Append(pszEvent[nEvent]);
    objLog.Append(", duration-registered=").Append(nDurationRegistered);
    objLog.Append(", expires=").Append(nExpires);
    objLog.Append(", callid=").Append(SIPDebug::GetCharA1(strCallId.GetStr(), 8, '@'));
    objLog.Append(", cseq=").Append(nCSeq);
    objLog.Append(", retry-after=").Append(nRetryAfter);
    objLog.Append(", q-value=").Append(strQValue);
    objLog.Append(", first-cseq=").Append(objTempGRUU.nFirstCSeq);

    if (!IMS_UTIL_SYS_PROP_IS_SERVER_INFO_HIDDEN_IN_LOG())
    {
        objLog.Append(", display-name=").Append(SIPDebug::GetCharA1(strDisplayName.GetStr(), 6));
        objLog.Append(", uri=").Append(SIPDebug::GetUri1(objURI.ToString()));
        objLog.Append(", pub-gruu=").Append(SIPDebug::GetUri1(strPubGRUU));
        objLog.Append(", temp-gruu=").Append(SIPDebug::GetUri1(objTempGRUU.strGRUU));
    }

    objLog.Append(", unknown-param-count=").Append(objUnknownParameters.GetSize());

    IMS_TRACE_I("CON(%s) :: %s", strTag.GetStr(), objLog.GetCharString(), 0);

    for (IMS_UINT32 i = 0; i < objUnknownParameters.GetSize(); ++i)
    {
        const AString &strName = objUnknownParameters.GetKeyAt(i);
        const AString &strValue = objUnknownParameters.GetValueAt(i);

        IMS_TRACE_D("(%s) unknown-param :: %s=%s",
                strTag.GetStr(), strName.GetStr(), SIPDebug::GetCharA1(strValue.GetStr(), 6));
    }
}

PRIVATE
void RegInfoContact::SetCallId(IN INamedNodeMap *piNodeMap)
{
    INode *piNode = piNodeMap->GetNamedItem(RegInfoConst::ATTR_CALLID);

    //---------------------------------------------------------------------------------------------

    if (piNode == IMS_NULL)
    {
        IMS_TRACE_D("No 'callid' attribute", 0, 0, 0);
        return;
    }

    strCallId = piNode->GetNodeValue();
}

PRIVATE
void RegInfoContact::SetCSeq(IN INamedNodeMap *piNodeMap)
{
    INode *piNode = piNodeMap->GetNamedItem(RegInfoConst::ATTR_CSEQ);

    //---------------------------------------------------------------------------------------------

    if (piNode == IMS_NULL)
    {
        IMS_TRACE_D("No 'cseq' attribute", 0, 0, 0);
        return;
    }

    const AString& strCSeq = piNode->GetNodeValue();
    IMS_BOOL bOK = IMS_FALSE;
    IMS_UINT32 nNewCSeq = strCSeq.ToUInt32(&bOK);

    if (!bOK)
    {
        return;
    }

    nCSeq = nNewCSeq;
}

PRIVATE
void RegInfoContact::SetDurationRegistered(IN INamedNodeMap *piNodeMap)
{
    INode *piNode = piNodeMap->GetNamedItem(RegInfoConst::ATTR_DURATION_REGISTERED);

    //---------------------------------------------------------------------------------------------

    if (piNode == IMS_NULL)
    {
        IMS_TRACE_D("No 'duration-registered' attribute", 0, 0, 0);
        return;
    }

    const AString& strDurationRegistered = piNode->GetNodeValue();
    IMS_BOOL bOK = IMS_FALSE;
    IMS_UINT32 nNewDurationRegistered = strDurationRegistered.ToUInt32(&bOK);

    if (!bOK)
    {
        return;
    }

    nDurationRegistered = nNewDurationRegistered;
}

PRIVATE
IMS_BOOL RegInfoContact::SetEvent(IN INamedNodeMap *piNodeMap)
{
    INode *piNode = piNodeMap->GetNamedItem(RegInfoConst::ATTR_EVENT);

    //---------------------------------------------------------------------------------------------

    if (piNode == IMS_NULL)
    {
        IMS_TRACE_E(0, "Can't find 'event' attribute", 0, 0, 0);
        return IMS_FALSE;
    }

    const AString& strEvent = piNode->GetNodeValue();

    if (strEvent.EqualsIgnoreCase(RegInfoConst::ATTR_EVENT_REGISTERED))
    {
        nEvent = EVENT_REGISTERED;
    }
    else if (strEvent.EqualsIgnoreCase(RegInfoConst::ATTR_EVENT_CREATED))
    {
        nEvent = EVENT_CREATED;
    }
    else if (strEvent.EqualsIgnoreCase(RegInfoConst::ATTR_EVENT_REFRESHED))
    {
        nEvent = EVENT_REFRESHED;
    }
    else if (strEvent.EqualsIgnoreCase(RegInfoConst::ATTR_EVENT_SHORTENED))
    {
        nEvent = EVENT_SHORTENED;
    }
    else if (strEvent.EqualsIgnoreCase(RegInfoConst::ATTR_EVENT_EXPIRED))
    {
        nEvent = EVENT_EXPIRED;
    }
    else if (strEvent.EqualsIgnoreCase(RegInfoConst::ATTR_EVENT_DEACTIVATED))
    {
        nEvent = EVENT_DEACTIVATED;
    }
    else if (strEvent.EqualsIgnoreCase(RegInfoConst::ATTR_EVENT_PROBATION))
    {
        nEvent = EVENT_PROBATION;
    }
    else if (strEvent.EqualsIgnoreCase(RegInfoConst::ATTR_EVENT_UNREGISTERED))
    {
        nEvent = EVENT_UNREGISTERED;
    }
    else if (strEvent.EqualsIgnoreCase(RegInfoConst::ATTR_EVENT_REJECTED))
    {
        nEvent = EVENT_REJECTED;
    }

    return IMS_TRUE;
}

PRIVATE
void RegInfoContact::SetExpiresValue(IN INamedNodeMap *piNodeMap)
{
    // If 'event' is "shortened", then it MUST be present
    INode *piNode = piNodeMap->GetNamedItem(RegInfoConst::ATTR_EXPIRES);

    //---------------------------------------------------------------------------------------------

    if (piNode == IMS_NULL)
    {
        IMS_TRACE_D("No 'expires' attribute", 0, 0, 0);
        return;
    }

    const AString& strExpires = piNode->GetNodeValue();
    IMS_BOOL bOK = IMS_FALSE;
    IMS_UINT32 nNewExpires = strExpires.ToUInt32(&bOK);

    if (!bOK)
    {
        return;
    }

    nExpires = nNewExpires;
}

PRIVATE
IMS_BOOL RegInfoContact::SetId(IN INamedNodeMap *piNodeMap)
{
    INode *piNode = piNodeMap->GetNamedItem(RegInfoConst::ATTR_ID);

    //---------------------------------------------------------------------------------------------

    if (piNode == IMS_NULL)
    {
        IMS_TRACE_E(0, "Can't find 'id' attribute", 0, 0, 0);
        return IMS_FALSE;
    }

    strId = piNode->GetNodeValue();

    return IMS_TRUE;
}

PRIVATE
void RegInfoContact::SetQValue(IN INamedNodeMap *piNodeMap)
{
    INode *piNode = piNodeMap->GetNamedItem(RegInfoConst::ATTR_Q);

    //---------------------------------------------------------------------------------------------

    if (piNode == IMS_NULL)
    {
        IMS_TRACE_D("No 'q' attribute", 0, 0, 0);
        return;
    }

    strQValue = piNode->GetNodeValue();
}

PRIVATE
void RegInfoContact::SetRetryAfterValue(IN INamedNodeMap *piNodeMap)
{
    // If 'event' is "probation", then it MUST be present.
    INode *piNode = piNodeMap->GetNamedItem(RegInfoConst::ATTR_RETRY_AFTER);

    //---------------------------------------------------------------------------------------------

    if (piNode == IMS_NULL)
    {
        IMS_TRACE_D("No 'retry-after' attribute", 0, 0, 0);
        return;
    }

    const AString& strRetryAfter = piNode->GetNodeValue();
    IMS_BOOL bOK = IMS_FALSE;
    IMS_UINT32 nNewRetryAfter = strRetryAfter.ToUInt32(&bOK);

    if (!bOK)
    {
        return;
    }

    nRetryAfter = nNewRetryAfter;
}

PRIVATE
IMS_BOOL RegInfoContact::SetState(IN INamedNodeMap *piNodeMap)
{
    INode *piNode = piNodeMap->GetNamedItem(RegInfoConst::ATTR_STATE);

    //---------------------------------------------------------------------------------------------

    if (piNode == IMS_NULL)
    {
        IMS_TRACE_E(0, "Can't find 'state' attribute", 0, 0, 0);
        return IMS_FALSE;
    }

    const AString& strState = piNode->GetNodeValue();

    if (strState.EqualsIgnoreCase(RegInfoConst::ATTR_STATE_ACTIVE))
    {
        nState = STATE_ACTIVE;
    }
    else if (strState.EqualsIgnoreCase(RegInfoConst::ATTR_STATE_TERMINATED))
    {
        nState = STATE_TERMINATED;
    }

    return IMS_TRUE;
}

PRIVATE
void RegInfoContact::SetDisplayName(IN INode *piNode)
{
    //---------------------------------------------------------------------------------------------

    if (piNode == IMS_NULL)
    {
        return;
    }

    INode *piContent = piNode->GetFirstChild();

    strDisplayName = piContent->GetNodeValue();
}

PRIVATE
void RegInfoContact::SetPublicGRUU(IN INode *piNode)
{
    INamedNodeMap *piNodeMap = piNode->GetAttributes();

    //---------------------------------------------------------------------------------------------

    if (piNodeMap == IMS_NULL)
    {
        return;
    }

    INode *piAttrNode = piNodeMap->GetNamedItem(RegInfoConst::ATTR_URI);

    if (piAttrNode == IMS_NULL)
    {
        piNode->DestroyNamedNodeMap(piNodeMap);
        IMS_TRACE_D("Pub-GRUU :: Can't find 'uri' attribute", 0, 0, 0);
        return;
    }

    strPubGRUU = piAttrNode->GetNodeValue();

    piNode->DestroyNamedNodeMap(piNodeMap);
}

PRIVATE
void RegInfoContact::SetTemporaryGRUU(IN INode *piNode)
{
    INamedNodeMap *piNodeMap = piNode->GetAttributes();

    //---------------------------------------------------------------------------------------------

    if (piNodeMap == IMS_NULL)
    {
        return;
    }

    INode *piAttrNode = piNodeMap->GetNamedItem(RegInfoConst::ATTR_URI);

    if (piAttrNode == IMS_NULL)
    {
        piNode->DestroyNamedNodeMap(piNodeMap);
        IMS_TRACE_D("Temp-GRUU :: Can't find 'uri' attribute", 0, 0, 0);
        return;
    }

    objTempGRUU.strGRUU = piAttrNode->GetNodeValue();

    piAttrNode = piNodeMap->GetNamedItem(RegInfoConst::ATTR_FIRST_CSEQ);

    if (piAttrNode == IMS_NULL)
    {
        objTempGRUU.nFirstCSeq = 0;
        piNode->DestroyNamedNodeMap(piNodeMap);

        IMS_TRACE_D("Temp-GRUU :: Can't find 'first-cseq' attribute", 0, 0, 0);
        return;
    }

    const AString& strFirstCSeq = piAttrNode->GetNodeValue();
    IMS_BOOL bOK = IMS_FALSE;
    IMS_UINT32 nNewFirstCSeq = strFirstCSeq.ToUInt32(&bOK);

    if (!bOK)
    {
        objTempGRUU.nFirstCSeq = 0;
        piNode->DestroyNamedNodeMap(piNodeMap);
        return;
    }

    objTempGRUU.nFirstCSeq = nNewFirstCSeq;

    piNode->DestroyNamedNodeMap(piNodeMap);
}

PRIVATE
void RegInfoContact::SetUnknownParameter(IN INode *piNode)
{
    INamedNodeMap *piNodeMap = piNode->GetAttributes();

    //---------------------------------------------------------------------------------------------

    if (piNodeMap == IMS_NULL)
    {
        return;
    }

    INode *piNameNode = piNodeMap->GetNamedItem(RegInfoConst::ATTR_NAME);

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
    INode *piContent = piNode->GetFirstChild();

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
                    SIPDebug::GetCharA1(strValue.GetStr(), 6), 0);
        }
    }

    objUnknownParameters.SetValue(strName, strValue);
}

PRIVATE
IMS_BOOL RegInfoContact::SetURI(IN INode *piNode)
{
    //---------------------------------------------------------------------------------------------

    if (piNode == IMS_NULL)
    {
        IMS_TRACE_E(0, "No uri element", 0, 0, 0);
        return IMS_FALSE;
    }

    INode *piContent = piNode->GetFirstChild();

    if (piContent == IMS_NULL)
    {
        IMS_TRACE_E(0, "No content for uri", 0, 0, 0);
        return IMS_FALSE;
    }

    const AString& strURI = piContent->GetNodeValue();

    if (!objURI.Create(strURI))
    {
        IMS_TRACE_E(0, "Creating a Contact URI (%s) failed",
                SIPDebug::GetUri1(strURI).GetStr(), 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}
