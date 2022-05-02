#ifndef _SIP_DEBUG_H_
#define _SIP_DEBUG_H_

#include "IPAddress.h"
#include "SipMethod.h"

/**
 * @brief This class provides an interface for SIP debug operations.
 */
class SIPDebug
{
private:
    SIPDebug();

private:
    SIPDebug(IN CONST SIPDebug &objRHS);
    SIPDebug& operator=(IN CONST SIPDebug &objRHS);

public:
    /**
     * @brief Sends SIP message information to the upper layer.
     *
     * @param nSlotId The slot id
     * @param nMsgType SIP message type\n
     *                 #MSG_REQ\n
     *                 #MSG_RSP
     * @param nDirection The message direction\n
     *                   #DIR_OUT\n
     *                   #DIR_IN
     * @param nMethod SIP method type\n
     *                #SIPMethod\n
     *                #M_DEREGISTER\n
     *                #M_REINVITE
     * @param nStatusCode SIP status code for SIP response
     */
    static void Send(IN IMS_SINT32 nSlotId, IN IMS_SINT32 nMsgType,
        IN IMS_SINT32 nDirection, IN IMS_SINT32 nMethod, IN IMS_SINT32 nStatusCode = 0);

    // Methods for logging based on release mode
    /**
     * @brief Initializes the logging buffers.
     */
    static void InitLogging();
    /**
     * @brief Gets the adjusted logging string with the count and the delimiter.
     *
     * @param pszValue The logging string
     * @param nCount The count of the return string\n
     *               It should be less than MAX_LOG_CHAR_ARRAY.
     * @param cDelimiter The delimiter character to be returned
     * @return The adjusted logging string.
     */
    static const IMS_CHAR* GetCharA1(IN CONST IMS_CHAR *pszValue, IN IMS_SINT32 nCount,
            IN CONST IMS_CHAR cDelimiter = 0 /* no delimiter */);
    /**
     * @brief Gets the adjusted logging string with the count and the delimiter.
     *
     * @param pszValue The logging string
     * @param nCount The count of the return string\n
     *               It should be less than MAX_LOG_CHAR_ARRAY.
     * @param cDelimiter The delimiter character to be returned
     * @return The adjusted logging string.
     */
    static const IMS_CHAR* GetCharA2(IN CONST IMS_CHAR *pszValue, IN IMS_SINT32 nCount,
            IN CONST IMS_CHAR cDelimiter = 0 /* no delimiter */);
    /**
     * @brief Gets a string representation of IP address.
     *
     * In release mode, the first 5 characters are returned as it is and others are set to 'x'.
     *
     * @param objIPA IP address
     * @return The adjusted logging string.
     */
    static const IMS_CHAR* GetIP(IN CONST IPAddress &objIPA);
    /**
     * @brief Gets a string representation of IP address.
     *
     * In release mode, the first 5 characters are returned as it is and others are set to 'x'.
     *
     * @param strIP IP address
     * @return The adjusted logging string.
     */
    static const IMS_CHAR* GetIP(IN CONST AString &strIP);
    /**
     * @brief Gets the adjusted logging string with the count and the delimiter.
     *
     * GetStrX(1/2) and GetUriX(1/2) share the log buffer,
     * so, please don't use the composition of these 3 methods at the same time.\n
     * (GetStrX can be used at the same time. It's also same for GetUriX methods.)
     *
     * @param strValue The logging string
     * @param nCount The count of the return string\n
     *               It should be less than MAX_LOG_CHAR_ARRAY.
     * @param cDelimiter The delimiter character to be returned
     * @return The adjusted logging string.
     */
    static const AString& GetStr1(IN CONST AString &strValue, IN IMS_SINT32 nCount,
            IN CONST IMS_CHAR cDelimiter = 0 /* no delimiter */);
    /**
     * @brief Gets the adjusted logging string with the count and the delimiter.
     *
     * GetStrX(1/2) and GetUriX(1/2) share the log buffer,
     * so, please don't use the composition of these 3 methods at the same time.\n
     * (GetStrX can be used at the same time. It's also same for GetUriX methods.)
     *
     * @param strValue The logging string
     * @param nCount The count of the return string\n
     *               It should be less than MAX_LOG_CHAR_ARRAY.
     * @param cDelimiter The delimiter character to be returned
     * @return The adjusted logging string.
     */
    static const AString& GetStr2(IN CONST AString &strValue, IN IMS_SINT32 nCount,
            IN CONST IMS_CHAR cDelimiter = 0 /* no delimiter */);
    /**
     * @brief Gets the adjusted logging string for the given URI string.
     *
     * GetStrX(1/2) and GetUriX(1/2) share the log buffer,
     * so, please don't use the composition of these 3 methods at the same time.\n
     * (GetStrX can be used at the same time. It's also same for GetUriX methods.)
     *
     * @param strValue The URI string
     * @return The adjusted logging string.
     */
    static const AString& GetUri1(IN CONST AString &strValue);
    /**
     * @brief Gets the adjusted logging string for the given URI string.
     *
     * GetStrX(1/2) and GetUriX(1/2) share the log buffer,
     * so, please don't use the composition of these 3 methods at the same time.\n
     * (GetStrX can be used at the same time. It's also same for GetUriX methods.)
     *
     * @param strValue The URI string
     * @return The adjusted logging string.
     */
    static const AString& GetUri2(IN CONST AString &strValue);

private:
    static IMS_BOOL CheckIfDebugRequired(IN IMS_SINT32 nSlotId, IN IMS_SINT32 nMsgType,
            IN IMS_SINT32 nDirection, IN IMS_SINT32 nMethod, IN IMS_SINT32 nStatusCode);
    static IMS_SINT32 GetSimSlot();

public:
    /// Message Type
    enum
    {
        MSG_REQ = 0x00000000,
        MSG_RSP = 0x00000100
    };

    /// Message Direction
    enum
    {
        DIR_OUT = 0x00000000,
        DIR_IN = 0x00000001
    };

    /// Method & StatusCode\n
    /// Refer SIPMethod & SIPStatusCode class for each enumerations
    enum
    {
        M_DEREGISTER = (SIPMethod::MAX + 1),
        M_REINVITE
    };

private:
    enum
    {
        MAX_LOG_IP = 16,
        MAX_LOG_CHAR_ARRAY = 64
    };

    static IMS_CHAR acIP[MAX_LOG_IP + 1];
    static IMS_CHAR acLog1[MAX_LOG_CHAR_ARRAY + 3 + 1];
    static IMS_CHAR acLog2[MAX_LOG_CHAR_ARRAY + 3 + 1];
    static AString strLog1;
    static AString strLog2;

    // For SIM2 in multi-SIM device
    static IMS_CHAR acIP_1[MAX_LOG_IP + 1];
    static IMS_CHAR acLog1_1[MAX_LOG_CHAR_ARRAY + 3 + 1];
    static IMS_CHAR acLog2_1[MAX_LOG_CHAR_ARRAY + 3 + 1];
    static AString strLog1_1;
    static AString strLog2_1;
};

#endif // _SIP_DEBUG_H_
