#ifndef _P_ACCESS_NETWORK_INFO_HEADER_H_
#define _P_ACCESS_NETWORK_INFO_HEADER_H_

#include "IPAddress.h"
#include "ImsAccessNetworkInfoType.h"

class INetworkConnection;
class ISIPMessage;
class SIPProfile;

/**
 * @brief This class provides an interface to form P-Access-Network-Info header.
 */
class PAccessNetworkInfoHeader
{
private:
    PAccessNetworkInfoHeader();

public:
    /**
     * @brief Forms P-Access-Network-Info header field.
     *
     * @param nSlotId Slot id for this header
     * @param objANInfo Current access network information which the device is attached
     * @param strHeader Formed header string
     * @return If it succeeds, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    static IMS_BOOL FormHeader(IN IMS_SINT32 nSlotId,
            IN const AccessNetworkInfo &objANInfo, OUT AString &strHeader);
    /**
     * @brief Forms P-Access-Network-Info header field for a specific operator.
     *
     * @param nSlotId Slot id for this header
     * @param piConnection Current network connection which the device is attached
     * @param objMethod SIP method for this header
     * @param pSIPProfile SIP profile for this header
     * @param strHeader Formed header string
     * @return If it succeeds, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    static IMS_BOOL FormHeaderForOperatorSpecific(IN IMS_SINT32 nSlotId,
            IN INetworkConnection* piConnection, IN const SIPMethod& objMethod,
            IN const SIPProfile* pSIPProfile, OUT AString& strHeader);
    /**
     * @brief Forms P-Access-Network-Info header field for a specific operator.
     *
     * @param nSlotId Slot id for this header
     * @param objIP Local IP address
     * @param objMethod SIP method for this header
     * @param pSIPProfile SIP profile for this header
     * @param strHeader Formed header string
     * @return If it succeeds, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    static IMS_BOOL FormHeaderForOperatorSpecific(IN IMS_SINT32 nSlotId,
            IN const IPAddress &objIP, IN const SIPMethod& objMethod,
            IN const SIPProfile* pSIPProfile, OUT AString& strHeader);

    /**
     * @brief Forms P-Access-Network-Info header and sets it to SIP message.
     *
     * @param nSlotId Slot id for this header
     * @param objIP Local IP address
     * @param pSIPProfile SIP profile for this header
     * @param piSIPMsg SIP message object
     */
    static void SetHeader(IN IMS_SINT32 nSlotId, IN const IPAddress &objIP,
            IN const SIPProfile* pSIPProfile, IN_OUT ISIPMessage *&piSIPMsg);

private:
    static void ReformPANIHeaderForATT(
            IN CONST AccessNetworkInfo &objANInfo, IN_OUT AString &strPANIHeader);
    static void ReformPANIHeaderForTEL(IN_OUT AString &strHeader);
    static void ReformPANIHeaderForCountryCode(IN IMS_SINT32 nSlotId,
            IN_OUT AString &strHeader, IN IMS_BOOL bUseUICC);
    static void SetPrivateHeaderForTMUS(IN IMS_SINT32 nSlotId,
            IN INetworkConnection *piConnection, IN_OUT ISIPMessage *&piSIPMsg);
    static void SetPrivateHeaderForMTS(IN IMS_SINT32 nSlotId,
            IN INetworkConnection *piConnection, IN_OUT ISIPMessage *&piSIPMsg);
    static void SetCNIHeader(IN IMS_SINT32 nSlotId, IN INetworkConnection *piConnection,
            IN const SIPProfile* pSIPProfile, IN_OUT ISIPMessage *&piSIPMsg);
    static IMS_BOOL IsAccessNetworkTypeWiFi(IN CONST AccessNetworkInfo &objANInfo);
    static IMS_BOOL IsCountryInfoRequiredForVoWiFi(IN IMS_SINT32 nSlotId,
            IN const SIPProfile* pSIPProfile);
};

#endif // _P_ACCESS_NETWORK_INFO_HEADER_H_
