/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100719  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _REG_INFO_H_
#define _REG_INFO_H_

#include "IMSList.h"
#include "RegInfoRegistration.h"
#include "IRegInfo.h"

class IDocument;
class INode;
class IRegInfoListener;

class RegInfo : public IRegInfo
{
public:
    RegInfo();
    virtual ~RegInfo();

public:
    // IRegInfo interface
    virtual IRegInfoRegistration* GetRegistration(IN CONST AString& strAOR) const;
    virtual IRegInfoRegistration* GetRegistration(IN CONST SipAddress& objAOR) const;
    virtual IMSList<IRegInfoRegistration*> GetRegistrations() const;

    void AddListener(IN IRegInfoListener* piListener);
    void RemoveListener(IN IRegInfoListener* piListener);
    IMS_BOOL Update(IN IDocument* piDocument);

    void DisplayRegInfo();

private:
    void CallListener(IN IMS_SINT32 nStatus);
    RegInfoRegistration* CheckNCreateRegistration(IN INode* piNode);
    void RemoveAllRegistrations();

private:
    enum
    {
        STATUS_REFRESH_REQUIRED = 0,
        STATUS_UPDATED,
        STATUS_UPDATE_FAILED
    };

    IMS_BOOL bIsCreated;
    IMS_UINT32 nVersion;
    IMSList<IRegInfoListener*> objListeners;
    IMSList<RegInfoRegistration*> objRegistrations;
};

#endif  // _REG_INFO_H_
