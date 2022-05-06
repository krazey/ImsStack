/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20120802  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "util/CallerPreferenceManager.h"

PRIVATE
CallerPreferenceManager::CallerPreferenceManager() :
        objEmptyPreferenceWrapper(PreferenceWrapper()),
        objPreferenceWrappers(IMSMap<AString, PreferenceWrapper>())
{
}

PRIVATE
CallerPreferenceManager::~CallerPreferenceManager() {}

/*

Remarks

*/
PUBLIC
IMS_BOOL CallerPreferenceManager::CreatePreferenceWrapper(
        IN CONST AString& strName, IN CONST AString& strDialogId)
{
    IMS_SLONG nIndex = objPreferenceWrappers.GetIndexOfKey(strName);

    //---------------------------------------------------------------------------------------------

    if (nIndex >= 0)
    {
        return IMS_TRUE;
    }

    PreferenceWrapper objPreferenceWrapper;

    objPreferenceWrapper.SetDialogId(strDialogId);

    return objPreferenceWrappers.SetValue(strName, objPreferenceWrapper);
}

/*

Remarks

*/
PUBLIC
void CallerPreferenceManager::DestroyPreferenceWrapper(IN CONST AString& strName)
{
    //---------------------------------------------------------------------------------------------

    objPreferenceWrappers.Remove(strName);
}

/*

Remarks

*/
PUBLIC
const IMSList<AString>& CallerPreferenceManager::GetAcceptContacts(
        IN CONST AString& strDialogId) const
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objPreferenceWrappers.GetSize(); ++i)
    {
        const PreferenceWrapper& objPreferenceWrapper = objPreferenceWrappers.GetValueAt(i);

        if (strDialogId.Equals(objPreferenceWrapper.GetDialogId()))
        {
            return objPreferenceWrapper.GetAcceptContacts();
        }
    }

    return objEmptyPreferenceWrapper.GetAcceptContacts();
}

/*

Remarks

*/
PUBLIC
const IMSList<AString>& CallerPreferenceManager::GetAcceptContactsByName(
        IN CONST AString& strName) const
{
    IMS_SLONG nIndex = objPreferenceWrappers.GetIndexOfKey(strName);

    //---------------------------------------------------------------------------------------------

    if (nIndex < 0)
    {
        return objEmptyPreferenceWrapper.GetAcceptContacts();
    }

    const PreferenceWrapper& objPreferenceWrapper = objPreferenceWrappers.GetValueAt(nIndex);

    return objPreferenceWrapper.GetAcceptContacts();
}

/*

Remarks

*/
PUBLIC
void CallerPreferenceManager::UpdateAcceptContacts(
        IN CONST AString& strName, IN CONST IMSList<AString>& objAcceptContacts)
{
    IMS_SLONG nIndex = objPreferenceWrappers.GetIndexOfKey(strName);

    //---------------------------------------------------------------------------------------------

    if (nIndex < 0)
    {
        return;
    }

    PreferenceWrapper& objPreferenceWrapper = objPreferenceWrappers.GetValueAt(nIndex);

    objPreferenceWrapper.SetAcceptContacts(objAcceptContacts);
}

/*

Remarks

*/
PUBLIC
void CallerPreferenceManager::UpdateDialogId(
        IN CONST AString& strName, IN CONST AString& strDialogId)
{
    IMS_SLONG nIndex = objPreferenceWrappers.GetIndexOfKey(strName);

    //---------------------------------------------------------------------------------------------

    if (nIndex < 0)
    {
        return;
    }

    PreferenceWrapper& objPreferenceWrapper = objPreferenceWrappers.GetValueAt(nIndex);

    objPreferenceWrapper.SetDialogId(strDialogId);
}

/*

Remarks

*/
PUBLIC GLOBAL CallerPreferenceManager* CallerPreferenceManager::GetInstance()
{
    static CallerPreferenceManager* pCallerPreferenceMngr = IMS_NULL;

    //---------------------------------------------------------------------------------------------

    if (pCallerPreferenceMngr == IMS_NULL)
    {
        pCallerPreferenceMngr = new CallerPreferenceManager();
    }

    return pCallerPreferenceMngr;
}
