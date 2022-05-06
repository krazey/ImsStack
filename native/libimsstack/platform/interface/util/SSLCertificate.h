/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20111207  hwangoo.park@               Created
    </table>

    Description

*/

#ifndef _SSL_CERTIFICATE_H_
#define _SSL_CERTIFICATE_H_

#include "AString.h"

class SSLCertificate
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
    SSLCertificate();
    explicit SSLCertificate(IN CONST AString& strKeyFile_, IN IMS_SINT32 nFileType_ = FILETYPE_PEM);
    SSLCertificate(IN CONST SSLCertificate& objRHS);
    ~SSLCertificate();

public:
    SSLCertificate& operator=(IN CONST SSLCertificate& objRHS);

public:
    const AString& GetCAFile() const;
    const AString& GetCAPath() const;
    const AString& GetCiphers() const;
    const AString& GetKeyFile() const;
    IMS_SINT32 GetKeyFileType() const;
    const AString& GetPassword() const;

    void SetCAFile(IN CONST AString& strCAFile);
    void SetCAPath(IN CONST AString& strCAPath);
    void SetCiphers(IN CONST AString& strCiphers);
    void SetKeyFile(IN CONST AString& strKeyFile);
    void SetKeyFileType(IN IMS_SINT32 nKeyFileType);
    void SetPassword(IN CONST AString& strPassword);

private:
    // Key file info.
    IMS_SINT32 nKeyFileType;
    AString strKeyFile;

    // Password info.
    AString strPassword;

    // CA certificates
    AString strCAFile;
    AString strCAPath;

    // Cipher strings (separated by colons)
    AString strCiphers;
};

#endif  // _SSL_CERTIFICATE_H_
