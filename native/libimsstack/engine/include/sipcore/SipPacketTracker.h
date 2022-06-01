/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20131024  seungjin82.choi@          Created
    </table>

    Description

*/

#ifndef _SIP_PACKET_TRACKER_H_
#define _SIP_PACKET_TRACKER_H_

#include "ISipMessage.h"
#include "ISipPacketTracker.h"

class SIPPacketTracker : public ISipPacketTracker
{
public:
    SIPPacketTracker();
    virtual ~SIPPacketTracker();

private:
    // ISipPacketTrackerListener class
    virtual void SetListener(IN ISipPacketTrackerListener* piListener);

public:
    IMS_BOOL IsPacketTrackerEnabled() const;
    void NotifyMessageSent(
            IN ISipMessage* piSIPMsg, IN CONST ByteArray& objMsg, IN IMS_BOOL bIsRetransmission);
    void NotifyMessageReceived(
            IN ISipMessage* piSIPMsg, IN CONST ByteArray& objMsg, IN IMS_BOOL bIsRetransmission);

public:
    ISipPacketTrackerListener* piListener;
};

#endif  // _SIP_PACKET_TRACKER_H_
