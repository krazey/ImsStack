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
#include "INamedNodeMap.h"
#include "INode.h"
#include "SipDebug.h"
#include "RegInfoConst.h"
#include "RegInfoRegistration.h"

__IMS_TRACE_TAG_REG__;



PUBLIC
RegInfoRegistration::RegInfoRegistration()
    : strId(AString::ConstNull())
    , nState(STATE_CREATED)
    , objContacts(IMSList<RegInfoContact*>())
{
}

PUBLIC VIRTUAL
RegInfoRegistration::~RegInfoRegistration()
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objContacts.GetSize(); ++i)
    {
        RegInfoContact *pContact = objContacts.GetAt(i);

        if (pContact != IMS_NULL)
        {
            delete pContact;
        }
    }

    IMS_TRACE_D("Destructor :: aor=%s, id=%s",
            SIPDebug::GetUri1(objAOR.ToString()).GetStr(), strId.GetStr(), 0);
}

PUBLIC VIRTUAL
const SIPAddress& RegInfoRegistration::GetAOR() const
{
    //---------------------------------------------------------------------------------------------

    return objAOR;
}

PUBLIC VIRTUAL
IRegInfoContact* RegInfoRegistration::GetContact(IN CONST SIPAddress &objContactUri) const
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objContacts.GetSize(); ++i)
    {
        RegInfoContact *pContact = objContacts.GetAt(i);

        if (objContactUri.Equals(pContact->GetURI()))
        {
            return pContact;
        }
    }

    return IMS_NULL;
}

PUBLIC VIRTUAL
IMSList<IRegInfoContact*> RegInfoRegistration::GetContacts() const
{
    IMSList<IRegInfoContact*> objIContacts;

    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objContacts.GetSize(); ++i)
    {
        objIContacts.Append(objContacts.GetAt(i));
    }

    return objIContacts;
}

PUBLIC VIRTUAL
RegInfoContact* RegInfoRegistration::GetPriorContact() const
{
    //---------------------------------------------------------------------------------------------

    if (objContacts.IsEmpty())
    {
        return IMS_NULL;
    }

    RegInfoContact *pContact = objContacts.GetAt(0);

    for (IMS_UINT32 i = 1; i < objContacts.GetSize(); ++i)
    {
        RegInfoContact *pTmpContact = objContacts.GetAt(i);

        if (pContact->GetQValue() < pTmpContact->GetQValue())
        {
            pContact = pTmpContact;
        }
    }

    return pContact;
}

PUBLIC VIRTUAL
IMS_SINT32 RegInfoRegistration::GetState() const
{
    //---------------------------------------------------------------------------------------------

    return nState;
}

PUBLIC
IMS_BOOL RegInfoRegistration::Equals(IN INode *piNode) const
{
    //---------------------------------------------------------------------------------------------

    if (piNode == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!piNode->GetLocalName().EqualsIgnoreCase(RegInfoConst::ELEMENT_REGISTRATION))
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

void RegInfoRegistration::DisplayRegInfo()
{
    static const IMS_CHAR *pszState[] =
    {
        "CREATED",
        "INIT",
        "ACTIVE",
        "TERMINATED"
    };

    //---------------------------------------------------------------------------------------------

    if (IMS_UTIL_SYS_PROP_IS_SERVER_INFO_HIDDEN_IN_LOG())
    {
        IMS_TRACE_I("REG :: id=%s, state=%s", strId.GetStr(), pszState[nState], 0);
    }
    else
    {
        IMS_TRACE_I("REG :: id=%s, aor=%s, state=%s",
                strId.GetStr(), SIPDebug::GetUri1(objAOR.ToString()).GetStr(), pszState[nState]);
    }

    AString strTag;

    for (IMS_UINT32 i = 0; i < objContacts.GetSize(); ++i)
    {
        RegInfoContact *pContact = objContacts.GetAt(i);

        strTag.SetNumber(i);

        pContact->DisplayRegInfo(strTag);
    }

    RegInfoContact *pContact = GetPriorContact();

    pContact->DisplayRegInfo("Prior");
}

PUBLIC
IMS_BOOL RegInfoRegistration::Update(IN INode *piNode)
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

        // "aor" attribute
        if (!SetAOR(piNodeMap))
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
            IMS_TRACE_I("REG :: id=%s", strId.GetStr(), 0, 0);
        }
        else
        {
            IMS_TRACE_I("REG :: aor=%s, id=%s",
                    SIPDebug::GetUri1(objAOR.ToString()).GetStr(), strId.GetStr(), 0);
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
    INode *piNode_Contact = piNode->GetFirstChild();

    if (!SetContacts(piNode_Contact))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
RegInfoContact* RegInfoRegistration::CheckNCreateContact(IN INode *piNode)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 j = 0; j < objContacts.GetSize(); ++j)
    {
        RegInfoContact *pContact = objContacts.GetAt(j);

        if (pContact->Equals(piNode))
        {
            return pContact;
        }
    }

    // New contact updated...
    RegInfoContact *pContact = new RegInfoContact();

    if (pContact == IMS_NULL)
    {
        return IMS_NULL;
    }

    if (!objContacts.Append(pContact))
    {
        delete pContact;
        return IMS_NULL;
    }

    return pContact;
}

PRIVATE
IMS_BOOL RegInfoRegistration::SetAOR(IN INamedNodeMap *piNodeMap)
{
    INode *piNode = piNodeMap->GetNamedItem(RegInfoConst::ATTR_AOR);

    //---------------------------------------------------------------------------------------------

    if (piNode == IMS_NULL)
    {
        IMS_TRACE_E(0, "Can't find 'aor' attribute", 0, 0, 0);
        return IMS_FALSE;
    }

    if (!objAOR.Create(piNode->GetNodeValue()))
    {
        IMS_TRACE_E(0, "Creating an AOR (%s) failed",
                SIPDebug::GetUri1(piNode->GetNodeValue()).GetStr(), 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL RegInfoRegistration::SetContacts(IN INode *piNode)
{
    //---------------------------------------------------------------------------------------------

    while (piNode != IMS_NULL)
    {
        RegInfoContact *pContact = CheckNCreateContact(piNode);

        if (pContact != IMS_NULL)
        {
            pContact->Update(piNode);
        }

        piNode = piNode->GetNextSibling();
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL RegInfoRegistration::SetId(IN INamedNodeMap *piNodeMap)
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
IMS_BOOL RegInfoRegistration::SetState(IN INamedNodeMap *piNodeMap)
{
    INode *piNode = piNodeMap->GetNamedItem(RegInfoConst::ATTR_STATE);

    //---------------------------------------------------------------------------------------------

    if (piNode == IMS_NULL)
    {
        IMS_TRACE_E(0, "Can't find 'state' attribute", 0, 0, 0);
        return IMS_FALSE;
    }

    const AString& strState = piNode->GetNodeValue();

    if (strState.EqualsIgnoreCase(RegInfoConst::ATTR_STATE_INIT))
    {
        nState = STATE_INIT;
    }
    else if (strState.EqualsIgnoreCase(RegInfoConst::ATTR_STATE_ACTIVE))
    {
        nState = STATE_ACTIVE;
    }
    else if (strState.EqualsIgnoreCase(RegInfoConst::ATTR_STATE_TERMINATED))
    {
        nState = STATE_TERMINATED;
    }

    return IMS_TRUE;
}
