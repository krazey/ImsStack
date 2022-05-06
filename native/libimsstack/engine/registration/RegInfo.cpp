/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100719  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "IDocument.h"
#include "IElement.h"
#include "INodeList.h"
#include "IRegInfoListener.h"
#include "RegInfoConst.h"
#include "RegInfo.h"

__IMS_TRACE_TAG_REG__;

PUBLIC
RegInfo::RegInfo() :
        bIsCreated(IMS_FALSE),
        nVersion(0),
        objListeners(IMSList<IRegInfoListener*>()),
        objRegistrations(IMSList<RegInfoRegistration*>())
{
}

PUBLIC VIRTUAL RegInfo::~RegInfo()
{
    //---------------------------------------------------------------------------------------------

    RemoveAllRegistrations();
}

PUBLIC VIRTUAL IRegInfoRegistration* RegInfo::GetRegistration(IN CONST AString& strAOR) const
{
    SipAddress objAOR;

    //---------------------------------------------------------------------------------------------

    if (!objAOR.Create(strAOR))
    {
        return IMS_NULL;
    }

    return GetRegistration(objAOR);
}

PUBLIC VIRTUAL IRegInfoRegistration* RegInfo::GetRegistration(IN CONST SipAddress& objAOR) const
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objRegistrations.GetSize(); ++i)
    {
        RegInfoRegistration* pRegistration = objRegistrations.GetAt(i);

        if (objAOR.Equals(pRegistration->GetAOR()))
        {
            return pRegistration;
        }
    }

    return IMS_NULL;
}

PUBLIC VIRTUAL IMSList<IRegInfoRegistration*> RegInfo::GetRegistrations() const
{
    IMSList<IRegInfoRegistration*> objIRegInfoRegistrations;

    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objRegistrations.GetSize(); ++i)
    {
        RegInfoRegistration* pRegistration = objRegistrations.GetAt(i);

        if (pRegistration != IMS_NULL)
        {
            objIRegInfoRegistrations.Append(pRegistration);
        }
    }

    return objIRegInfoRegistrations;
}

PUBLIC
void RegInfo::AddListener(IN IRegInfoListener* piListener)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
    {
        IRegInfoListener* piTmpListener = objListeners.GetAt(i);

        if (piTmpListener == piListener)
        {
            return;
        }
    }

    objListeners.Append(piListener);
}

PUBLIC
void RegInfo::RemoveListener(IN IRegInfoListener* piListener)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
    {
        IRegInfoListener* piTmpListener = objListeners.GetAt(i);

        if (piTmpListener == piListener)
        {
            objListeners.RemoveAt(i);
            return;
        }
    }
}

PUBLIC
IMS_BOOL RegInfo::Update(IN IDocument* piDocument)
{
    //---------------------------------------------------------------------------------------------

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
    IMS_BOOL bOK = IMS_FALSE;
    IMS_UINT32 nNewVersion = strVersion.ToUInt32(&bOK);

    if (!bOK)
    {
        IMS_TRACE_E(0, "Invalid version attribute", 0, 0, 0);

        CallListener(STATUS_UPDATE_FAILED);
        return IMS_FALSE;
    }

    // If the version is 0, it means that an initial reginfo is received
    if (bIsCreated && (nVersion >= nNewVersion))
    {
        IMS_TRACE_I("RegInfo :: Equal or less (%d) than the local version (%d) - discarded...",
                nNewVersion, nVersion, 0);

        CallListener(STATUS_UPDATE_FAILED);
        return IMS_TRUE;
    }

    IMS_BOOL bSubscriptionRefreshRequired = IMS_FALSE;

    // more than one higher than the local version number
    if (nNewVersion > (nVersion + 1))
    {
        // refreshed SUBSCRIBE needs to be sent
        bSubscriptionRefreshRequired = IMS_TRUE;

        IMS_TRACE_I("RegInfo :: Subscription refresh is required - "
                    "New version (%d), Old version (%d)",
                nNewVersion, nVersion, 0);
    }

    if (!bIsCreated)
    {
        bIsCreated = IMS_TRUE;
    }

    nVersion = nNewVersion;

    // "state" attribute
    const AString& strState = piElement->GetAttribute(RegInfoConst::ATTR_STATE);

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
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("RegInfo :: Version (%d)", nVersion, 0, 0);

    for (IMS_UINT32 i = 0; i < objRegistrations.GetSize(); ++i)
    {
        IMS_TRACE_D("", 0, 0, 0);

        RegInfoRegistration* pRegistration = objRegistrations.GetAt(i);

        pRegistration->DisplayRegInfo();
    }
}

PRIVATE
void RegInfo::CallListener(IN IMS_SINT32 nStatus)
{
    //---------------------------------------------------------------------------------------------

    if (objListeners.IsEmpty())
    {
        return;
    }

    switch (nStatus)
    {
        case STATUS_REFRESH_REQUIRED:
            for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
            {
                IRegInfoListener* piListener = objListeners.GetAt(i);

                if (piListener != IMS_NULL)
                {
                    piListener->RegInfo_Updated(IMS_TRUE);
                }
            }
            break;

        case STATUS_UPDATED:
            for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
            {
                IRegInfoListener* piListener = objListeners.GetAt(i);

                if (piListener != IMS_NULL)
                {
                    piListener->RegInfo_Updated();
                }
            }
            break;

        case STATUS_UPDATE_FAILED:
            for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
            {
                IRegInfoListener* piListener = objListeners.GetAt(i);

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
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 j = 0; j < objRegistrations.GetSize(); ++j)
    {
        RegInfoRegistration* pRegistration = objRegistrations.GetAt(j);

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

    if (!objRegistrations.Append(pRegistration))
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
    //---------------------------------------------------------------------------------------------

    if (objRegistrations.IsEmpty())
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < objRegistrations.GetSize(); ++i)
    {
        RegInfoRegistration* pRegistration = objRegistrations.GetAt(i);

        if (pRegistration != IMS_NULL)
        {
            delete pRegistration;
        }
    }

    objRegistrations.Clear();
}
