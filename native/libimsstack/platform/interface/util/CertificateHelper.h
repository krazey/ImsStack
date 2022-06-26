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
#ifndef CERTIFICATE_HELPER_H_
#define CERTIFICATE_HELPER_H_

class CertificateHelper
{
public:
    inline virtual ~CertificateHelper() {}

private:
    CertificateHelper();

public:
    static CertificateHelper* GetInstance();

    inline const AString& GetCertificateName() { return m_strCertificateName; }
    inline const AString& GetFingerPrint() { return m_strFingerPrint; }

private:
    void Init();
    void CreateCertificate();

private:
    static const IMS_CHAR* const CERTIFICATE;
    AString m_strCertificateName;
    AString m_strFingerPrint;
};

#endif
