#ifndef _INTERFACE_SIP_PACKET_TRACKER_H_
#define _INTERFACE_SIP_PACKET_TRACKER_H_

class ISIPPacketTrackerListener;

/**
 * @brief This class provides an interface to monitor SIP packets.
 *
 * @see ISIPPacketTrackerListener
 */
class ISIPPacketTracker
{
public:
    /**
     * @brief Sets the listener to monitor SIP packets.
     *
     * @param piListener Listener to be set
     */
    virtual void SetListener(IN ISIPPacketTrackerListener* pListener) = 0;
};

#endif // _INTERFACE_SIP_PACKET_TRACKER_H_
