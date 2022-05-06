/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090605  lovil@                    Created
    </table>

    Description
    A MessageBodyPart can contain different kinds of content, for example text,
    an image or an audio clip.
*/

#ifndef _MESSAGE_BODY_PART_H_
#define _MESSAGE_BODY_PART_H_

#include "ISipMessageBodyPart.h"
#include "IMessageBodyPart.h"

class IMessage;

class MessageBodyPart : public IMessageBodyPart
{
public:
    MessageBodyPart(IN IMessage* piMessage_, IN ISipMessageBodyPart* piBodyPart_);
    virtual ~MessageBodyPart();

public:
    ISipMessageBodyPart* GetBodyPart() const;
    void SetBodyPart(IN ISipMessageBodyPart* piNewBodyPart);

private:
    // IMessageBodyPart interface implementation
    virtual const ByteArray& GetContent() const;
    virtual AString GetHeader(IN CONST AString& strName) const;
    virtual IMS_RESULT SetContent(IN CONST ByteArray& objContent);
    virtual IMS_RESULT SetHeader(IN CONST AString& strName, IN CONST AString& strValue);
    static IMS_SINT32 GetHeaderType(IN CONST AString& strName);

private:
    IMessage* piMessage;
    ISipMessageBodyPart* piBodyPart;
};

#endif  // _MESSAGE_BODY_PART_H_
