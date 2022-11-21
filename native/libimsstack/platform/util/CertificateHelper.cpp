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
#include "ByteArray.h"
#include "CertificateHelper.h"
#include "ImsConstDef.h"
#include "ServiceFile.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceUtil.h"

__IMS_TRACE_TAG_ADAPT__;

const IMS_CHAR* const CertificateHelper::CERTIFICATE =
        IMS_SOLUTION_STORAGE_ROOT_DIR "/files/client.pem";

PRIVATE
CertificateHelper::CertificateHelper()
{
    Init();
}

PUBLIC GLOBAL CertificateHelper* CertificateHelper::GetInstance()
{
    static CertificateHelper* s_pCertificateHelper = IMS_NULL;

    if (s_pCertificateHelper == IMS_NULL)
    {
        s_pCertificateHelper = new CertificateHelper();
    }

    return s_pCertificateHelper;
}

PRIVATE
void CertificateHelper::Init()
{
    // Create a certificate
    CreateCertificate();

    // Set the certificate path
    m_strCertificateName = CERTIFICATE;

    // Generate the fingerprint
    // Sample fingerprint: "06:11:F7:C0:1F:93:54:0F:50:AA:95:A9:84:55:7B:CD:09:78:B9:EB"
    UtilService::GetUtilService()->GetSystemUtil()->DigestSha1(
            m_strCertificateName, m_strFingerPrint);

    IMS_TRACE_D("Init :: Certificate(%s) -> Fingerprint(%s)", m_strCertificateName.GetStr(),
            m_strFingerPrint.GetStr(), 0);
}

PRIVATE
void CertificateHelper::CreateCertificate()
{
    ByteArray objCertificate;
    objCertificate.Append(
            "-----BEGIN CERTIFICATE-----\n"
            "MIICEzCCAXwCCQD9XU1OoCVDrjANBgkqhkiG9w0BAQUFADBNMQswCQYDVQQGEwJF\n"
            "UzEPMA0GA1UECBMGTWFkcmlkMQ8wDQYDVQQHEwZNYWRyaWQxDDAKBgNVBAoTA1ZE\n"
            "RjEOMAwGA1UEAxMFTEdSQ1MwIBcNMTExMDA4MDE0MDQyWhgPMjExMTA5MTQwMTQw\n"
            "NDJaME0xCzAJBgNVBAYTAkVTMQ8wDQYDVQQIEwZNYWRyaWQxDzANBgNVBAcTBk1h\n"
            "ZHJpZDEMMAoGA1UEChMDVkRGMQ4wDAYDVQQDEwVMR1JDUzCBnzANBgkqhkiG9w0B\n"
            "AQEFAAOBjQAwgYkCgYEA4Qi5Rs8gv6w13G3TwFAzMOcn7pWaazoCRdztxj5KcMJr\n"
            "aGxZqKbZQYF/QYI6NeSTFPWwrD12wt8ZpB6OzWFd7dBNkB3HktiLp8HPZcSENJLh\n"
            "cYdO34yi/tUUBA09KfohWO/f9KBg+Xy46JBCOmhPoEe/aTfKScjiRs3ejCalYOUC\n"
            "AwEAATANBgkqhkiG9w0BAQUFAAOBgQA373ZRHDM8Ajw5dgbdfXaJWOW+/tCSpJKO\n"
            "2E/Q5Atk0afPnaOdDvanTYQxIGTmuihhYymyhoNHeJ5JbHMw9EupRRhn/PwrMbRP\n"
            "LexJpgpoVwrkDixyixigqRi9tCyTnPfs7JwLrt07i8To/vvPR8Dpmh4kw1BWyWBd\n"
            "yUzop+y2zQ==\n"
            "-----END CERTIFICATE-----\n"
            "-----BEGIN RSA PRIVATE KEY-----\n"
            "MIICXAIBAAKBgQDhCLlGzyC/rDXcbdPAUDMw5yfulZprOgJF3O3GPkpwwmtobFmo\n"
            "ptlBgX9Bgjo15JMU9bCsPXbC3xmkHo7NYV3t0E2QHceS2Iunwc9lxIQ0kuFxh07f\n"
            "jKL+1RQEDT0p+iFY79/0oGD5fLjokEI6aE+gR79pN8pJyOJGzd6MJqVg5QIDAQAB\n"
            "AoGACMzxif5htn1Cofs9k8DPL7NqkV8Sae0b16WbOuyGtsAzuSK7oufbiT9KPcKd\n"
            "CiyFQAWXwjSpJ1fYRd3YEdoj9S0xKhk2V05j42ZXDnxlau+uS1BbR/CpS6gE6wY5\n"
            "O3ItBomH2++PoICHWSnxAs/n3AAIdLsx6mjuK6rcIdAKWqUCQQDx3RXDqZlcZUVP\n"
            "Sj5CLWIot7+0N+nFFxPIJgNoPJ5SDbegIRrDeBTj1l4T4TVttrl3qKOM1jYrpT69\n"
            "Ibs7kOSrAkEA7i/THD7iVsY339isggyTYPNb3DgxBA1Vsa3fkbL4MEUZ0I4Pwfwq\n"
            "ABQ2p7Xxz0fn7BxKDCy/7Xx/UpSYkSswrwJAcB/cLR7819ai8QUsI6XCcbnth3C8\n"
            "UQBHzWvB/JrNkqCFVhjCvYd3t7/zUSgAiuJAzPZDC9Fqv4UVtrxiflTHjQJAEl5B\n"
            "y4XV8pcqq+qLsyPBIdLinKMAtK1KlH8yJIxGs4JAsWKjOHR30LW+WUSgtzl2WzD7\n"
            "TOEOlAPr1bR754YLJQJBANL+IdR345Bd11OSOFfcOWxaKXWDw1zkiLH+dmm5RlnZ\n"
            "cytgUbWfUDmqIXiEVVFNLjCZ3Z4XOoOJtd8iY34Zsdg=\n-----END RSA PRIVATE KEY-----");

    if (IMS_FILE_Exist(CERTIFICATE))
    {
        return;
    }

    IFile* piFile = IMS_FILE_Create();

    if (piFile == IMS_NULL)
    {
        return;
    }

    if (piFile->Open(CERTIFICATE, FILE_OPEN_WRITEONLY) == IMS_FALSE)
    {
        IMS_FILE_Destroy(piFile);
        return;
    }

    piFile->Write(static_cast<void*>(objCertificate.GetData()), objCertificate.GetLength());

    piFile->Close();

    IMS_FILE_Destroy(piFile);

    IMS_TRACE_D("CreateCertificate - done(%s)", m_strCertificateName.GetStr(), 0, 0);
}
