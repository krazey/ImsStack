#ifndef _SIP_ERROR_H_
#define _SIP_ERROR_H_

#include "IMSTypeDef.h"

/**
 * @brief This class provides an interface for SIP error codes.
 */
class SIPError
{
public:
    static IMS_SINT32 GetLastError();

public:
    enum
    {
        NO_ERROR = 11001,

        // Error codes from J180
        GENERAL_ERROR = (NO_ERROR + 1),
        CONNECTION_NOT_FOUND,                  // Move to common error codes
        TRANSPORT_NOT_SUPPORTED,
        DIALOG_UNAVAILABLE,
        UNKNOWN_CONTENT_TYPE,
        INVALID_STATE,
        INVALID_OPERATION,
        TRANSACTION_UNAVAILABLE,
        INVALID_MESSAGE,
        ALREADY_RESPONDED,

        // Error codes to protocol operation
        CSEQ_VALUE_EXCEEDED,
        CSEQ_VALUE_MISMATCHED,
        DIALOG_NOT_EXIST,
        INVALID_SIP_ADDRESS,
        LOCAL_TAG_MISMATCH,
        LOOP_DETECTED,
        NO_MESSAGE,
        PORT_ALREADY_RESERVED,
        REQUEST_OUT_OF_ORDER,
        SECOND_REQUEST_FAILED,
        TRANSACTION_NOT_EXIST,
        TRANSACTION_TIMER_EXPIRED,
        URI_SCHEME_NOT_SUPPORTED,
        VIA_PROTOCOL_MISMATCH,
        VIA_ADDRESS_MISMATCH,
        AUTHENTICATION_FAILED,

        // General error codes
        ILLEGAL_ARGUMENT,
        LIST_OPERATION_FAILED,
        NO_MEMORY,
        PARSING_ERROR,
        TIMER_ERROR,
        TRANSPORT_ERROR
    };

    /// SIP_TRANSPORT_ERROR_REPORT_ON_TXN\n
    /// Detailed error codes for SIP transport layer (TRANSPORT_ERROR)
    enum
    {
        /// "Too long wouldblock": Send enabled event is not invoked during some time
        TRANSPORT_E_CODE_101 = 101,
        /// "TCP connection timed out": TCP connection request is timed out
        TRANSPORT_E_CODE_102 = 102,
        /// "Message transmission failed": Sending SIP packets is failed
        TRANSPORT_E_CODE_103 = 103,
        /// "Socket is closed by peer": Socket is closed by the remote endpoint
        TRANSPORT_E_CODE_104 = 104,
        /// "Socket connect failed": Connecting a remote endpoint is failed
        TRANSPORT_E_CODE_105 = 105,
        /// "Socket is abnormally closed": Socket is closed by the data connection lost
        TRANSPORT_E_CODE_106 = 106,
        /// "Message retransmission failed": Sending SIP packets is failed (retransmission)
        TRANSPORT_E_CODE_107 = 107
    };
};

#endif // _SIP_ERROR_H_
