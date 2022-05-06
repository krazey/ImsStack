/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20111009  il.won@                   Created

    </table>

    Description

*/

#ifndef _CERTIFICATE_HELPER_H_
#define _CERTIFICATE_HELPER_H_

class CertificateHelper
{
public:
    virtual ~CertificateHelper();

private:
    CertificateHelper();

public:
    static CertificateHelper* GetInstance();

    const AString& GetCertificateName();
    const AString& GetFingerPrint();

private:
    void Init();
    void CreateCertificate();

private:
    static const IMS_CHAR* const CERTIFICATE;
    AString strCertificateName;
    AString strFingerPrint;
};

#endif  // _CERTIFICATE_HELPER_H_
