/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20111207  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "SSLCertificate.h"

PUBLIC
SSLCertificate::SSLCertificate() :
        nKeyFileType(FILETYPE_PEM),
        strKeyFile(AString::ConstNull()),
        strPassword(AString::ConstNull()),
        strCAFile(AString::ConstNull()),
        strCAPath(AString::ConstNull()),
        strCiphers(AString::ConstNull())
{
}

PUBLIC
SSLCertificate::SSLCertificate(
        IN CONST AString& strKeyFile_, IN IMS_SINT32 nKeyFileType_ /* = FILETYPE_PEM */) :
        nKeyFileType(nKeyFileType_),
        strKeyFile(strKeyFile_),
        strPassword(AString::ConstNull()),
        strCAFile(AString::ConstNull()),
        strCAPath(AString::ConstNull()),
        strCiphers(AString::ConstNull())
{
}

PUBLIC
SSLCertificate::SSLCertificate(IN CONST SSLCertificate& objRHS) :
        nKeyFileType(objRHS.nKeyFileType),
        strKeyFile(objRHS.strKeyFile),
        strPassword(objRHS.strPassword),
        strCAFile(objRHS.strCAFile),
        strCAPath(objRHS.strCAPath),
        strCiphers(objRHS.strCiphers)
{
}

PUBLIC
SSLCertificate::~SSLCertificate() {}

/*

Remarks

*/
PUBLIC
SSLCertificate& SSLCertificate::operator=(IN CONST SSLCertificate& objRHS)
{
    if (this != &objRHS)
    {
        nKeyFileType = objRHS.nKeyFileType;
        strKeyFile = objRHS.strKeyFile;

        strPassword = objRHS.strPassword;

        strCAFile = objRHS.strCAFile;
        strCAPath = objRHS.strCAPath;

        strCiphers = objRHS.strCiphers;
    }

    return (*this);
}

/*

Remarks

*/
PUBLIC
const AString& SSLCertificate::GetCAFile() const
{
    return strCAFile;
}

/*

Remarks

*/
PUBLIC
const AString& SSLCertificate::GetCAPath() const
{
    return strCAPath;
}

/*

Remarks

*/
PUBLIC
const AString& SSLCertificate::GetCiphers() const
{
    return strCiphers;
}

/*

Remarks

*/
PUBLIC
const AString& SSLCertificate::GetKeyFile() const
{
    return strKeyFile;
}

/*

Remarks

*/
PUBLIC
IMS_SINT32 SSLCertificate::GetKeyFileType() const
{
    return nKeyFileType;
}

/*

Remarks

*/
PUBLIC
const AString& SSLCertificate::GetPassword() const
{
    return strPassword;
}

/*

Remarks

*/
PUBLIC
void SSLCertificate::SetCAFile(IN CONST AString& strCAFile)
{
    this->strCAFile = strCAFile;
}

/*

Remarks

*/
PUBLIC
void SSLCertificate::SetCAPath(IN CONST AString& strCAPath)
{
    this->strCAPath = strCAPath;
}

/*

Remarks

*/
PUBLIC
void SSLCertificate::SetCiphers(IN CONST AString& strCiphers)
{
    this->strCiphers = strCiphers;
}

/*

Remarks

*/
PUBLIC
void SSLCertificate::SetKeyFile(IN CONST AString& strKeyFile)
{
    this->strKeyFile = strKeyFile;
}

/*

Remarks

*/
PUBLIC
void SSLCertificate::SetKeyFileType(IN IMS_SINT32 nKeyFileType)
{
    this->nKeyFileType = nKeyFileType;
}

/*

Remarks

*/
PUBLIC
void SSLCertificate::SetPassword(IN CONST AString& strPassword)
{
    this->strPassword = strPassword;
}
