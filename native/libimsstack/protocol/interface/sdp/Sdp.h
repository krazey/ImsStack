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
#ifndef SDP_H_
#define SDP_H_

#include "ImsTypeDef.h"

class AString;
class AStringArray;

class Sdp
{
public:
    static IMS_BOOL IsDigitString(IN const AString& strValue);
    static IMS_BOOL IsFqdnString(IN const AString& strValue);
    static IMS_BOOL IsTextString(IN const AString& strValue);
    static IMS_BOOL IsTokenString(IN const AString& strValue, IN IMS_BOOL bAllowSpace = IMS_FALSE);
    static IMS_BOOL IsTypedTimeString(IN const AString& strValue);
    static IMS_BOOL IsNonWsString(IN const AString& strValue);
    static IMS_BOOL IsUriString(IN const AString& strValue);
    static IMS_BOOL SplitLine(
            IN const AString& strValue, IN IMS_SINT32 nNumOfParts, OUT AStringArray& objTokens);
    static IMS_UINT32 ConvertTypedTimeToSeconds(IN const AString& strValue);
    static IMS_SINT32 GetPayloadTypeFromAttribute(IN const AString& strValue);
    static AString IncreaseSessionVersion(IN const AString& strValue);
    static IMS_BOOL ParseAttributeRtpmap(IN const AString& strValue, OUT IMS_SINT32& nPayloadType,
            OUT AString& strEncodingName, OUT IMS_UINT32& nClockRate,
            OUT AString& strEncodingParameters);
    static IMS_BOOL ParseAttributeFmtp(
            IN const AString& strValue, OUT IMS_SINT32& nPayloadType, OUT AString& strParameters);
    static IMS_BOOL ParseAttributeRtcp(IN const AString& strValue, OUT IMS_SINT32& nPort);
    static void ParseAttributeSetup(IN const AString& strValue, OUT IMS_SINT32& nTypeOfSetup);
    static void ParseAttributeConnection(
            IN const AString& strValue, OUT IMS_SINT32& nTypeOfConnection);
    static IMS_BOOL ParseAttributeFramesize(IN const AString& strValue,
            OUT IMS_SINT32& nPayloadType, OUT IMS_SINT32& nWidth, OUT IMS_SINT32& nHeight);

private:
    static IMS_BOOL IsByteCharacter(IN const IMS_CHAR c);
    static IMS_BOOL IsVisibleCharacter(IN const IMS_CHAR c);

public:
    // Type definitions for each SDP lines
    // It can be used to check whether the SDP contains the specified type.
    enum
    {
        TYPE_V = 0,
        TYPE_O,
        TYPE_S,
        TYPE_I,
        TYPE_U,
        TYPE_E,
        TYPE_P,
        TYPE_C,
        TYPE_B,
        TYPE_T,
        TYPE_R,
        TYPE_Z,
        TYPE_K,
        TYPE_A,
        TYPE_M,
        TYPE_MAX
    };

    // Character for each SDP lines
    enum
    {
        LINE_V = 0x76,  // 'v'
        LINE_O = 0x6F,  // 'o'
        LINE_S = 0x73,  // 's'
        LINE_I = 0x69,  // 'i'
        LINE_U = 0x75,  // 'u'
        LINE_E = 0x65,  // 'e'
        LINE_P = 0x70,  // 'p'
        LINE_C = 0x63,  // 'c'
        LINE_B = 0x62,  // 'b'
        LINE_T = 0x74,  // 't'
        LINE_R = 0x72,  // 'r'
        LINE_Z = 0x7A,  // 'z'
        LINE_K = 0x6B,  // 'k'
        LINE_A = 0x61,  // 'a'
        LINE_M = 0x6D   // 'm'
    };

    // Type of network
    enum
    {
        NET_TYPE_IN,
        NET_TYPE_OTHER
    };

    // Type of address
    enum
    {
        ADDR_TYPE_IP4,
        ADDR_TYPE_IP6,
        ADDR_TYPE_OTHER
    };

    // Attribute : Type of direction
    enum
    {
        DIRECTION_NONE = (-1),
        DIRECTION_INACTIVE,
        DIRECTION_RECVONLY,
        DIRECTION_SENDONLY,
        DIRECTION_SENDRECV
    };

    // Attribute : Type of setup
    enum
    {
        SETUP_NONE = (-1),
        SETUP_ACTIVE,
        SETUP_PASSIVE,
        SETUP_ACTPASS,
        SETUP_HOLDCONN,
        SETUP_MAX
    };

    // Attribute : Type of connection
    enum
    {
        CONNECTION_NONE = (-1),
        CONNECTION_NEW,
        CONNECTION_EXISTING,
        CONNECTION_MAX
    };

    static const IMS_CHAR STR_ADDR_TYPE_IP4[];
    static const IMS_CHAR STR_ADDR_TYPE_IP6[];
    static const IMS_CHAR STR_NET_TYPE_IN[];

    static const IMS_CHAR* STR_A_SETUP[SETUP_MAX];
    static const IMS_CHAR* STR_A_CONNECTION[CONNECTION_MAX];
};

#endif
