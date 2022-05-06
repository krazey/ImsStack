/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090911  toastops@                 Created
    </table>

    Description

*/

#ifndef _REG_FLOW_H_
#define _REG_FLOW_H_

#include "IPAddress.h"
#include "RegKey.h"

class RegFlow
{
public:
    explicit RegFlow(IN const RegKey& objRegKey_);
    RegFlow(IN const RegFlow& objRHS);
    ~RegFlow();

public:
    RegFlow& operator=(IN const RegFlow& objRHS);

public:
    IMS_BOOL Capture(IN IMS_UINT32 nSubscriber_ = DEFAULT_SUBSCRIBER);
    const AString& GetCallId() const;
    const RegKey& GetRegKey() const;
    // HEADER_REQ_SESSION-ID
    const AString& GetSessionId() const;
    IMS_SINT32 IncreaseNGetCSeqValue(IN IMS_SINT32 nIncrement = 1);
    IMS_BOOL IsReserved(OUT IMS_UINT32* pnSubscriber_ = IMS_NULL) const;
    void Release();
    void Restore();
    void SetCSeqValue(IN IMS_SINT32 nValue);
    void UpdateCallId(IN const IPAddress& objIP);

public:
    enum
    {
        DEFAULT_SUBSCRIBER = 0,
        // 1 ~ : Unique identifier to identify the current subscriber
        NO_SUBSCRIBER = 0x7FFFFFFF
    };

private:
    // Registration key : slot-id & flow-id
    RegKey objRegKey;

    // Call-ID; Key value of this object
    AString strCallId;
    // CSeq number
    IMS_SINT32 nCSeqValue;

    // Subscriber
    IMS_UINT32 nSubscriber;

    // HEADER_REQ_SESSION-ID
    AString strSessionId;
};

#endif  // _REG_FLOW_H_
