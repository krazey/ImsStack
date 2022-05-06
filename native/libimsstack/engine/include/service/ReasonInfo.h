/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090702  toastops@                 Created
    </table>

    Description

*/

#ifndef _REASON_INFO_H_
#define _REASON_INFO_H_

#include "SipStatusCode.h"
#include "IReasonInfo.h"

class ReasonInfo : public IReasonInfo
{
public:
    ReasonInfo();
    explicit ReasonInfo(IN IMS_SINT32 nType_);
    ReasonInfo(IN IMS_SINT32 nType_, IN CONST SipStatusCode& objStatusCode_);
    ~ReasonInfo();

private:
    ReasonInfo(IN CONST ReasonInfo& objRHS);
    ReasonInfo& operator=(IN CONST ReasonInfo& objRHS);

public:
    // IReasonInfo interface implementations
    virtual const AString& GetReasonPhrase() const;
    virtual IMS_SINT32 GetReasonType() const;
    virtual IMS_SINT32 GetStatusCode() const;

    void SetReasonType(IN IMS_SINT32 nType);
    void SetStatusCode(IN IMS_SINT32 nStatusCode);

private:
    IMS_SINT32 nType;
    SipStatusCode objStatusCode;
};

#endif  // _REASON_INFO_H_
