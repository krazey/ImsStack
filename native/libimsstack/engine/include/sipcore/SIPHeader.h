/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description
     This class provides generic SIP header parser helper. It can be used to parse base string
    header values that are read from SIP message using e.g. SIPConnection::GetHeader() method.
    It uses generic format to parse the header value and parameters following the syntax given
    in RFC 3261.
    - field-name: field-value *(;parameter-name=parameter-value)
    - auth-header-name: auth-scheme LWS auth-param *(COMMA auth-param)
*/

#ifndef _SIP_HEADER_H_
#define _SIP_HEADER_H_

#include "ISIPHeader.h"
#include "SIPStackHeaders.h"

class SIPParameter;



class SIPHeader
    : public ISIPHeader
{
public:
    SIPHeader();
    explicit SIPHeader(IN IMS_SINT32 nType_);
    explicit SIPHeader(IN CONST AString &strName_);
    explicit SIPHeader(IN CONST SipHeaderBase *pstHeader);
    virtual ~SIPHeader();

private:
    SIPHeader(IN CONST SIPHeader &objRHS);
    // To ignore an assignment operator of object
    SIPHeader& operator=(IN CONST SIPHeader& objRHS);

public:
    // ISIPObject interface
    virtual void Destroy();
    // ISIPHeader interface
    virtual ISIPHeader* Clone() const;
    virtual IMS_BOOL Equals(IN CONST ISIPHeader *piHeader) const;
    virtual const SIPAddress* GetSIPAddress() const;
    virtual AString GetHeaderValue() const;
    virtual const AString& GetName() const;
    virtual const SIPParameter* GetParameter(IN CONST AString &strName) const;
    virtual IMS_RESULT GetParameterNames(OUT IMSList<AString> &objPNames) const;
    virtual const IMSList<SIPParameter*>& GetParameters() const;
    virtual IMS_SINT32 GetType() const;
    virtual const AString& GetValue() const;
    virtual IMS_SINT32 GetValueInt() const;
    virtual void RemoveParameter(IN CONST AString &strName);
    virtual void SetName(IN CONST AString &strName_);
    virtual IMS_RESULT SetParameter(IN CONST AString &strName, IN CONST AString &strValue);
    virtual IMS_RESULT SetHeaderValue(IN CONST AString &strHeaderValue);
    virtual IMS_RESULT SetValue(IN CONST AString &strValue);
    virtual IMS_RESULT SetValueInt(IN IMS_SINT32 nValue);
    virtual AString ToString() const;
    virtual AString ToStringWithoutName() const;

    static IMS_BOOL IsValidType(IN IMS_SINT32 nType);
private:
    IMS_BOOL Decode(IN CONST AString &strBody_, IN IMS_BOOL bParseParameter = IMS_TRUE);
    IMS_BOOL ParseUnknownBody(IN CONST AString &strBody_);

public:
    static const IMS_CHAR* NAME[];

private:
    IMS_SINT32 nType;
    AString strName;
    AString strBody;
    // This field is not NULL if the header type can have an URI format body
    SIPAddress *pAddress;
    IMSList<SIPParameter*> objParams;
};

#endif // _SIP_HEADER_H_
