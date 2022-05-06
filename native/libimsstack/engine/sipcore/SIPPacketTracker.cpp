/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20131024  seungjin82.choi@          Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ISipPacketTrackerListener.h"
#include "SIPPacketTracker.h"

PUBLIC
SIPPacketTracker::SIPPacketTracker() :
        piListener(IMS_NULL)
{
}

PUBLIC VIRTUAL SIPPacketTracker::~SIPPacketTracker() {}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPPacketTracker::SetListener(IN ISipPacketTrackerListener* piListener)
{
    //---------------------------------------------------------------------------------------------

    this->piListener = piListener;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPPacketTracker::IsPacketTrackerEnabled() const
{
    //---------------------------------------------------------------------------------------------

    return (piListener != IMS_NULL);
}

/*

Remarks

*/
PUBLIC
void SIPPacketTracker::NotifyMessageSent(
        IN ISipMessage* piSIPMsg, IN CONST ByteArray& objMsg, IN IMS_BOOL bIsRetransmission)
{
    //---------------------------------------------------------------------------------------------

    if (piListener == IMS_NULL)
        return;

    piListener->PacketTracker_NotifyMessageSent(piSIPMsg, objMsg, bIsRetransmission);
}

/*

Remarks

*/
PUBLIC
void SIPPacketTracker::NotifyMessageReceived(
        IN ISipMessage* piSIPMsg, IN CONST ByteArray& objMsg, IN IMS_BOOL bIsRetransmission)
{
    //---------------------------------------------------------------------------------------------

    if (piListener == IMS_NULL)
        return;

    piListener->PacketTracker_NotifyMessageReceived(piSIPMsg, objMsg, bIsRetransmission);
}
