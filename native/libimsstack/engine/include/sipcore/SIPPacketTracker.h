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



class SIPPacketTracker
    : public ISIPPacketTracker
{
public:
    SIPPacketTracker();
    virtual ~SIPPacketTracker();

private:
    // ISIPPacketTrackerListener class
    virtual void SetListener(IN ISIPPacketTrackerListener* piListener);

public:
    IMS_BOOL IsPacketTrackerEnabled() const;
    void NotifyMessageSent(IN ISIPMessage *piSIPMsg,
            IN CONST ByteArray &objMsg, IN IMS_BOOL bIsRetransmission);
    void NotifyMessageReceived(IN ISIPMessage *piSIPMsg,
            IN CONST ByteArray &objMsg, IN IMS_BOOL bIsRetransmission);

public:
    ISIPPacketTrackerListener *piListener;
};

#endif // _SIP_PACKET_TRACKER_H_
