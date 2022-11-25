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
#ifndef SSL_CERTIFICATE_H_
#define SSL_CERTIFICATE_H_

#include "AString.h"

class SslCertificate
{
public:
    // Formatting type of the certificate
    enum
    {
        // Default : file type
        FILETYPE_PEM = 0,
        FILETYPE_ASN1
    };

public:
    SslCertificate();
    explicit SslCertificate(
            IN const AString& strKeyFile, IN IMS_SINT32 nKeyFileType = FILETYPE_PEM);
    SslCertificate(IN const SslCertificate& other);
    inline ~SslCertificate() {}

public:
    SslCertificate& operator=(IN const SslCertificate& other);

public:
    inline const AString& GetCaFile() const { return m_strCaFile; }
    inline const AString& GetCaPath() const { return m_strCaPath; }
    inline const AString& GetCiphers() const { return m_strCiphers; }
    inline const AString& GetKeyFile() const { return m_strKeyFile; }
    inline IMS_SINT32 GetKeyFileType() const { return m_nKeyFileType; }
    inline const AString& GetPassword() const { return m_strPassword; }

    inline void SetCaFile(IN const AString& strCaFile) { m_strCaFile = strCaFile; }
    inline void SetCaPath(IN const AString& strCaPath) { m_strCaPath = strCaPath; }
    inline void SetCiphers(IN const AString& strCiphers) { m_strCiphers = strCiphers; }
    inline void SetKeyFile(IN const AString& strKeyFile) { m_strKeyFile = strKeyFile; }
    inline void SetKeyFileType(IN IMS_SINT32 nKeyFileType) { m_nKeyFileType = nKeyFileType; }
    inline void SetPassword(IN const AString& strPassword) { m_strPassword = strPassword; }

private:
    // Key file info.
    IMS_SINT32 m_nKeyFileType;
    AString m_strKeyFile;

    // Password info.
    AString m_strPassword;

    // CA certificates
    AString m_strCaFile;
    AString m_strCaPath;

    // Cipher strings (separated by colons)
    AString m_strCiphers;
};

#endif
