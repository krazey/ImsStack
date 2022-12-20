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
#ifndef IMS_IPSEC_TYPE_H_
#define IMS_IPSEC_TYPE_H_

class IpSecType
{
public:
    /// Transport protocol of upper layer
    enum
    {
        TRANS_PROTOCOL_UDP = 0,
        TRANS_PROTOCOL_TCP,
        TRANS_PROTOCOL_ANY
    };

    /// Action
    enum
    {
        ACTION_APPLY = 0,
        ACTION_PERMIT,
        ACTION_DROP,
        ACTION_BYPASS
    };

    /// Direction
    enum
    {
        DIRECTION_INBOUND = 0,
        DIRECTION_OUTBOUND,
        DIRECTION_ANY
    };

    /// Mode
    enum
    {
        MODE_TRANSPORT = 0,
        MODE_TUNNEL
    };

    /// Security Protocol
    enum
    {
        SECURITY_PROTOCOL_AH = 0,
        SECURITY_PROTOCOL_ESP
    };

    /// Integrity Algorithm
    enum
    {
        INTEGRITY_ALGORITHM_HMAC_MD5_96 = 0,
        INTEGRITY_ALGORITHM_HMAC_SHA_1_96
    };

    /// Encryption Algorithm
    enum
    {
        ENCRYPTION_ALGORITHM_DES_EDE3_CBC = 0,
        ENCRYPTION_ALGORITHM_AES_CBC,
        ENCRYPTION_ALGORITHM_NO
    };
};

#endif
