/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20140321  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _SIP_TXN_KEY_H_
#define _SIP_TXN_KEY_H_

#include "SipMethod.h"

class SIPTxnKey
{
public:
    SIPTxnKey();
    SIPTxnKey(IN CONST SipMethod& objMethod_, IN IMS_SINT32 nStatusCode_,
            IN CONST AString& strViaBranch_, IN IMS_UINT32 nCSeq_);
    SIPTxnKey(IN CONST SIPTxnKey& objRHS);
    ~SIPTxnKey();

public:
    SIPTxnKey& operator=(IN CONST SIPTxnKey& objRHS);

public:
    IMS_BOOL Equals(IN CONST SIPTxnKey* pKey) const;
    IMS_SINT32 GetExtraInt() const;
    const AString& GetExtraString() const;
    const SipMethod& GetMethod() const;
    IMS_SINT32 GetCSeq() const;
    IMS_SINT32 GetStatusCode() const;
    const AString& GetViaBranch() const;

    void SetExtraInt(IN IMS_SINT32 nExtraInt);
    void SetExtraString(IN CONST AString& strExtraString);

private:
    SipMethod objMethod;
    IMS_SINT32 nStatusCode;
    AString strViaBranch;
    IMS_UINT32 nCSeq;

    IMS_SINT32 nExtraInt;
    AString strExtraString;
};

#endif  // _SIP_TXN_KEY_H_
