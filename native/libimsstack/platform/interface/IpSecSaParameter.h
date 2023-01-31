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
#ifndef IPSEC_SA_PARAMETER_H_
#define IPSEC_SA_PARAMETER_H_

#include "SocketAddress.h"

/**
 * @brief This class defines the parameters for IpSec security association.
 *
 * @see IIpSecPolicy
 */
class IpSecSaParameter
{
public:
    class Policy
    {
    public:
        Policy();
        Policy(IN IMS_SINT32 nSpi, IN IMS_SINT32 nDirection, IN IMS_SINT32 nMode,
                IN IMS_SINT32 nTransportProtocol, IN const SocketAddress& objLocalAddress,
                IN const SocketAddress& objRemoteAddress);
        Policy(IN IMS_SINT32 nSpi, IN IMS_SINT32 nDirection, IN IMS_SINT32 nMode,
                IN IMS_SINT32 nTransportProtocol, IN const IpAddress& objLocalIpAddress,
                IN IMS_SINT32 nLocalPort, IN const IpAddress& objRemoteIpAddress,
                IN IMS_SINT32 nRemotePort);
        Policy(IN const Policy& other);
        inline ~Policy() {}

    public:
        Policy& operator=(IN const Policy& other);

    public:
        inline IMS_SINT32 GetSpi() const { return m_nSpi; }
        inline IMS_SINT32 GetDirection() const { return m_nDirection; }
        inline IMS_SINT32 GetMode() const { return m_nMode; }
        inline IMS_SINT32 GetTransportProtocol() const { return m_nTransportProtocol; }
        inline const SocketAddress& GetLocalAddress() const { return m_objLocalAddress; }
        inline const SocketAddress& GetRemoteAddress() const { return m_objRemoteAddress; }
        inline IMS_SINT32 GetSocketId() const { return m_nSocketId; }
        inline void SetSocketId(IN IMS_SINT32 nSocketId) { m_nSocketId = nSocketId; }

        IMS_BOOL HasAcceptedSocketId(IN IMS_SINT32 nSocketId);
        void AddAcceptedSocketId(IN IMS_SINT32 nSocketId);
        void RemoveAcceptedSocketId(IN IMS_SINT32 nSocketId);
        AString ToString() const;

        static const IMS_CHAR* DirectionToString(IN IMS_SINT32 nDirection);
        static const IMS_CHAR* ModeToString(IN IMS_SINT32 nMode);
        static const IMS_CHAR* TransportProtocolToString(IN IMS_SINT32 nProtocol);

    public:
        enum
        {
            DIRECTION_IN = 0,
            DIRECTION_OUT = 1
        };

        enum
        {
            MODE_TRANSPORT = 0,
            MODE_TUNNEL = 1
        };

        enum
        {
            TRANSPORT_PROTOCOL_UDP = 0,
            TRANSPORT_PROTOCOL_TCP = 1
        };

        enum
        {
            SOCKET_NOT_SET = 0
        };

    private:
        IMS_SINT32 m_nSpi;
        IMS_SINT32 m_nDirection;
        IMS_SINT32 m_nMode;
        IMS_SINT32 m_nTransportProtocol;
        SocketAddress m_objLocalAddress;
        SocketAddress m_objRemoteAddress;
        IMS_SINT32 m_nSocketId;
        ImsList<IMS_SINT32> m_objAcceptedSocketIds;
    };

public:
    IpSecSaParameter();
    IpSecSaParameter(IN IMS_SINT32 nIpSecId, IN IMS_SINT32 nSecurityProtocol,
            IN IMS_SINT32 nIntegrityAlgorithm, IN IMS_SINT32 nEncryptionAlgorithm,
            IN const ByteArray& objIk, IN const ByteArray& objCk);
    IpSecSaParameter(IN const IpSecSaParameter& other);
    inline ~IpSecSaParameter() {}

public:
    IpSecSaParameter& operator=(IN const IpSecSaParameter& other);

public:
    inline IMS_SINT32 GetIpSecId() const { return m_nIpSecId; }
    inline IMS_SINT32 GetSecurityProtocol() const { return m_nSecurityProtocol; }
    inline IMS_SINT32 GetIntegrityAlgorithm() const { return m_nIntegrityAlgorithm; }
    inline IMS_SINT32 GetEncryptionAlgorithm() const { return m_nEncryptionAlgorithm; }
    inline const ByteArray& GetIk() const { return m_objIk; }
    inline const ByteArray& GetCk() const { return m_objCk; }

    void AddPolicy(IN const Policy& objPolicy);
    void AddPolicys(IN const IMSList<Policy>& objPolicys);
    inline const IMSList<Policy>& GetPolicys() const { return m_objPolicys; }
    void RemoveAllPolicys();
    AString ToString() const;

    static const IMS_CHAR* SecurityProtocolToString(IN IMS_SINT32 nSecurityProtocol);
    static const IMS_CHAR* IntegrityAlgToString(IN IMS_SINT32 nIntegrityAlg);
    static const IMS_CHAR* EncryptionAlgToString(IN IMS_SINT32 nEncryptionAlg);

public:
    enum
    {
        SECURITY_PROTOCOL_AH = 0,
        SECURITY_PROTOCOL_ESP = 1
    };

    enum
    {
        INTEGRITY_ALG_HMAC_MD5_96 = 0,  // Deprecated
        INTEGRITY_ALG_HMAC_SHA_1_96 = 1,
        INTEGRITY_ALG_AES_GMAC = 2,
        INTEGRITY_ALG_NULL = 3
    };

    enum
    {
        ENCRYPTION_ALG_DES_EDE3_CBC = 0,  // Deprecated
        ENCRYPTION_ALG_AES_CBC = 1,
        ENCRYPTION_ALG_NULL = 2,
        ENCRYPTION_ALG_AES_GCM = 3
    };

private:
    IMS_SINT32 m_nIpSecId;
    IMS_SINT32 m_nSecurityProtocol;
    IMS_SINT32 m_nIntegrityAlgorithm;
    IMS_SINT32 m_nEncryptionAlgorithm;
    ByteArray m_objIk;
    ByteArray m_objCk;

    IMSList<Policy> m_objPolicys;
};

#endif
