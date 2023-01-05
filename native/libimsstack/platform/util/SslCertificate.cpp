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
#include "SslCertificate.h"

PUBLIC
SslCertificate::SslCertificate() :
        m_nKeyFileType(FILETYPE_PEM),
        m_strKeyFile(AString::ConstNull()),
        m_strPassword(AString::ConstNull()),
        m_strCaFile(AString::ConstNull()),
        m_strCaPath(AString::ConstNull()),
        m_strCiphers(AString::ConstNull())
{
}

PUBLIC
SslCertificate::SslCertificate(
        IN const AString& strKeyFile, IN IMS_SINT32 nKeyFileType /*= FILETYPE_PEM*/) :
        m_nKeyFileType(nKeyFileType),
        m_strKeyFile(strKeyFile),
        m_strPassword(AString::ConstNull()),
        m_strCaFile(AString::ConstNull()),
        m_strCaPath(AString::ConstNull()),
        m_strCiphers(AString::ConstNull())
{
}

PUBLIC
SslCertificate::SslCertificate(IN const SslCertificate& other) :
        m_nKeyFileType(other.m_nKeyFileType),
        m_strKeyFile(other.m_strKeyFile),
        m_strPassword(other.m_strPassword),
        m_strCaFile(other.m_strCaFile),
        m_strCaPath(other.m_strCaPath),
        m_strCiphers(other.m_strCiphers)
{
}

PUBLIC
SslCertificate& SslCertificate::operator=(IN const SslCertificate& other)
{
    if (this != &other)
    {
        m_nKeyFileType = other.m_nKeyFileType;
        m_strKeyFile = other.m_strKeyFile;
        m_strPassword = other.m_strPassword;
        m_strCaFile = other.m_strCaFile;
        m_strCaPath = other.m_strCaPath;
        m_strCiphers = other.m_strCiphers;
    }

    return (*this);
}
