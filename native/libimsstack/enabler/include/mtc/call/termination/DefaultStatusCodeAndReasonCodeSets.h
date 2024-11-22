/*
 * Copyright (C) 2024 The Android Open Source Project
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

#ifndef DEFAULT_STATUS_CODE_AND_REASON_CODE_SETS_H_
#define DEFAULT_STATUS_CODE_AND_REASON_CODE_SETS_H_

#include "CallReasonInfo.h"
#include "SipStatusCode.h"
#include <unordered_map>

static const std::unordered_map<IMS_SINT32, IMS_SINT32> s_defaultStatusCodeAndReasonCodeMap = {
        {SipStatusCode::SC_300, CODE_SIP_REDIRECTED                   },
        {SipStatusCode::SC_301, CODE_SIP_REDIRECTED                   },
        {SipStatusCode::SC_302, CODE_SIP_REDIRECTED                   },
        {SipStatusCode::SC_305, CODE_SIP_REDIRECTED                   },
        {SipStatusCode::SC_380, CODE_SIP_REDIRECTED                   },

        {SipStatusCode::SC_400, CODE_SIP_BAD_REQUEST                  },
        {SipStatusCode::SC_401, CODE_SIP_CLIENT_ERROR                 },
        {SipStatusCode::SC_403, CODE_SIP_FORBIDDEN                    },
        {SipStatusCode::SC_404, CODE_SIP_NOT_FOUND                    },
        {SipStatusCode::SC_405, CODE_SIP_METHOD_NOT_ALLOWED           },
        {SipStatusCode::SC_406, CODE_SIP_NOT_ACCEPTABLE               },
        {SipStatusCode::SC_407, CODE_SIP_PROXY_AUTHENTICATION_REQUIRED},
        {SipStatusCode::SC_408, CODE_SIP_REQUEST_TIMEOUT              },
        {SipStatusCode::SC_410, CODE_SIP_NOT_REACHABLE                },
        {SipStatusCode::SC_413, CODE_SIP_REQUEST_ENTITY_TOO_LARGE     },
        {SipStatusCode::SC_414, CODE_SIP_REQUEST_URI_TOO_LARGE        },
        {SipStatusCode::SC_415, CODE_SIP_NOT_SUPPORTED                },
        {SipStatusCode::SC_416, CODE_SIP_NOT_SUPPORTED                },
        {SipStatusCode::SC_420, CODE_SIP_NOT_SUPPORTED                },
        {SipStatusCode::SC_421, CODE_SIP_EXTENSION_REQUIRED           },
        {SipStatusCode::SC_422, CODE_SIP_INTERVAL_TOO_BRIEF           },
        {SipStatusCode::SC_480, CODE_SIP_TEMPRARILY_UNAVAILABLE       },
        {SipStatusCode::SC_481, CODE_SIP_TRANSACTION_DOES_NOT_EXIST   },
        {SipStatusCode::SC_482, CODE_SIP_LOOP_DETECTED                },
        {SipStatusCode::SC_483, CODE_SIP_TOO_MANY_HOPS                },
        {SipStatusCode::SC_484, CODE_SIP_BAD_ADDRESS                  },
        {SipStatusCode::SC_485, CODE_SIP_AMBIGUOUS                    },
        {SipStatusCode::SC_486, CODE_SIP_BUSY                         },
        {SipStatusCode::SC_487, CODE_SIP_REQUEST_CANCELLED            },
        {SipStatusCode::SC_488, CODE_SIP_NOT_ACCEPTABLE               },
        {SipStatusCode::SC_491, CODE_SIP_REQUEST_PENDING              },
        {SipStatusCode::SC_493, CODE_SIP_UNDECIPHERABLE               },
        {SipStatusCode::SC_499, CODE_SIP_NOT_REACHABLE                },

        {SipStatusCode::SC_500, CODE_SIP_SERVER_ERROR                 },
        {SipStatusCode::SC_501, CODE_SIP_SERVER_INTERNAL_ERROR        },
        {SipStatusCode::SC_502, CODE_SIP_SERVER_ERROR                 },
        {SipStatusCode::SC_503, CODE_SIP_SERVICE_UNAVAILABLE          },
        {SipStatusCode::SC_504, CODE_SIP_SERVER_TIMEOUT               },
        {SipStatusCode::SC_505, CODE_SIP_SERVER_ERROR                 },
        {SipStatusCode::SC_513, CODE_SIP_SERVER_ERROR                 },
        {SipStatusCode::SC_580, CODE_SIP_SERVER_ERROR                 },

        {SipStatusCode::SC_600, CODE_SIP_BUSY                         },
        {SipStatusCode::SC_603, CODE_SIP_USER_REJECTED                },
        {SipStatusCode::SC_604, CODE_SIP_NOT_REACHABLE                },
        {SipStatusCode::SC_606, CODE_SIP_NOT_ACCEPTABLE               },
        {SipStatusCode::SC_699, CODE_SIP_GLOBAL_ERROR                 }
};
#endif
