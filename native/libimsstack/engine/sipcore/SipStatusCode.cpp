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
#include "ServiceMemory.h"

#include "SipStatusCode.h"

PRIVATE GLOBAL const SipStatusCode::CodeTable SipStatusCode::CODE_TABLE[] = {
        {SipStatusCode::SC_100, "Trying"                          },
        {SipStatusCode::SC_180, "Ringing"                         },
        {SipStatusCode::SC_181, "Call Is Being Forwarded"         },
        {SipStatusCode::SC_182, "Queued"                          },
        {SipStatusCode::SC_183, "Session Progress"                },
        {SipStatusCode::SC_199, "Early Dialog Terminated"         },
        {SipStatusCode::SC_200, "OK"                              },
        {SipStatusCode::SC_202, "Accepted"                        },
        {SipStatusCode::SC_204, "No Notification"                 },
        {SipStatusCode::SC_300, "Multiple Choices"                },
        {SipStatusCode::SC_301, "Moved Permanently"               },
        {SipStatusCode::SC_302, "Moved Temporarily"               },
        {SipStatusCode::SC_305, "Use Proxy"                       },
        {SipStatusCode::SC_380, "Alternative Service"             },
        {SipStatusCode::SC_400, "Bad Request"                     },
        {SipStatusCode::SC_401, "Unauthorized"                    },
        {SipStatusCode::SC_402, "Payment Required"                },
        {SipStatusCode::SC_403, "Forbidden"                       },
        {SipStatusCode::SC_404, "Not Found"                       },
        {SipStatusCode::SC_405, "Method Not Allowed"              },
        {SipStatusCode::SC_406, "Not Acceptable"                  },
        {SipStatusCode::SC_407, "Proxy Authentication Required"   },
        {SipStatusCode::SC_408, "Request Timeout"                 },
        {SipStatusCode::SC_410, "Gone"                            },
        {SipStatusCode::SC_412, "Conditional Request Failed"      },
        {SipStatusCode::SC_413, "Request Entity Too Large"        },
        {SipStatusCode::SC_414, "Request-URI Too Long"            },
        {SipStatusCode::SC_415, "Unsupported Media Type"          },
        {SipStatusCode::SC_416, "Unsupported URI Scheme"          },
        {SipStatusCode::SC_417, "Unknown Resource-Priority"       },
        {SipStatusCode::SC_420, "Bad Extension"                   },
        {SipStatusCode::SC_421, "Extension Required"              },
        {SipStatusCode::SC_422, "Session Interval Too Small"      },
        {SipStatusCode::SC_423, "Interval Too Brief"              },
        {SipStatusCode::SC_424, "Bad Location Information"        },
        {SipStatusCode::SC_428, "Use Identity Header"             },
        {SipStatusCode::SC_429, "Provide Referrer Identity"       },
        {SipStatusCode::SC_430, "Flow Failed"                     },
        {SipStatusCode::SC_433, "Anonymity Disallowed"            },
        {SipStatusCode::SC_436, "Bad Identity-Info"               },
        {SipStatusCode::SC_437, "Unsupported Certificate"         },
        {SipStatusCode::SC_438, "Invalid Identity Header"         },
        {SipStatusCode::SC_439, "First Hop Lacks Outbound Support"},
        {SipStatusCode::SC_440, "Max-Breadth Exceeded"            },
        {SipStatusCode::SC_469, "Bad Info Package"                },
        {SipStatusCode::SC_470, "Consent Needed"                  },
        {SipStatusCode::SC_480, "Temporarily Unavailable"         },
        {SipStatusCode::SC_481, "Call/Transaction Does Not Exist" },
        {SipStatusCode::SC_482, "Loop Detect"                     },
        {SipStatusCode::SC_483, "Too Many Hops"                   },
        {SipStatusCode::SC_484, "Address Incomplete"              },
        {SipStatusCode::SC_485, "Ambiguous"                       },
        {SipStatusCode::SC_486, "Busy Here"                       },
        {SipStatusCode::SC_487, "Request Terminated"              },
        {SipStatusCode::SC_488, "Not Acceptable Here"             },
        {SipStatusCode::SC_489, "Bad Event"                       },
        {SipStatusCode::SC_491, "Request Pending"                 },
        {SipStatusCode::SC_493, "Undecipherable"                  },
        {SipStatusCode::SC_494, "Security Agreement Required"     },
        {SipStatusCode::SC_499, "Not Reachable"                   },
        {SipStatusCode::SC_500, "Server Internal Error"           },
        {SipStatusCode::SC_501, "Not Implemented"                 },
        {SipStatusCode::SC_502, "Bad Gateway"                     },
        {SipStatusCode::SC_503, "Service Unavailable"             },
        {SipStatusCode::SC_504, "Server Time-out"                 },
        {SipStatusCode::SC_505, "Version Not Supported"           },
        {SipStatusCode::SC_513, "Message Too Large"               },
        {SipStatusCode::SC_580, "Precondition Failure"            },
        {SipStatusCode::SC_600, "Busy Everywhere"                 },
        {SipStatusCode::SC_603, "Decline"                         },
        {SipStatusCode::SC_604, "Does Not Exist Everywhere"       },
        {SipStatusCode::SC_606, "Not Acceptable"                  },

        {SipStatusCode::SC_MAX, IMS_NULL                          }
};

PUBLIC
SipStatusCode::SipStatusCode(IN IMS_SINT32 nCode /* = SC_INVALID*/) :
        m_nCode(nCode),
        m_strReasonPhrase(AString::ConstNull())
{
}

PUBLIC
SipStatusCode::SipStatusCode(IN IMS_SINT32 nCode, IN const IMS_CHAR* pszReasonPhrase) :
        m_nCode(nCode),
        m_strReasonPhrase(pszReasonPhrase)
{
}

PUBLIC
SipStatusCode::SipStatusCode(IN const SipStatusCode& other) :
        m_nCode(other.m_nCode),
        m_strReasonPhrase(other.m_strReasonPhrase)
{
}

PUBLIC
SipStatusCode::~SipStatusCode() {}

PUBLIC
SipStatusCode& SipStatusCode::operator=(IN const SipStatusCode& other)
{
    if (this != &other)
    {
        m_nCode = other.m_nCode;
        m_strReasonPhrase = other.m_strReasonPhrase;
    }

    return (*this);
}

/**
 * @brief Gets a textual representation of the given SIP status code.
 */
PUBLIC
SipStatusCode& SipStatusCode::operator=(IN IMS_SINT32 nCode)
{
    m_nCode = nCode;
    return (*this);
}

/**
 * @brief Gets a textual representation of the given SIP status code.
 */
PUBLIC
SipStatusCode& SipStatusCode::operator=(IN const IMS_CHAR* pszReasonPhrase)
{
    m_strReasonPhrase = pszReasonPhrase;
    return (*this);
}

/**
 * @brief Gets a textual representation of the given SIP status code.
 */
PUBLIC
SipStatusCode& SipStatusCode::operator=(IN const AString& strReasonPhrase)
{
    m_strReasonPhrase = strReasonPhrase;
    return (*this);
}

/**
 * @brief Gets a textual representation of the given SIP status code.
 */
PUBLIC
IMS_SINT32 SipStatusCode::Compare(IN IMS_SINT32 nCode) const
{
    return m_nCode - nCode;
}

/**
 * @brief Gets a textual representation of the given SIP status code.
 */
PUBLIC GLOBAL const IMS_CHAR* SipStatusCode::GetReasonPhrase(IN IMS_SINT32 nCode)
{
    IMS_SINT32 nIndex = 0;

    while (CODE_TABLE[nIndex].nCode != SC_MAX)
    {
        if (CODE_TABLE[nIndex].nCode == nCode)
        {
            return CODE_TABLE[nIndex].pszReasonPhrase;
        }

        nIndex++;
    }

    return "";
}
