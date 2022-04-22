/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20111009  il.won@                   Created

    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceFile.h"
#include "ServiceUtil.h"
#include "ImsConstDef.h"
#include "ByteArray.h"
#include "CertificateHelper.h"

__IMS_TRACE_TAG_ADAPT__;

//const IMS_CHAR * const CertificateHelper::CERTIFICATE = "/system/etc/msrpcert.pem";
const IMS_CHAR * const CertificateHelper::CERTIFICATE
        = IMS_SOLUTION_STORAGE_ROOT_DIR "/databases/client.pem";

PRIVATE
CertificateHelper::CertificateHelper()
{
    Init();
}

PRIVATE
void CertificateHelper::Init()
{
    // Create a certificate
    CreateCertificate();

    // Set the certificate path
    strCertificateName = CERTIFICATE;

    // Generate the fingerprint
    UtilService::GetUtilService()->GetSystemUtil()->DigestSha1(
            strCertificateName, strFingerPrint);

    IMS_TRACE_D("Init :: Certificate(%s) -> Fingerprint(%s)",
            strCertificateName.GetStr(), strFingerPrint.GetStr(), 0);
}

PUBLIC
CertificateHelper::~CertificateHelper()
{

}

PUBLIC GLOBAL
CertificateHelper* CertificateHelper::GetInstance()
{
    static CertificateHelper* pCH = IMS_NULL;

    if (pCH == IMS_NULL)
    {
        pCH = new CertificateHelper();
    }

    return pCH;
}

PUBLIC
const AString& CertificateHelper::GetCertificateName()
{
    return strCertificateName;
}

PUBLIC
const AString& CertificateHelper::GetFingerPrint()
{
    // This is the correct fingerprint.
    // strFingerPrint = "06:11:F7:C0:1F:93:54:0F:50:AA:95:A9:84:55:7B:CD:09:78:B9:EB";

    return strFingerPrint;
}

PRIVATE
void CertificateHelper::CreateCertificate()
{
    ByteArray objCertificate;
    objCertificate.Append("-----BEGIN CERTIFICATE-----\n" \
            "MIICEzCCAXwCCQD9XU1OoCVDrjANBgkqhkiG9w0BAQUFADBNMQswCQYDVQQGEwJF\n" \
            "UzEPMA0GA1UECBMGTWFkcmlkMQ8wDQYDVQQHEwZNYWRyaWQxDDAKBgNVBAoTA1ZE\n" \
            "RjEOMAwGA1UEAxMFTEdSQ1MwIBcNMTExMDA4MDE0MDQyWhgPMjExMTA5MTQwMTQw\n" \
            "NDJaME0xCzAJBgNVBAYTAkVTMQ8wDQYDVQQIEwZNYWRyaWQxDzANBgNVBAcTBk1h\n" \
            "ZHJpZDEMMAoGA1UEChMDVkRGMQ4wDAYDVQQDEwVMR1JDUzCBnzANBgkqhkiG9w0B\n" \
            "AQEFAAOBjQAwgYkCgYEA4Qi5Rs8gv6w13G3TwFAzMOcn7pWaazoCRdztxj5KcMJr\n" \
            "aGxZqKbZQYF/QYI6NeSTFPWwrD12wt8ZpB6OzWFd7dBNkB3HktiLp8HPZcSENJLh\n" \
            "cYdO34yi/tUUBA09KfohWO/f9KBg+Xy46JBCOmhPoEe/aTfKScjiRs3ejCalYOUC\n" \
            "AwEAATANBgkqhkiG9w0BAQUFAAOBgQA373ZRHDM8Ajw5dgbdfXaJWOW+/tCSpJKO\n" \
            "2E/Q5Atk0afPnaOdDvanTYQxIGTmuihhYymyhoNHeJ5JbHMw9EupRRhn/PwrMbRP\n" \
            "LexJpgpoVwrkDixyixigqRi9tCyTnPfs7JwLrt07i8To/vvPR8Dpmh4kw1BWyWBd\n" \
            "yUzop+y2zQ==\n" \
            "-----END CERTIFICATE-----\n" \
            "-----BEGIN RSA PRIVATE KEY-----\n" \
            "MIICXAIBAAKBgQDhCLlGzyC/rDXcbdPAUDMw5yfulZprOgJF3O3GPkpwwmtobFmo\n" \
            "ptlBgX9Bgjo15JMU9bCsPXbC3xmkHo7NYV3t0E2QHceS2Iunwc9lxIQ0kuFxh07f\n" \
            "jKL+1RQEDT0p+iFY79/0oGD5fLjokEI6aE+gR79pN8pJyOJGzd6MJqVg5QIDAQAB\n" \
            "AoGACMzxif5htn1Cofs9k8DPL7NqkV8Sae0b16WbOuyGtsAzuSK7oufbiT9KPcKd\n" \
            "CiyFQAWXwjSpJ1fYRd3YEdoj9S0xKhk2V05j42ZXDnxlau+uS1BbR/CpS6gE6wY5\n" \
            "O3ItBomH2++PoICHWSnxAs/n3AAIdLsx6mjuK6rcIdAKWqUCQQDx3RXDqZlcZUVP\n" \
            "Sj5CLWIot7+0N+nFFxPIJgNoPJ5SDbegIRrDeBTj1l4T4TVttrl3qKOM1jYrpT69\n" \
            "Ibs7kOSrAkEA7i/THD7iVsY339isggyTYPNb3DgxBA1Vsa3fkbL4MEUZ0I4Pwfwq\n" \
            "ABQ2p7Xxz0fn7BxKDCy/7Xx/UpSYkSswrwJAcB/cLR7819ai8QUsI6XCcbnth3C8\n" \
            "UQBHzWvB/JrNkqCFVhjCvYd3t7/zUSgAiuJAzPZDC9Fqv4UVtrxiflTHjQJAEl5B\n" \
            "y4XV8pcqq+qLsyPBIdLinKMAtK1KlH8yJIxGs4JAsWKjOHR30LW+WUSgtzl2WzD7\n" \
            "TOEOlAPr1bR754YLJQJBANL+IdR345Bd11OSOFfcOWxaKXWDw1zkiLH+dmm5RlnZ\n" \
            "cytgUbWfUDmqIXiEVVFNLjCZ3Z4XOoOJtd8iY34Zsdg=\n-----END RSA PRIVATE KEY-----");

    if (IMS_FILE_Exist(CERTIFICATE))
    {
        return;
    }

    IFile *piFile = IMS_FILE_Create();

    if (piFile == IMS_NULL)
    {
        return;
    }

    if (piFile->Open(CERTIFICATE, FILE_OPEN_WRITEONLY) == IMS_FALSE)
    {
        IMS_FILE_Destroy(piFile);
        return;
    }

    piFile->Write((void*)objCertificate.GetData(), objCertificate.GetLength());

    piFile->Close();

    IMS_FILE_Destroy(piFile);

    IMS_TRACE_D("CreateCertificate - done(%s)", strCertificateName.GetStr(), 0, 0);
}
