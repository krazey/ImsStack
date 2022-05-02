#ifndef _INTERFACE_SIP_SERVER_CONNECTION_H_
#define _INTERFACE_SIP_SERVER_CONNECTION_H_

#include "ISipConnection.h"

/**
 * @brief This class provides an interface to handle SIP server transaction.
 *
 * ISIPServerConnection is created by the ISIPConnectionNotifier when a new request is received.
 *
 * @see ISIPConnection
 */
class ISIPServerConnection
    : public ISIPConnection
{
public:
    /**
     * @brief Initializes ISIPServerConnection with a specific SIP response
     *        to the received request.
     *
     * The default headers and reason phrase will be initialized automatically.\n
     * After this, the ISIPServerConnection is in INITIALIZED state and the response can be sent.
     *
     * The following headers will be set by the method:
     *     - From
     *     - Call-ID
     *     - CSeq
     *     - To
     *     - Contact
     *
     * If the system has automatically sent the "100 Trying" response, the 100 response initialized
     * and sent by the user is just ignored.\n
     * If the system has automatically sent a response to a MESSAGE request, then this method will
     * throw ALREADY_RESPONDED.
     *
     * @param nStatusCode SIP response status code between 1xx and 6xx
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT InitResponse(IN IMS_SINT32 nStatusCode) = 0;

    /**
     * @brief Changes the default reason phrase.
     *
     * Empty string or null means that an empty (zero-length) reason phrase will be set.
     *
     * @param strReasonPhrase SIP reason phrase to be set
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SetReasonPhrase(IN CONST AString &strReasonPhrase) = 0;

    /**
     * @brief Checks if the specified SIP server connection is the same transaction.
     *
     * This function can be called by CANCEL transaction only.\n
     * If the caller of this function is not a CANCEL SIP connection (transaction),
     * the function returns IMS_FALSE without any comparison.
     *
     * @param piOngoingSSC Pointer to SIP server connection to be compared\n
     *                     In this moment, INVITE server connection only supports.
     * @return If the specified server connection equals to this, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL IsSameTransaction(IN CONST ISIPServerConnection *piOngoingSSC) const = 0;
};

#endif // _INTERFACE_SIP_SERVER_CONNECTION_H_
