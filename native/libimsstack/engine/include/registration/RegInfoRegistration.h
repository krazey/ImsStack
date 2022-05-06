/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100720  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _REG_INFO_REGISTRATION_H_
#define _REG_INFO_REGISTRATION_H_

#include "IRegInfoRegistration.h"
#include "RegInfoContact.h"

class INamedNodeMap;
class INode;

class RegInfoRegistration : public IRegInfoRegistration
{
public:
    RegInfoRegistration();
    virtual ~RegInfoRegistration();

public:
    // IRegInfoRegistration interface
    virtual const SipAddress& GetAOR() const;
    virtual IRegInfoContact* GetContact(IN CONST SipAddress& objContactUri) const;
    virtual IMSList<IRegInfoContact*> GetContacts() const;
    virtual RegInfoContact* GetPriorContact() const;
    virtual IMS_SINT32 GetState() const;

    IMS_BOOL Equals(IN INode* piNode) const;
    IMS_BOOL Update(IN INode* piNode);

    void DisplayRegInfo();

private:
    RegInfoContact* CheckNCreateContact(IN INode* piNode);
    IMS_BOOL SetAOR(IN INamedNodeMap* piNodeMap);
    IMS_BOOL SetContacts(IN INode* piNode);
    IMS_BOOL SetId(IN INamedNodeMap* piNodeMap);
    IMS_BOOL SetState(IN INamedNodeMap* piNodeMap);

private:
    AString strId;
    IMS_SINT32 nState;
    SipAddress objAOR;

    IMSList<RegInfoContact*> objContacts;
};

#endif  // _REG_INFO_REGISTRATION_H_
