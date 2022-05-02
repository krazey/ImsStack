#ifndef _SIP_FACTORY_H_
#define _SIP_FACTORY_H_

#include "AString.h"

class ISIPIPSecState;
class ISIPKeepAliveHelper;
class ISIPMessageTracker;
class ISIPPacketTracker;
class ISIPRoutingRejectNotifier;
class ISIPRTConfigHelper;
class ISIPTokenGenerator;
class ISIPTransportHelper;

/**
 * @brief This class provides an interface to create SIP helper interface to control SIP engine.
 */
class SIPFactory
{
private:
    SIPFactory();

public:
    /**
     * @brief Creates the keep-alive helper.
     *
     * @param nSlotId Slot id to provision a proper configuration
     * @return Pointer to ISIPKeepAliveHelper.
     */
    static ISIPKeepAliveHelper* CreateKeepAliveHelper(IN IMS_SINT32 nSlotId = IMS_SLOT_0);

    /**
     * @brief Generates a string for Call-ID header field.
     *
     * @param strHost Device's host name (IP address)
     * @param strCallId Call-ID header field value
     */
    static void GenerateCallId(IN const AString &strHost, OUT AString &strCallId);

    /**
     * @brief Generates a string for Session-ID header field.
     *
     * NOTE: draft-kaplan-insipid-session-id-04, for Session-ID header field.
     *
     * @param nSlotId Slot id to obtain a fixed key
     * @param strCallId Call-ID for this Session-ID
     * @param strSessionId Session-ID header field value
     */
    static void GenerateSessionId(IN IMS_SINT32 nSlotId,
            IN const AString &strCallId, OUT AString &strSessionId);

    /**
     * @brief Returns the instance of SIP IPSec state.
     *
     * @param nSlotId Slot id to provision a proper configuration
     * @return Pointer to ISIPIPSecState.
     */
    static ISIPIPSecState* GetIPSecState(IN IMS_SINT32 nSlotId = IMS_SLOT_0);

    /**
     * @brief Returns the instance of SIP message tracker.
     *
     * @param nSlotId Slot id to provision a proper configuration
     * @return Pointer to ISIPMessageTracker.
     */
    static ISIPMessageTracker* GetMessageTracker(IN IMS_SINT32 nSlotId = IMS_SLOT_0);

    /**
     * @brief Returns the instance of SIP packet tracker.
     *
     * @param nSlotId Slot id to provision a proper configuration
     * @return Pointer to ISIPPacketTracker.
     */
    static ISIPPacketTracker* GetPacketTracker(IN IMS_SINT32 nSlotId = IMS_SLOT_0);

    /**
     * @brief Returns the instance of SIP routing reject notifier.
     *
     * @param nSlotId Slot id to provision a proper configuration
     * @return Pointer to ISIPRoutingRejectNotifier.
     */
    static ISIPRoutingRejectNotifier* GetRoutingRejectNotifier(IN IMS_SINT32 nSlotId = IMS_SLOT_0);

    /**
     * @brief Returns the instance of SIP run-time (or real-time) configuration helper.
     *
     * @param nSlotId Slot id to provision a proper configuration
     * @return Pointer to ISIPRTConfigHelper.
     */
    static ISIPRTConfigHelper* GetRTConfigHelper(IN IMS_SINT32 nSlotId = IMS_SLOT_0);

    /**
     * @brief Returns the instance of SIP transport helper.
     *
     * @param nSlotId Slot id to provision a proper configuration
     * @return Pointer to ISIPTransportHelper.
     */
    static ISIPTransportHelper* GetTransportHelper(IN IMS_SINT32 nSlotId = IMS_SLOT_0);

    /**
     * @brief Sets the specific token generator.
     *
     * @param nSlotId Slot id to provision a proper configuration
     * @param piTokenGenerator Pointer to ISIPTokenGenerator
     * @deprecated NOT_USED.
     */
    static void SetTokenGenerator(IN IMS_SINT32 nSlotId, IN ISIPTokenGenerator *piTokenGenerator);
};

#endif // _SIP_FACTORY_H_
