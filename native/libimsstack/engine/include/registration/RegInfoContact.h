/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100720  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _REG_INFO_CONTACT_H_
#define _REG_INFO_CONTACT_H_

#include "IRegInfoContact.h"

class INamedNodeMap;
class INode;

class RegInfoContact : public IRegInfoContact
{
public:
    RegInfoContact();
    virtual ~RegInfoContact();

public:
    // IRegInfoContact interface
    virtual IMS_UINT32 GetCSeq() const;
    virtual const AString& GetDisplayName() const;
    virtual IMS_SINT32 GetEvent() const;
    virtual IMS_UINT32 GetExpiresValue() const;
    virtual IMS_UINT32 GetFirstCSeq() const;
    virtual const AString& GetPublicGRUU() const;
    virtual const AString& GetTemporaryGRUU() const;
    virtual const AString& GetQValue() const;
    virtual IMS_UINT32 GetRetryAfterValue() const;
    virtual IMS_SINT32 GetState() const;
    virtual const AString& GetUnknownParameter(IN CONST AString& strName) const;
    virtual const IMSMap<AString, AString>& GetUnknownParameters() const;
    virtual const SipAddress& GetURI() const;

    IMS_BOOL Equals(IN INode* piNode) const;
    IMS_BOOL Update(IN INode* piNode);

    void DisplayRegInfo(IN const AString& strTag = AString::ConstNull());

private:
    // Attributes
    void SetCallId(IN INamedNodeMap* piNodeMap);
    void SetCSeq(IN INamedNodeMap* piNodeMap);
    void SetDurationRegistered(IN INamedNodeMap* piNodeMap);
    IMS_BOOL SetEvent(IN INamedNodeMap* piNodeMap);
    void SetExpiresValue(IN INamedNodeMap* piNodeMap);
    IMS_BOOL SetId(IN INamedNodeMap* piNodeMap);
    void SetQValue(IN INamedNodeMap* piNodeMap);
    void SetRetryAfterValue(IN INamedNodeMap* piNodeMap);
    IMS_BOOL SetState(IN INamedNodeMap* piNodeMap);

    // Elements
    void SetDisplayName(IN INode* piNode);
    void SetPublicGRUU(IN INode* piNode);
    void SetTemporaryGRUU(IN INode* piNode);
    void SetUnknownParameter(IN INode* piNode);
    IMS_BOOL SetURI(IN INode* piNode);

private:
    class TempGRUU
    {
    public:
        inline TempGRUU() :
                strGRUU(AString::ConstNull()),
                nFirstCSeq(0)
        {
        }
        inline TempGRUU(IN CONST TempGRUU& objRHS) :
                strGRUU(objRHS.strGRUU),
                nFirstCSeq(objRHS.nFirstCSeq)
        {
        }
        inline ~TempGRUU() {}

    public:
        inline TempGRUU& operator=(IN CONST TempGRUU& objRHS)
        {
            if (this != &objRHS)
            {
                strGRUU = objRHS.strGRUU;
                nFirstCSeq = objRHS.nFirstCSeq;
            }

            return (*this);
        }

    public:
        AString strGRUU;
        IMS_UINT32 nFirstCSeq;
    };

private:
    AString strId;
    IMS_SINT32 nState;
    IMS_SINT32 nEvent;
    // The amount of time that the contact has been bound to the address-of-record, in seconds
    IMS_UINT32 nDurationRegistered;

    // The number of seconds remaining until the binding is due to expire
    // "shortened" event
    IMS_UINT32 nExpires;
    // The amount of seconds after which the owner of the contact is expected
    // to retry its registration
    // "probation" event
    IMS_UINT32 nRetryAfter;
    // URI associated with this contact
    SipAddress objURI;
    // Display name
    AString strDisplayName;

    // The relative priority of this contact compared to other registered contacts
    AString strQValue;
    // The current Call-ID carried in the REGISTER that was last used to update this contact
    AString strCallId;
    // The last CSeq value present in a REGISTER request that updated this contact
    IMS_UINT32 nCSeq;

    // Public GRUU
    AString strPubGRUU;
    // Temporary GRUU
    TempGRUU objTempGRUU;

    // Map for unknown parameters
    IMSMap<AString, AString> objUnknownParameters;
};

#endif  // _REG_INFO_CONTACT_H_
