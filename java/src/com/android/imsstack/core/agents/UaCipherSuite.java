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

package com.android.imsstack.core.agents;

import com.android.imsstack.util.ImsLog;

import java.util.HashMap;

/**
 * Description of Ua security protocol identifiers defined in 3GPP TS 33.220 H.3
 */
public class UaCipherSuite {
    private HashMap<String, Integer> mUaSecurityProtocolIds = new HashMap<>();
    private static UaCipherSuite sUaCipherSuite = new UaCipherSuite();

    private UaCipherSuite() {
        init();
    }

    public static UaCipherSuite getInstance() {
        return sUaCipherSuite;
    }

    protected int getCipherSuiteValue(String securityProtocol) {
        if (mUaSecurityProtocolIds.containsKey(securityProtocol)) {
            return mUaSecurityProtocolIds.get(securityProtocol);
        }

        ImsLog.e(this, "Unsupported cipher suite: " + securityProtocol);
        return mUaSecurityProtocolIds.get("TLS_NULL_WITH_NULL_NULL");
    }

    // Currently, some TLS params are not supported. Please refer supported param list in
    // {@link android.telephony.gba.TlsParam#TlsCipherSuite}
    private void init() {
        // RFC5246 : Cipher suites for TLS v1.2
        mUaSecurityProtocolIds.put("TLS_NULL_WITH_NULL_NULL", 0x0000);
        mUaSecurityProtocolIds.put("TLS_RSA_WITH_NULL_MD5", 0x0001);
        mUaSecurityProtocolIds.put("TLS_RSA_WITH_NULL_SHA", 0x0002);
        mUaSecurityProtocolIds.put("TLS_RSA_WITH_RC4_128_MD5", 0x0004);
        mUaSecurityProtocolIds.put("TLS_RSA_WITH_RC4_128_SHA", 0x0005);
        mUaSecurityProtocolIds.put("TLS_RSA_WITH_3DES_EDE_CBC_SHA", 0x000A);
        mUaSecurityProtocolIds.put("TLS_DH_DSS_WITH_3DES_EDE_CBC_SHA", 0x000D);
        mUaSecurityProtocolIds.put("TLS_DH_RSA_WITH_3DES_EDE_CBC_SHA", 0x0010);
        mUaSecurityProtocolIds.put("TLS_DHE_DSS_WITH_3DES_EDE_CBC_SHA", 0x0013);
        mUaSecurityProtocolIds.put("TLS_DHE_RSA_WITH_3DES_EDE_CBC_SHA", 0x0016);
        mUaSecurityProtocolIds.put("TLS_RSA_WITH_AES_128_CBC_SHA", 0x002F);
        mUaSecurityProtocolIds.put("TLS_DH_DSS_WITH_AES_128_CBC_SHA", 0x0030);
        mUaSecurityProtocolIds.put("TLS_DH_RSA_WITH_AES_128_CBC_SHA", 0x0031);
        mUaSecurityProtocolIds.put("TLS_DHE_DSS_WITH_AES_128_CBC_SHA", 0x0032);
        mUaSecurityProtocolIds.put("TLS_DHE_RSA_WITH_AES_128_CBC_SHA", 0x0033);
        mUaSecurityProtocolIds.put("TLS_DH_anon_WITH_AES_128_CBC_SHA", 0x0034);
        mUaSecurityProtocolIds.put("TLS_RSA_WITH_AES_256_CBC_SHA", 0x0035);
        mUaSecurityProtocolIds.put("TLS_DH_DSS_WITH_AES_256_CBC_SHA", 0x0036);
        mUaSecurityProtocolIds.put("TLS_DH_RSA_WITH_AES_256_CBC_SHA", 0x0037);
        mUaSecurityProtocolIds.put("TLS_DHE_DSS_WITH_AES_256_CBC_SHA", 0x0038);
        mUaSecurityProtocolIds.put("TLS_DHE_RSA_WITH_AES_256_CBC_SHA", 0x0039);
        mUaSecurityProtocolIds.put("TLS_DH_ANON_WITH_AES_256_CBC_SHA", 0x003A);
        mUaSecurityProtocolIds.put("TLS_RSA_WITH_NULL_SHA256", 0x003B);
        mUaSecurityProtocolIds.put("TLS_RSA_WITH_AES_128_CBC_SHA256", 0x003C);
        mUaSecurityProtocolIds.put("TLS_RSA_WITH_AES_256_CBC_SHA256", 0x003D);
        mUaSecurityProtocolIds.put("TLS_DH_DSS_WITH_AES_128_CBC_SHA256", 0x003E);
        mUaSecurityProtocolIds.put("TLS_DH_RSA_WITH_AES_128_CBC_SHA256", 0x003F);
        mUaSecurityProtocolIds.put("TLS_DHE_DSS_WITH_AES_128_CBC_SHA256", 0x0040);
        mUaSecurityProtocolIds.put("TLS_DHE_RSA_WITH_AES_128_CBC_SHA256", 0x0067);
        mUaSecurityProtocolIds.put("TLS_DH_DSS_WITH_AES_256_CBC_SHA256", 0x0068);
        mUaSecurityProtocolIds.put("TLS_DH_RSA_WITH_AES_256_CBC_SHA256", 0x0069);
        mUaSecurityProtocolIds.put("TLS_DHE_DSS_WITH_AES_256_CBC_SHA256", 0x006A);
        mUaSecurityProtocolIds.put("TLS_DHE_RSA_WITH_AES_256_CBC_SHA256", 0x006B);
        mUaSecurityProtocolIds.put("TLS_DH_ANON_WITH_RC4_128_MD5", 0x0018);
        mUaSecurityProtocolIds.put("TLS_DH_ANON_WITH_3DES_EDE_CBC_SHA", 0x001B);
        mUaSecurityProtocolIds.put("TLS_DH_ANON_WITH_AES_128_CBC_SHA256", 0x006C);
        mUaSecurityProtocolIds.put("TLS_DH_ANON_WITH_AES_256_CBC_SHA256", 0x006D);

        // RFC4492 : Elliptic Curve Cryptography (ECC) Cipher Suites for Transport Layer Security
        mUaSecurityProtocolIds.put("TLS_ECDH_ECDSA_WITH_NULL_SHA", 0xC001);
        mUaSecurityProtocolIds.put("TLS_ECDH_ECDSA_WITH_RC4_128_SHA", 0xC002);
        mUaSecurityProtocolIds.put("TLS_ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA", 0xC003);
        mUaSecurityProtocolIds.put("TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA", 0xC004);
        mUaSecurityProtocolIds.put("TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA", 0xC005);
        mUaSecurityProtocolIds.put("TLS_ECDHE_ECDSA_WITH_NULL_SHA", 0xC006);
        mUaSecurityProtocolIds.put("TLS_ECDHE_ECDSA_WITH_RC4_128_SHA", 0xC007);
        mUaSecurityProtocolIds.put("TLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA", 0xC008);
        mUaSecurityProtocolIds.put("TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA", 0xC009);
        mUaSecurityProtocolIds.put("TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA", 0xC00A);
        mUaSecurityProtocolIds.put("TLS_ECDH_RSA_WITH_NULL_SHA", 0xC00B);
        mUaSecurityProtocolIds.put("TLS_ECDH_RSA_WITH_RC4_128_SHA", 0xC00C);
        mUaSecurityProtocolIds.put("TLS_ECDH_RSA_WITH_3DES_EDE_CBC_SHA", 0xC00D);
        mUaSecurityProtocolIds.put("TLS_ECDH_RSA_WITH_AES_128_CBC_SHA", 0xC00E);
        mUaSecurityProtocolIds.put("TLS_ECDH_RSA_WITH_AES_256_CBC_SHA", 0xC00F);
        mUaSecurityProtocolIds.put("TLS_ECDHE_RSA_WITH_NULL_SHA", 0xC010);
        mUaSecurityProtocolIds.put("TLS_ECDHE_RSA_WITH_RC4_128_SHA", 0xC011);
        mUaSecurityProtocolIds.put("TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA", 0xC012);
        mUaSecurityProtocolIds.put("TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA", 0xC013);
        mUaSecurityProtocolIds.put("TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA", 0xC014);
        mUaSecurityProtocolIds.put("TLS_ECDH_anon_WITH_NULL_SHA", 0xC015);
        mUaSecurityProtocolIds.put("TLS_ECDH_anon_WITH_RC4_128_SHA", 0xC016);
        mUaSecurityProtocolIds.put("TLS_ECDH_anon_WITH_3DES_EDE_CBC_SHA", 0xC017);
        mUaSecurityProtocolIds.put("TLS_ECDH_anon_WITH_AES_128_CBC_SHA", 0xC018);
        mUaSecurityProtocolIds.put("TLS_ECDH_anon_WITH_AES_256_CBC_SHA", 0xC019);

        // RFC6101 : The Secure Sockets Layer (SSL) Cipher Suites
        mUaSecurityProtocolIds.put("SSL_RSA_WITH_NULL_MD5", 0x0001);
        mUaSecurityProtocolIds.put("SSL_RSA_WITH_NULL_SHA", 0x0002);
        mUaSecurityProtocolIds.put("SSL_RSA_EXPORT_WITH_RC4_40_MD5", 0x0003);
        mUaSecurityProtocolIds.put("SSL_RSA_WITH_RC4_128_MD5", 0x0004);
        mUaSecurityProtocolIds.put("SSL_RSA_WITH_RC4_128_SHA", 0x0005);
        mUaSecurityProtocolIds.put("SSL_RSA_EXPORT_WITH_DES40_CBC_SHA", 0x0008);
        mUaSecurityProtocolIds.put("SSL_RSA_WITH_DES_CBC_SHA", 0x0009);
        mUaSecurityProtocolIds.put("SSL_RSA_WITH_3DES_EDE_CBC_SHA", 0x000A);
        mUaSecurityProtocolIds.put("SSL_DHE_DSS_EXPORT_WITH_DES40_CBC_SHA", 0x0011);
        mUaSecurityProtocolIds.put("SSL_DHE_DSS_WITH_DES_CBC_SHA", 0x0012);
        mUaSecurityProtocolIds.put("SSL_DHE_DSS_WITH_3DES_EDE_CBC_SHA", 0x0013);
        mUaSecurityProtocolIds.put("SSL_DHE_RSA_EXPORT_WITH_DES40_CBC_SHA", 0x0014);
        mUaSecurityProtocolIds.put("SSL_DHE_RSA_WITH_DES_CBC_SHA", 0x0015);
        mUaSecurityProtocolIds.put("SSL_DH_anon_EXPORT_WITH_RC4_40_MD5", 0x0017);
        mUaSecurityProtocolIds.put("SSL_DH_anon_WITH_RC4_128_MD5", 0x0018);
        mUaSecurityProtocolIds.put("SSL_DH_anon_EXPORT_WITH_DES40_CBC_SHA", 0x0019);
        mUaSecurityProtocolIds.put("SSL_DH_anon_WITH_DES_CBC_SHA", 0x001A);
        mUaSecurityProtocolIds.put("SSL_DH_anon_WITH_3DES_EDE_CBC_SHA", 0x001B);

        // RFC4346 : ephemeral Diffie-Hellman Cipher Suites
        mUaSecurityProtocolIds.put("TLS_RSA_EXPORT_WITH_DES40_CBC_SHA", 0x0008);
        mUaSecurityProtocolIds.put("TLS_DH_DSS_EXPORT_WITH_DES40_CBC_SHA", 0x000B);
        mUaSecurityProtocolIds.put("TLS_DH_RSA_EXPORT_WITH_DES40_CBC_SHA", 0x000E);
        mUaSecurityProtocolIds.put("TLS_DHE_DSS_EXPORT_WITH_DES40_CBC_SHA", 0x0011);
        mUaSecurityProtocolIds.put("TLS_DHE_RSA_EXPORT_WITH_DES40_CBC_SHA", 0x0014);
        mUaSecurityProtocolIds.put("TLS_DH_anon_EXPORT_WITH_DES40_CBC_SHA", 0x0019);

        // RFC5746
        mUaSecurityProtocolIds.put("TLS_EMPTY_RENEGOTIATION_INFO_SCSV", 0xC0FF);

        // RFC7507
        mUaSecurityProtocolIds.put("TLS_FALLBACK_SCSV", 0x5600);

        // RFC4279 : ChaCha20-Poly1305 Cipher Suites for TLS
        mUaSecurityProtocolIds.put("TLS_PSK_WITH_RC4_128_SHA", 0x008A);
        mUaSecurityProtocolIds.put("TLS_PSK_WITH_3DES_EDE_CBC_SHA", 0x008B);
        mUaSecurityProtocolIds.put("TLS_PSK_WITH_AES_128_CBC_SHA", 0x008C);
        mUaSecurityProtocolIds.put("TLS_PSK_WITH_AES_256_CBC_SHA", 0x008D);

        // RFC5288 : AES Galois Counter Mode (GCM) Cipher Suites for TLS
        mUaSecurityProtocolIds.put("TLS_DHE_RSA_WITH_DES_CBC_SHA", 0x0015);
        mUaSecurityProtocolIds.put("TLS_RSA_WITH_AES_128_GCM_SHA256", 0x009C);
        mUaSecurityProtocolIds.put("TLS_RSA_WITH_AES_256_GCM_SHA384", 0x009D);
        mUaSecurityProtocolIds.put("TLS_DHE_RSA_WITH_AES_128_GCM_SHA256", 0x009E);
        mUaSecurityProtocolIds.put("TLS_DHE_RSA_WITH_AES_256_GCM_SHA384", 0x009F);
        mUaSecurityProtocolIds.put("TLS_DHE_DSS_WITH_AES_128_GCM_SHA256", 0x00A2);
        mUaSecurityProtocolIds.put("TLS_DHE_DSS_WITH_AES_256_GCM_SHA384", 0x00A3);
        mUaSecurityProtocolIds.put("TLS_DH_anon_WITH_AES_128_GCM_SHA256", 0x00A6);
        mUaSecurityProtocolIds.put("TLS_DH_anon_WITH_AES_256_GCM_SHA384", 0x00A7);
        mUaSecurityProtocolIds.put("TLS_DHE_PSK_WITH_AES_128_GCM_SHA256", 0x00AA);
        mUaSecurityProtocolIds.put("TLS_DHE_PSK_WITH_AES_256_GCM_SHA384", 0x00AB);

        // RFC5289 : AES Galois Counter Mode (GCM) Cipher Suites for TLS
        mUaSecurityProtocolIds.put("TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256", 0xC023);
        mUaSecurityProtocolIds.put("TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384", 0xC024);
        mUaSecurityProtocolIds.put("TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256", 0xC025);
        mUaSecurityProtocolIds.put("TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA384", 0xC026);
        mUaSecurityProtocolIds.put("TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256", 0xC027);
        mUaSecurityProtocolIds.put("TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384", 0xC028);
        mUaSecurityProtocolIds.put("TLS_ECDH_RSA_WITH_AES_128_CBC_SHA256", 0xC029);
        mUaSecurityProtocolIds.put("TLS_ECDH_RSA_WITH_AES_256_CBC_SHA384", 0xC02A);
        mUaSecurityProtocolIds.put("TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256", 0xC02B);
        mUaSecurityProtocolIds.put("TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384", 0xC02C);
        mUaSecurityProtocolIds.put("TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256", 0xC02D);
        mUaSecurityProtocolIds.put("TLS_ECDH_ECDSA_WITH_AES_256_GCM_SHA384", 0xC02E);
        mUaSecurityProtocolIds.put("TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256", 0xC02F);
        mUaSecurityProtocolIds.put("TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384", 0xC030);
        mUaSecurityProtocolIds.put("TLS_ECDH_RSA_WITH_AES_128_GCM_SHA256", 0xC031);
        mUaSecurityProtocolIds.put("TLS_ECDH_RSA_WITH_AES_256_GCM_SHA384", 0xC032);
        mUaSecurityProtocolIds.put("TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA", 0xC035);
        mUaSecurityProtocolIds.put("TLS_ECDHE_PSK_WITH_AES_256_CBC_SHA", 0xC036);

        // RFC5469 : DES and IDEA Cipher Suites for Transport Layer Security (TLS)
        mUaSecurityProtocolIds.put("TLS_RSA_WITH_DES_CBC_SHA", 0x0009);
        mUaSecurityProtocolIds.put("TLS_DHE_DSS_WITH_DES_CBC_SHA", 0x0012);
        mUaSecurityProtocolIds.put("TLS_DH_anon_WITH_DES_CBC_SHA", 0x00A7);
        mUaSecurityProtocolIds.put("TLS_DH_DSS_WITH_DES_CBC_SHA", 0x000C);

        // RFC6665
        mUaSecurityProtocolIds.put("TLS_DHE_RSA_WITH_AES_128_CCM", 0xC09E);
        mUaSecurityProtocolIds.put("TLS_DHE_RSA_WITH_AES_256_CCM", 0xC09F);
        mUaSecurityProtocolIds.put("TLS_DHE_PSK_WITH_AES_128_CCM", 0xC0A6);
        mUaSecurityProtocolIds.put("TLS_DHE_PSK_WITH_AES_256_CCM", 0xC0A7);

        // RFC7905
        mUaSecurityProtocolIds.put("TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256", 0xCCA8);
        mUaSecurityProtocolIds.put("TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256", 0xCCA9);
        mUaSecurityProtocolIds.put("TLS_DHE_RSA_WITH_CHACHA20_POLY1305_SHA256", 0xCCAA);
        mUaSecurityProtocolIds.put("TLS_ECDHE_PSK_WITH_CHACHA20_POLY1305_SHA256", 0xCCAC);
        mUaSecurityProtocolIds.put("TLS_DHE_PSK_WITH_CHACHA20_POLY1305_SHA256", 0xCCAD);

        // RFC8442 for TLS 1.3
        mUaSecurityProtocolIds.put("TLS_ECDHE_PSK_WITH_AES_128_GCM_SHA256", 0xD001);
        mUaSecurityProtocolIds.put("TLS_ECDHE_PSK_WITH_AES_256_GCM_SHA384", 0xD002);
        mUaSecurityProtocolIds.put("TLS_ECDHE_PSK_WITH_AES_128_CCM_SHA256", 0xD005);

        // RFC8446 for TLS 1.3
        mUaSecurityProtocolIds.put("TLS_AES_128_GCM_SHA256", 0x1301);
        mUaSecurityProtocolIds.put("TLS_AES_256_GCM_SHA384", 0x1302);
        mUaSecurityProtocolIds.put("TLS_CHACHA20_POLY1305_SHA256", 0x1303);
        mUaSecurityProtocolIds.put("TLS_AES_128_CCM_SHA256", 0x1304);
        mUaSecurityProtocolIds.put("TLS_AES_128_CCM_8_SHA256", 0x1305);

        // Signature algorithms shall be supported as per TS 33.210
        mUaSecurityProtocolIds.put("SIG_RSA_PKCS1_SHA1", 0X0201);
        mUaSecurityProtocolIds.put("SIG_ECDSA_SHA1", 0X0203);
        mUaSecurityProtocolIds.put("SIG_RSA_PKCS1_SHA256", 0X0401);
        mUaSecurityProtocolIds.put("SIG_ECDSA_SECP256R1_SHA256", 0X0403);
        mUaSecurityProtocolIds.put("SIG_RSA_PKCS1_SHA256_LEGACY", 0X0420);
        mUaSecurityProtocolIds.put("SIG_RSA_PKCS1_SHA384", 0X0501);
        mUaSecurityProtocolIds.put("SIG_ECDSA_SECP384R1_SHA384", 0X0503);
        mUaSecurityProtocolIds.put("SIG_RSA_PKCS1_SHA384_LEGACY", 0X0520);
        mUaSecurityProtocolIds.put("SIG_RSA_PKCS1_SHA512", 0X0601);
        mUaSecurityProtocolIds.put("SIG_ECDSA_SECP521R1_SHA512", 0X0603);
        mUaSecurityProtocolIds.put("SIG_RSA_PKCS1_SHA512_LEGACY", 0X0620);
        mUaSecurityProtocolIds.put("SIG_RSA_PSS_RSAE_SHA256", 0X0804);
        mUaSecurityProtocolIds.put("SIG_RSA_PSS_RSAE_SHA384", 0X0805);
        mUaSecurityProtocolIds.put("SIG_RSA_PSS_RSAE_SHA512", 0X0806);
        mUaSecurityProtocolIds.put("SIG_ECDSA_BRAINPOOLP256R1TLS13_SHA256", 0X081A);
        mUaSecurityProtocolIds.put("SIG_ECDSA_BRAINPOOLP384R1TLS13_SHA384", 0X081B);
        mUaSecurityProtocolIds.put("SIG_ECDSA_BRAINPOOLP512R1TLS13_SHA512", 0X081C);
    }
}
