/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100415  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "TextParser.h"
#include "ISipDialog.h"
#include "Replaces.h"
#include "CallControlHelper.h"

PRIVATE
CallControlHelper::CallControlHelper() :
        nGlobalSessionId(0),
        objSessions(IMSMap<AString, Replaces*>())
{
}

PRIVATE
CallControlHelper::~CallControlHelper()
{
    if (!objSessions.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objSessions.GetSize(); ++i)
        {
            Replaces* pReplaces = objSessions.GetValueAt(i);

            if (pReplaces != IMS_NULL)
                delete pReplaces;
        }

        objSessions.Clear();
    }
}

/*

Remarks

*/
PUBLIC
IMS_BOOL CallControlHelper::AddSession(IN CONST AString& strSessionId, IN Replaces* pReplaces)
{
    //---------------------------------------------------------------------------------------------

    if (pReplaces == IMS_NULL)
        return IMS_FALSE;

    IMS_SLONG nIndex = objSessions.GetIndexOfKey(strSessionId);

    if (nIndex >= 0)
    {
        Replaces* pOldReplaces = objSessions.GetValueAt(nIndex);

        if (pOldReplaces != IMS_NULL)
        {
            delete pOldReplaces;
        }

        objSessions.RemoveAt(nIndex);
    }

    return objSessions.Add(strSessionId, pReplaces);
}

/*

Remarks

*/
PUBLIC
IMS_UINT32 CallControlHelper::GetSessionCount() const
{
    //---------------------------------------------------------------------------------------------

    return objSessions.GetSize();
}

/*

Remarks

*/
PUBLIC
void CallControlHelper::RemoveSession(IN CONST AString& strSessionId)
{
    IMS_SLONG nIndex = objSessions.GetIndexOfKey(strSessionId);

    //---------------------------------------------------------------------------------------------

    if (nIndex < 0)
    {
        return;
    }

    Replaces* pReplaces = objSessions.GetValueAt(nIndex);

    if (pReplaces != IMS_NULL)
    {
        delete pReplaces;
    }

    objSessions.RemoveAt(nIndex);
}

/*

Remarks

*/
PUBLIC
Replaces* CallControlHelper::GetReplacesFromSessionId(IN CONST AString& strSessionId)
{
    //---------------------------------------------------------------------------------------------

    if (objSessions.IsEmpty())
    {
        return IMS_NULL;
    }

    IMS_SLONG nIndex = objSessions.GetIndexOfKey(strSessionId);

    if (nIndex < 0)
    {
        return IMS_NULL;
    }

    return objSessions.GetValueAt(nIndex);
}

/*

Remarks

*/
PUBLIC
const AString& CallControlHelper::GetSessionIdFromReplaces(IN CONST Replaces* pReplaces)
{
    //---------------------------------------------------------------------------------------------

    if (pReplaces == IMS_NULL)
    {
        return AString::ConstNull();
    }

    for (IMS_UINT32 i = 0; i < objSessions.GetSize(); ++i)
    {
        const Replaces* pTmpReplaces = objSessions.GetValueAt(i);

        if (pTmpReplaces == IMS_NULL)
            continue;

        if (pTmpReplaces->IsSameDialog(pReplaces))
        {
            return objSessions.GetKeyAt(i);
        }
    }

    return AString::ConstNull();
}

/*

Remarks

*/
PUBLIC GLOBAL Replaces* CallControlHelper::CreateReplaces(IN IMS_BOOL bMO, IN ISipDialog* piDialog)
{
    //---------------------------------------------------------------------------------------------

    (void)bMO;

    if (piDialog == IMS_NULL)
        return IMS_NULL;

    Replaces* pReplaces = new Replaces(piDialog->GetComponent(ISipDialog::COMPONENT_CALL_ID),
            piDialog->GetComponent(ISipDialog::COMPONENT_LOCAL_TAG),
            piDialog->GetComponent(ISipDialog::COMPONENT_REMOTE_TAG));

    if (pReplaces == IMS_NULL)
    {
        return IMS_NULL;
    }

    return pReplaces;
}

/*

Remarks

*/
PUBLIC GLOBAL const AString CallControlHelper::CreateSessionId()
{
    CallControlHelper* pCCH = CallControlHelper::GetInstance();

    //---------------------------------------------------------------------------------------------

    pCCH->nGlobalSessionId++;

    AString strSessionId;

    strSessionId.Sprintf("sid%08x", pCCH->nGlobalSessionId);

    if (pCCH->nGlobalSessionId == 0xFFFFFFFE)
    {
        pCCH->nGlobalSessionId = 0;
    }

    return strSessionId;
}

/*

Remarks

*/
PUBLIC GLOBAL CallControlHelper* CallControlHelper::GetInstance()
{
    static CallControlHelper* pCCHelper = IMS_NULL;

    //---------------------------------------------------------------------------------------------

    if (pCCHelper == IMS_NULL)
    {
        pCCHelper = new CallControlHelper();
    }

    return pCCHelper;
}
