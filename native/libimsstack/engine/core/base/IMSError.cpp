/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090715  lovil@                    Created
    </table>

    Description

*/

#include "base/IMS.h"
#include "base/IMSError.h"

#define IMS_ERROR_CODE(CODE)  \
    {                         \
        IMSError::CODE, #CODE \
    }
#define IMS_ERROR_CODE_END() \
    {                        \
        0, IMS_NULL          \
    }

struct IMSErrorCode
{
    IMS_SINT32 nCode;
    const IMS_CHAR* pszMessage;
};

static const IMSErrorCode gstErrorCode[] =
{
    IMS_ERROR_CODE(NO_ERROR),
    IMS_ERROR_CODE(GENERAL_ERROR),
    IMS_ERROR_CODE(SERVICE_CLOSED),
    IMS_ERROR_CODE(ILLEGAL_STATE),
    IMS_ERROR_CODE(ILLEGAL_ARGUMENT),
    IMS_ERROR_CODE(PROFILE_MISSED),
    IMS_ERROR_CODE(PROFILE_CORRUPTED),
    IMS_ERROR_CODE(AUTHENTICATION_FAILED),
    IMS_ERROR_CODE(REGISTRATION_REQUEST_TIMEOUT),
    IMS_ERROR_CODE(REGISTRATION_FAILED_RESPONSE),
    // AppId is not an installed IMS app. or Protocol type is not supported
    IMS_ERROR_CODE(CONNECTION_NOT_FOUND),
    IMS_ERROR_CODE(INVALID_OPERATION),
    IMS_ERROR_CODE(REFRESH_FAILED),
    IMS_ERROR_CODE(NO_MESSAGE),
    IMS_ERROR_CODE(NO_MEMORY),
    IMS_ERROR_CODE(NOT_FOUND),
    IMS_ERROR_CODE(ALREADY_EXISTS),
    IMS_ERROR_CODE(LIST_OPERATION_FAILED),
    IMS_ERROR_CODE(PARSING_ERROR),
    IMS_ERROR_CODE(NO_SIP_CONNECTION),

    IMS_ERROR_CODE_END()
};

PUBLIC GLOBAL IMS_SINT32 IMSError::GetLastError()
{
    //---------------------------------------------------------------------------------------------

    return IMS::GetLastError();
}

PUBLIC GLOBAL const IMS_CHAR* IMSError::GetLastErrorString()
{
    //---------------------------------------------------------------------------------------------

    return GetString(IMS::GetLastError());
}

PUBLIC GLOBAL const IMS_CHAR* IMSError::GetString(IN IMS_SINT32 nError)
{
    IMS_UINT32 nCount = sizeof(gstErrorCode) / sizeof(gstErrorCode[0]);

    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < nCount; ++i)
    {
        const IMSErrorCode* pCode = &(gstErrorCode[i]);

        if ((pCode->nCode == 0) || (pCode->pszMessage == IMS_NULL))
            break;

        if (pCode->nCode == nError)
        {
            return pCode->pszMessage;
        }
    }

    return "__UNKNOWN__";
}
