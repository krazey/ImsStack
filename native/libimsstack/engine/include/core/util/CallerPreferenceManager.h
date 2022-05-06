/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20120802  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _CALLER_PREFERENCE_MANAGER_H_
#define _CALLER_PREFERENCE_MANAGER_H_

#include "AString.h"
#include "IMSMap.h"

class CallerPreferenceManager
{
private:
    CallerPreferenceManager();
    ~CallerPreferenceManager();

public:
    IMS_BOOL CreatePreferenceWrapper(IN CONST AString& strName, IN CONST AString& strDialogId);
    void DestroyPreferenceWrapper(IN CONST AString& strName);
    const IMSList<AString>& GetAcceptContacts(IN CONST AString& strDialogId) const;
    const IMSList<AString>& GetAcceptContactsByName(IN CONST AString& strName) const;
    void UpdateAcceptContacts(
            IN CONST AString& strName, IN CONST IMSList<AString>& objAcceptContacts);
    void UpdateDialogId(IN CONST AString& strName, IN CONST AString& strDialogId);

    static CallerPreferenceManager* GetInstance();

private:
    class PreferenceWrapper
    {
    public:
        inline PreferenceWrapper() :
                strDialogId(AString::ConstNull()),
                objAcceptContacts(IMSList<AString>())
        {
        }

        inline PreferenceWrapper(IN CONST PreferenceWrapper& objRHS) :
                strDialogId(objRHS.strDialogId),
                objAcceptContacts(objRHS.objAcceptContacts)
        {
        }

        inline ~PreferenceWrapper() {}

    public:
        inline PreferenceWrapper& operator=(IN CONST PreferenceWrapper& objRHS)
        {
            if (this != &objRHS)
            {
                strDialogId = objRHS.strDialogId;
                objAcceptContacts = objRHS.objAcceptContacts;
            }

            return (*this);
        }

    public:
        inline const IMSList<AString>& GetAcceptContacts() const { return objAcceptContacts; }
        inline const AString& GetDialogId() const { return strDialogId; }
        inline void SetAcceptContacts(IN CONST IMSList<AString>& objAcceptContacts)
        {
            this->objAcceptContacts = objAcceptContacts;
        }
        inline void SetDialogId(IN CONST AString& strDialogId) { this->strDialogId = strDialogId; }

    public:
        AString strDialogId;
        IMSList<AString> objAcceptContacts;
    };

private:
    // Empty PreferenceWrapper
    PreferenceWrapper objEmptyPreferenceWrapper;
    // Name (identifier) / AcceptContactWrapper
    IMSMap<AString, PreferenceWrapper> objPreferenceWrappers;
};

#endif  // _CALLER_PREFERENCE_MANAGER_H_
