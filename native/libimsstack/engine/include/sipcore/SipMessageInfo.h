/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100318  hwangoo.park@             Created
    </table>

    Description
     This class includes an information of the current processing SIP Message.
*/

#ifndef _SIP_MESSAGE_INFO_H_
#define _SIP_MESSAGE_INFO_H_

#include "SipStackHeaders.h"

class SIPMessageInfo
{
public:
    inline SIPMessageInfo(IN IMS_SINT32 nSlotId_, IN CONST SipMethod& objMethod_,
            IN SipMessage* pstMessage_, IN IMS_SINT32 nDirection_) :
            nSlotId(nSlotId_),
            objMethod(objMethod_),
            nDirection(nDirection_),
            pstMessage(pstMessage_)
    {
    }

    inline ~SIPMessageInfo() {}

public:
    inline IMS_SINT32 GetDirection() const { return nDirection; }

    inline SipMessage* GetMessage() const { return pstMessage; }

    inline const SipMethod& GetMethod() const { return objMethod; }

    inline IMS_SINT32 GetSlotId() const { return nSlotId; }

    inline IMS_BOOL IsOutgoingMessage() const { return (nDirection == DIRECTION_OUTGOING); }

public:
    enum
    {
        DIRECTION_OUTGOING = 0,
        DIRECTION_INCOMING
    };

private:
    IMS_SINT32 nSlotId;

    const SipMethod& objMethod;
    IMS_SINT32 nDirection;

    SipMessage* pstMessage;
};

#endif  // _SIP_MESSAGE_INFO_H_
