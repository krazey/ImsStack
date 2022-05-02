#ifndef _SIP_FEATURES_H_
#define _SIP_FEATURES_H_

/**
 * @brief This class provides an interface to check the run-time features for SIP engine.
 */
class SIPFeatures
{
public:
    /**
     * @brief Indicates that Event header can be added for REFER request.
     *
     * @param nSlotId Slot id
     */
    static IMS_BOOL IsEventHeaderApplicableForRefer(IN IMS_SINT32 nSlotId);

    /**
     * @brief Indicates that the control of Session-ID header field is required.
     *
     * @param nSlotId Slot id
     */
    static IMS_BOOL IsHeaderSessionIdRequired(IN IMS_SINT32 nSlotId);

    /**
     * @brief Indicates that the only validation of IP and port is required when Request-URI is
     *        evaluated for the incoming request routing even if the full URI match is failed.
     *
     * @param nSlotId Slot id
     */
    static IMS_BOOL IsHostPartValidationRequiredForIncomingRequestRouting(IN IMS_SINT32 nSlotId);

    /**
     * @brief Indicates that SIP dialog state is determined by the reference
     *        of multiple dialog usages.
     *
     * If false, it will be determined by the count of dialog usages of the same dialog.
     *
     * @param nSlotId Slot id
     */
    static IMS_BOOL IsMultipleDialogUsagesRequiredForNonSharedDialog(IN IMS_SINT32 nSlotId);

    /**
     * @brief Indicates that P-Access-Network-Info header is required in ACK message.
     *
     * 3GPP Rel. 14 describes it's an optional to set PANI header in ACK message.
     *
     * @param nSlotId Slot id
     */
    static IMS_BOOL IsPANIHeaderForAckRequired(IN IMS_SINT32 nSlotId);

    /**
     * @brief Indicates that "TcpMaxSeg" socket option is required
     *       to control TCP maximum packet size.
     *
     * @param nSlotId Slot id
     */
    static IMS_BOOL IsSocketOptionRequiredForTcpMaxSeg(IN IMS_SINT32 nSlotId);

    /**
     * @brief Checks if Refer-Sub / Supported("norefersub") header is supported or not.
     *
     * @param nSlotId Slot id
     */
    static IMS_BOOL IsReferSubHeaderSupported(IN IMS_SINT32 nSlotId);

    /**
     * @brief Indicates that retransmission timeout interval for 2XX-INVITE is based on (T1 * 64).
     *
     * The default logic implements as the following preference order: TimerH > TimerB > (T1 * 64).
     *
     * @param nSlotId Slot id
     */
    static IMS_BOOL IsStandard2XXRetransmissionIntervalRequired(IN IMS_SINT32 nSlotId);

    /**
     * @brief Indicates that "transport" parameter should be removed from the Request-URI
     *        before evaluating the incoming request.
     *
     * @param nSlotId Slot id
     */
    static IMS_BOOL IsTransportParameterIgnoredForIncomingRequestRouting(IN IMS_SINT32 nSlotId);

    /**
     * @brief Indicates that "transport" parameter with "udp" should not be evaluated
     *        when forming the outgoing request from SIP routing address URI.
     *
     * @param nSlotId Slot id
     */
    static IMS_BOOL IsTransportParameterUdpIgnoredForOutgoingRequest(IN IMS_SINT32 nSlotId);

    /**
     * @brief Indicates that "transport" parameter should be removed from the Contact header
     *        before evaluating the registration binding.
     *
     * @param nSlotId Slot id
     */
    static IMS_BOOL IsTransportParameterIgnoredForRegBinding(IN IMS_SINT32 nSlotId);
};

#endif // _SIP_FEATURES_H_
