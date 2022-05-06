/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100524  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _INTERFACE_SESSION_STATE_H_
#define _INTERFACE_SESSION_STATE_H_

#include "AString.h"

class SdpSessionParameter;

class ISessionState
{
public:
    virtual const AString& GetConnectionAddress() const = 0;
    virtual IMS_SINT32 GetSessionState() const = 0;
    virtual SdpSessionParameter* GetSessionParameter() const = 0;
    virtual const AString& GetPeerConnectionAddress() const = 0;
    virtual SdpSessionParameter* GetPeerSessionParameter() const = 0;
    virtual SdpSessionParameter* GetProposalSessionParameter() = 0;

public:
    // Types of main session state
    enum
    {
        SESSION_STATE_CREATED = 0,
        SESSION_STATE_INITIATED = 1,
        SESSION_STATE_NEGOTIATING = 2,
        SESSION_STATE_ESTABLISHING = 3,
        SESSION_STATE_ESTABLISHED = 4,
        SESSION_STATE_RENEGOTIATING = 5,
        SESSION_STATE_REESTABLISHING = 6,
        SESSION_STATE_TERMINATING = 7,
        SESSION_STATE_TERMINATED = 8
    };
};

#endif  // _INTERFACE_SESSION_STATE_H_
