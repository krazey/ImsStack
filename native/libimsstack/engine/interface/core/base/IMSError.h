#ifndef _IMS_ERROR_H_
#define _IMS_ERROR_H_

#include "IMSTypeDef.h"

/**
 * @brief This class defines IMS error values.
 */
class IMSError
{
private:
    IMSError();

public:
    /**
     * @brief Returns the last error code.
     *
     * @return It returns the last error code.
     */
    static IMS_SINT32 GetLastError();

    /**
     * @brief Returns the string format of the last error code.
     *
     * @return It returns the string format of the last error code.
     */
    static const IMS_CHAR* GetLastErrorString();

    /**
     * @brief Returns the string format of the specified error code.
     *
     * @param nError the error code
     * @return It returns the string format of the specified error code.
     */
    static const IMS_CHAR* GetString(IN IMS_SINT32 nError);

public:
    enum
    {
        NO_ERROR = 10001,
        GENERAL_ERROR = (NO_ERROR + 1),
        SERVICE_CLOSED,
        ILLEGAL_STATE,
        ILLEGAL_ARGUMENT,
        PROFILE_MISSED,
        PROFILE_CORRUPTED,
        AUTHENTICATION_FAILED,
        REGISTRATION_REQUEST_TIMEOUT,
        REGISTRATION_FAILED_RESPONSE,
        // AppId is not an installed IMS app. or Protocol type is not supported
        CONNECTION_NOT_FOUND,
        INVALID_OPERATION,

        REFRESH_FAILED,

        NO_MESSAGE,
        NO_MEMORY,
        NOT_FOUND,
        ALREADY_EXISTS,
        LIST_OPERATION_FAILED,
        PARSING_ERROR,

        NO_SIP_CONNECTION
    };
};

#endif  // _IMS_ERROR_H_
