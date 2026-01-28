/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef SIP_STATUS_CODE_H_
#define SIP_STATUS_CODE_H_

#include "AString.h"

/**
 * @brief This class contains constants representing SIP response codes as defined in RFC 3261
 *        and extensions.
 */
class SipStatusCode
{
public:
    // cppcheck-suppress noExplicitConstructor
    SipStatusCode(IN IMS_SINT32 nCode = SC_INVALID);
    SipStatusCode(IN IMS_SINT32 nCode, IN const IMS_CHAR* pszReasonPhrase);
    SipStatusCode(IN const SipStatusCode& other);
    ~SipStatusCode();

public:
    SipStatusCode& operator=(IN const SipStatusCode& other);
    SipStatusCode& operator=(IN IMS_SINT32 nCode);
    SipStatusCode& operator=(IN const IMS_CHAR* pszReasonPhrase);
    SipStatusCode& operator=(IN const AString& strReasonPhrase);

public:
    /**
     * @brief Compares the SIP status code.
     *
     * @param nCode SIP status code
     * @return If this status code is greater than nCode_, returns the positive integer value.\n
     *         If this status code is less than nCode_, returns the negative integer value.\n
     *         If both is the same, returns 0.
     */
    IMS_SINT32 Compare(IN IMS_SINT32 nCode) const;

    /**
     * @brief Gets a textual representation of the given SIP status code.
     *
     * @return SIP reason phrase.
     */
    inline const AString& GetReasonPhrase() const { return m_strReasonPhrase; }

    /**
     * @brief Gets a SIP status code as integer value.
     *
     * @return SIP status code.
     */
    inline IMS_SINT32 ToInt() const { return m_nCode; }

    /**
     * @brief Converts the SIP status code to a SIP reason phrase.
     *
     * @param nCode SIP status code
     * @return SIP reason phrase.
     */
    static const IMS_CHAR* GetReasonPhrase(IN IMS_SINT32 nCode);

    /**
     * @brief Checks if this SIP status code is 1XX response or not.
     *
     * 1XX response: 100 ~ 199
     *
     * @return If this status code is 1XX, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline static IMS_BOOL Is1XX(IN IMS_SINT32 nCode)
    {
        return ((nCode >= SC_100) && (nCode < SC_200));
    }

    /**
     * @brief Checks if this SIP status code is a provisional response (excluding 100) or not.
     *
     * Provisional response: 101 ~ 199
     *
     * @return If this status code is a provisional response, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    inline static IMS_BOOL IsProvisional(IN IMS_SINT32 nCode)
    {
        return ((nCode > SC_100) && (nCode < SC_200));
    }

    /**
     * @brief Checks if this SIP status code is a final response or not.
     *
     * Final response: 200 ~
     *
     * @return If this status code is a final response, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    inline static IMS_BOOL IsFinal(IN IMS_SINT32 nCode) { return (nCode >= SC_200); }

    /**
     * @brief Checks if this SIP status code is a final successful response or not.
     *
     * Successful final response: 200 ~ 299
     *
     * @return If this status code is a final successful response, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    inline static IMS_BOOL IsFinalSuccess(IN IMS_SINT32 nCode)
    {
        return ((nCode >= SC_200) && (nCode < SC_300));
    }

    /**
     * @brief Checks if this SIP status code is a final failure response or not.
     *
     * Failure final response: 300 ~
     *
     * @return If this status code is a final failure response, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    inline static IMS_BOOL IsFinalFailure(IN IMS_SINT32 nCode)
    {
        return ((nCode >= SC_300) && (nCode < SC_MAX));
    }

public:
    /// SIP status codes (1xx ~ 6xx)
    enum
    {
        SC_INVALID = 0,

        /*
         * 1xx : provisional (informational) response codes
         *    Request received, continuing to process the request
         */
        SC_100 = 100,  ///< 100 Trying
        SC_180 = 180,  ///< 180 Ringing
        SC_181 = 181,  ///< 181 Call Is Being Forwarded
        SC_182 = 182,  ///< 182 Queued
        SC_183 = 183,  ///< 183 Session Progress
        SC_199 = 199,  ///< 199 Early Dialog Terminated [RFC 6228]

        /*
         * 2xx : successful final response codes
         *    The action was successfully received, understood, and accepted
         */
        SC_200 = 200,  ///< 200 OK
        SC_202 = 202,  ///< 202 Accepted [RFC 6665]
        SC_204 = 204,  ///< 204 No Notification [RFC 5839]

        /*
         * 3xx : redirection final response codes
         *    Further action needs to be taken in order to complete the request
         */
        SC_300 = 300,  ///< 300 Multiple Choices
        SC_301 = 301,  ///< 301 Moved Permanently
        SC_302 = 302,  ///< 302 Moved Temporarily
        SC_305 = 305,  ///< 305 Use Proxy
        SC_380 = 380,  ///< 380 Alternative Service

        /*
         * 4xx : client failure final response codes
         *    The request contains bad syntax or cannot be fulfilled at this server
         */
        SC_400 = 400,  ///< 400 Bad Request
        SC_401 = 401,  ///< 401 Unauthorized
        SC_402 = 402,  ///< 402 Payment Required
        SC_403 = 403,  ///< 403 Forbidden
        SC_404 = 404,  ///< 404 Not Found
        SC_405 = 405,  ///< 405 Method Not Allowed
        SC_406 = 406,  ///< 406 Not Acceptable
        SC_407 = 407,  ///< 407 Proxy Authentication Required
        SC_408 = 408,  ///< 408 Request Timeout
        SC_410 = 410,  ///< 410 Gone
        SC_412 = 412,  ///< 412 Conditional Request Failed
        SC_413 = 413,  ///< 413 Request Entity Too Large
        SC_414 = 414,  ///< 414 Request-URI Too Long
        SC_415 = 415,  ///< 415 Unsupported Media Type
        SC_416 = 416,  ///< 416 Unsupported URI Scheme
        SC_417 = 417,  ///< 417 Unknown Resource-Priority [RFC 4412]
        SC_420 = 420,  ///< 420 Bad Extension
        SC_421 = 421,  ///< 421 Extension Required
        SC_422 = 422,  ///< 422 Session Interval Too Small [RFC 4028]
        SC_423 = 423,  ///< 423 Interval Too Brief
        SC_424 = 424,  ///< 424 Bad Location Information [RFC 6442]
        SC_428 = 428,  ///< 428 Use Identity Header [RFC 4474]
        SC_429 = 429,  ///< 429 Provide Referrer Identity [RFC 3892]
        SC_430 = 430,  ///< 430 Flow Failed [RFC 5626]
        SC_433 = 433,  ///< 433 Anonymity Disallowed [RFC 5079]
        SC_436 = 436,  ///< 436 Bad Identity-Info [RFC 4474]
        SC_437 = 437,  ///< 437 Unsupported Certificate [RFC 4474]
        SC_438 = 438,  ///< 438 Invalid Identity Header [RFC 4474]
        SC_439 = 439,  ///< 439 First Hop Lacks Outbound Support [RFC 5626]
        SC_440 = 440,  ///< 440 Max-Breadth Exceeded [RFC 5393]
        SC_469 = 469,  ///< 469 Bad Info Package [RFC 6086]
        SC_470 = 470,  ///< 470 Consent Needed [RFC 5360]
        SC_480 = 480,  ///< 480 Temporarily Unavailable
        SC_481 = 481,  ///< 481 Call/Transaction Does Not Exist
        SC_482 = 482,  ///< 482 Loop Detected
        SC_483 = 483,  ///< 483 Too Many Hops
        SC_484 = 484,  ///< 484 Address Incomplete
        SC_485 = 485,  ///< 485 Ambiguous
        SC_486 = 486,  ///< 486 Busy Here
        SC_487 = 487,  ///< 487 Request Terminated
        SC_488 = 488,  ///< 488 Not Acceptable Here
        SC_489 = 489,  ///< 489 Bad Event [RFC 6665]
        SC_491 = 491,  ///< 491 Request Pending
        SC_493 = 493,  ///< 493 Undecipherable
        SC_494 = 494,  ///< 494 Security Agreement Required [RFC 3329]
        SC_499 = 499,  ///< 499 Not Reachable

        /*
         * 5xx : server failure final response codes
         *    The server failed to fulfill an apparently valid request
         */
        SC_500 = 500,  ///< 500 Server Internal Error
        SC_501 = 501,  ///< 501 Not Implemented
        SC_502 = 502,  ///< 502 Bad Gateway
        SC_503 = 503,  ///< 503 Service Unavailable
        SC_504 = 504,  ///< 504 Server Time-out
        SC_505 = 505,  ///< 505 Version Not Supported
        SC_513 = 513,  ///< 513 Message Too Large
        SC_580 = 580,  ///< 580 Precondition Failure [RFC 3312]

        /*
         * 6xx : global failure final response codes
         *    The request cannot be fulfilled at any server
         */
        SC_600 = 600,  ///< 600 Busy Everywhere
        SC_603 = 603,  ///< 603 Decline
        SC_604 = 604,  ///< 604 Does Not Exist Anywhere
        SC_606 = 606,  ///< 606 Not Acceptable

        SC_699 = 699,
        SC_MAX,
        SC_NOTUSED = 0x7fff
    };

private:
    struct CodeTable
    {
        IMS_SINT32 nCode;
        const IMS_CHAR* pszReasonPhrase;
    };

    static const CodeTable CODE_TABLE[];

    // SIP status code in status-line
    IMS_SINT32 m_nCode;

    // SIP reason phrase in status-line
    AString m_strReasonPhrase;
};

inline IMS_BOOL operator==(IN const SipStatusCode& objSc, IN IMS_SINT32 nCode)
{
    return (objSc.Compare(nCode) == 0);
}
inline IMS_BOOL operator==(IN IMS_SINT32 nCode, IN const SipStatusCode& objSc)
{
    return (objSc.Compare(nCode) == 0);
}

inline IMS_BOOL operator!=(IN const SipStatusCode& objSc, IN IMS_SINT32 nCode)
{
    return !(objSc.Compare(nCode) == 0);
}
inline IMS_BOOL operator!=(IN IMS_SINT32 nCode, IN const SipStatusCode& objSc)
{
    return !(objSc.Compare(nCode) == 0);
}

inline IMS_BOOL operator>=(IN const SipStatusCode& objSc, IN IMS_SINT32 nCode)
{
    return (objSc.Compare(nCode) >= 0);
}
inline IMS_BOOL operator>=(IN IMS_SINT32 nCode, IN const SipStatusCode& objSc)
{
    return (objSc.Compare(nCode) <= 0);
}

inline IMS_BOOL operator<=(IN const SipStatusCode& objSc, IN IMS_SINT32 nCode)
{
    return (objSc.Compare(nCode) <= 0);
}
inline IMS_BOOL operator<=(IN IMS_SINT32 nCode, IN const SipStatusCode& objSc)
{
    return (objSc.Compare(nCode) >= 0);
}

inline IMS_BOOL operator>(IN const SipStatusCode& objSc, IN IMS_SINT32 nCode)
{
    return (objSc.Compare(nCode) > 0);
}
inline IMS_BOOL operator>(IN IMS_SINT32 nCode, IN const SipStatusCode& objSc)
{
    return (objSc.Compare(nCode) < 0);
}

inline IMS_BOOL operator<(IN const SipStatusCode& objSc, IN IMS_SINT32 nCode)
{
    return (objSc.Compare(nCode) < 0);
}
inline IMS_BOOL operator<(IN IMS_SINT32 nCode, IN const SipStatusCode& objSc)
{
    return (objSc.Compare(nCode) > 0);
}

#endif
