/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#ifndef _SIP_MESSAGE_BODY_PART_H_
#define _SIP_MESSAGE_BODY_PART_H_

#include "ISIPMessageBodyPart.h"
#include "SIPStackHeaders.h"
#include "SIPUnknownHeaders.h"



class SIPMessageBodyPart
    : public ISIPMessageBodyPart
{
public:
    SIPMessageBodyPart(IN IMS_BOOL bSDPBody_ = IMS_FALSE);
    SIPMessageBodyPart(IN SipMsgBody *pstMsgBody_, IN IMS_BOOL bSDPBody_ = IMS_FALSE);
    virtual ~SIPMessageBodyPart();

private:
    SIPMessageBodyPart(IN CONST SIPMessageBodyPart &objRHS);

public:
    SIPMessageBodyPart& operator=(IN CONST SIPMessageBodyPart &objRHS);

public:
    // ISIPObject interface
    virtual void Destroy();
    // ISIPMessageBodyPart interface
    virtual ISIPMessageBodyPart* Clone() const;
    virtual void CopyFrom(IN CONST ISIPMessageBodyPart *piBodyPart);
    virtual AString GetHeader(IN IMS_SINT32 nType,
            IN CONST AString &strName = AString::ConstNull()) const;
    virtual void SetHeader(IN IMS_SINT32 nType, IN CONST AString &strValue,
            IN CONST AString &strName = AString::ConstNull());
    inline virtual const ByteArray& GetContent() const
    { return objContent; }
    virtual void SetContent(IN CONST ByteArray &objContent);

    IMS_BOOL FormMessageBody();
    void SetHeader(IN SipHeaderBase *pstHeader,
            IN IMS_SINT32 nType = ISIPMessageBodyPart::CONTENT_UNKNOWN);
    inline SipMsgBody* GetMessageBody() const
    { return pstMsgBody; }
    inline IMS_BOOL IsSDPBodyPart() const
    { return bSDPBody; }

private:
    IMS_BOOL ExtractProperties();

private:
    IMS_BOOL bSDPBody;
    SipMsgBody *pstMsgBody;
    SIPUnknownHeaders objOtherMimeHeaders;

    ByteArray objContent;
};

#endif // _SIP_MESSAGE_BODY_PART_H_
