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
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#ifdef __cplusplus
extern "C"
{
#endif
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#ifdef __cplusplus
}
#endif

#include "ServiceMemory.h"
#include "ServiceThread.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"
#include "ServiceUtil.h"
#include "SslCertificate.h"
#include "network/OsSocketDef.h"
#include "network/OsSocketMsg.h"
#include "network/OsSocketService.h"
#include "network/OsSslSocket.h"

__IMS_TRACE_TAG_IPL__;

/**
 * @brief Display all the ciphers available for a specific SSL structure.
 */
#if defined(__DEBUG__)
static void osSslSocket_DisplayCiphers(IN SSL* pstSsl)
{
    AString strBuffer;

    IMS_SINT32 nIndex = 0;
    const IMS_CHAR* pszCipher = IMS_NULL;

    do
    {
        pszCipher = SSL_get_cipher_list(pstSsl, nIndex);

        if (pszCipher != IMS_NULL)
        {
            strBuffer.Append(pszCipher);
            strBuffer.Append(", ");
            nIndex++;
        }
    } while (pszCipher != IMS_NULL);

    IMS_TRACE_D("SSL ciphers :: %d [%s]", nIndex, strBuffer.GetStr(), 0);
}
#endif

class OsSsl
{
public:
    explicit OsSsl(IN const SslCertificate* pCertificate);
    ~OsSsl();

    OsSsl(IN const OsSsl&) = delete;
    OsSsl& operator=(IN const OsSsl&) = delete;

public:
    static void StartUp();
    static SslCertificate CreateDefaultCertificate();

public:
    IMS_BOOL CreateSocket(IN IMS_SOCKET hSocket);
    IMS_BOOL Connect();
    IMS_BOOL Initialize();
    IMS_SINT32 Receive(OUT IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen);
    IMS_SINT32 Send(IN const IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen);
    void ShutDown();

private:
    static IMS_SINT32 GetPemPassword(IN IMS_CHAR* pszBuffer, IN IMS_SINT32 nBuffSize,
            IN IMS_SINT32 nRwFlag, IN IMS_PVOID pvUserData);

private:
    static const IMS_CHAR DEFAULT_CA_FILE[];
    static const IMS_CHAR DEFAULT_CERTIFICATE[];
    static const IMS_CHAR DEFAULT_PASSWORD[];

    static IMS_BOOL s_bSslLibInitialized;

    SSL_CTX* m_pstCtx;
    SSL* m_pstSsl;
    BIO* m_pstSocket;

    SslCertificate m_objCertificate;
};

// Example...
// CA_FILE : "/data/local/root.pem"
// CERTIFICATE : "/system/etc/msrpcert.pem"
// PASSWORD : "password"
PRIVATE GLOBAL const IMS_CHAR OsSsl::DEFAULT_CA_FILE[] = "";
PRIVATE GLOBAL const IMS_CHAR OsSsl::DEFAULT_CERTIFICATE[] = "";
PRIVATE GLOBAL const IMS_CHAR OsSsl::DEFAULT_PASSWORD[] = "";
PRIVATE GLOBAL IMS_BOOL OsSsl::s_bSslLibInitialized = IMS_FALSE;

PUBLIC
OsSsl::OsSsl(IN const SslCertificate* pCertificate) :
        m_pstCtx(IMS_NULL),
        m_pstSsl(IMS_NULL),
        m_pstSocket(IMS_NULL)
{
    if (pCertificate != IMS_NULL)
    {
        m_objCertificate = (*pCertificate);
    }
    else
    {
        m_objCertificate = CreateDefaultCertificate();
    }
}

PUBLIC
OsSsl::~OsSsl()
{
    m_pstSocket = IMS_NULL;

    if (m_pstSsl != IMS_NULL)
    {
        SSL_free(m_pstSsl);
        m_pstSsl = IMS_NULL;
    }

    if (m_pstCtx != IMS_NULL)
    {
        SSL_CTX_free(m_pstCtx);
        m_pstCtx = IMS_NULL;
    }
}

PUBLIC GLOBAL void OsSsl::StartUp()
{
    // Global system initialization
    if (!s_bSslLibInitialized)
    {
        IMS_TRACE_D("SSL::StartUp()", 0, 0, 0);

        SSL_library_init();
        SSL_load_error_strings();
        s_bSslLibInitialized = IMS_TRUE;
    }
}

PUBLIC GLOBAL SslCertificate OsSsl::CreateDefaultCertificate()
{
    SslCertificate objCertificate(DEFAULT_CERTIFICATE);

    objCertificate.SetCaFile(DEFAULT_CA_FILE);
    objCertificate.SetPassword(DEFAULT_PASSWORD);

    return objCertificate;
}

PUBLIC
IMS_BOOL OsSsl::Initialize()
{
    // TLSv1 : TLSv1_method, TLSv1_server_method, TLSv1_client_method
    // FIXME: consider TLS_method() for generic scheme
    const SSL_METHOD* pMethod = TLSv1_method();

    m_pstCtx = SSL_CTX_new(pMethod);

    if (m_pstCtx == IMS_NULL)
    {
        IMS_TRACE_E(0, "Instantiating a SSL_CTX failed", 0, 0, 0);
        return IMS_FALSE;
    }

    // Load our keys and certificates
    if (m_objCertificate.GetKeyFile().GetLength() > 0)
    {
        if (SSL_CTX_use_certificate_chain_file(m_pstCtx, m_objCertificate.GetKeyFile().GetStr()) !=
                1)
        {
            IMS_TRACE_E(0, "Setting a certificate chain file (%s) failed",
                    m_objCertificate.GetKeyFile().GetStr(), 0, 0);
            return IMS_FALSE;
        }

        IMS_SINT32 nFileType = SSL_FILETYPE_PEM;

        if (m_objCertificate.GetKeyFileType() == SslCertificate::FILETYPE_ASN1)
        {
            nFileType = SSL_FILETYPE_ASN1;
        }

        if (SSL_CTX_use_PrivateKey_file(
                    m_pstCtx, m_objCertificate.GetKeyFile().GetStr(), nFileType) != 1)
        {
            IMS_TRACE_E(0, "Setting a private key file (%s) failed",
                    m_objCertificate.GetKeyFile().GetStr(), 0, 0);
            return IMS_FALSE;
        }
    }

    if (m_objCertificate.GetPassword().GetLength() > 0)
    {
        SSL_CTX_set_default_passwd_cb(m_pstCtx, OsSsl::GetPemPassword);
        SSL_CTX_set_default_passwd_cb_userdata(m_pstCtx, reinterpret_cast<void*>(this));
    }

    // Sets default locations for trusted CA certificates : CA file, CA path
    const IMS_CHAR* pszCaFile = IMS_NULL;
    const IMS_CHAR* pszCaPath = IMS_NULL;

    if (m_objCertificate.GetCaFile().GetLength() > 0)
    {
        pszCaFile = m_objCertificate.GetCaFile().GetStr();
    }

    if (m_objCertificate.GetCaPath().GetLength() > 0)
    {
        pszCaPath = m_objCertificate.GetCaPath().GetStr();
    }

    if ((pszCaFile != IMS_NULL) || (pszCaPath != IMS_NULL))
    {
        if (SSL_CTX_load_verify_locations(m_pstCtx, pszCaFile, pszCaPath) != 1)
        {
            IMS_TRACE_E(
                    0, "Setting the default locations for trusted CA certificates failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }

#if (OPENSSL_VERSION_NUMBER < 0x00905100L)
    SSL_CTX_set_verify_depth(m_pstCtx, 1);
#endif

    // Choose list of available SSL_CIPHERs
    if (m_objCertificate.GetCiphers().GetLength() > 0)
    {
        if (SSL_CTX_set_cipher_list(m_pstCtx, m_objCertificate.GetCiphers().GetStr()) != 1)
        {
            IMS_TRACE_E(0, "Setting a cipher list for CTX failed", 0, 0, 0);
        }
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL OsSsl::CreateSocket(IN IMS_SOCKET hSocket)
{
    if (m_pstCtx == IMS_NULL)
    {
        IMS_TRACE_E(0, "SSL_CTX is null", 0, 0, 0);
        return IMS_FALSE;
    }

    m_pstSsl = SSL_new(m_pstCtx);

    if (m_pstSsl == IMS_NULL)
    {
        IMS_TRACE_E(0, "Instantiating a SSL failed", 0, 0, 0);
        return IMS_FALSE;
    }

    m_pstSocket = BIO_new_socket(hSocket, BIO_NOCLOSE);

    if (m_pstSocket == IMS_NULL)
    {
        IMS_TRACE_E(0, "Instantiating a socket BIO failed", 0, 0, 0);
        return IMS_FALSE;
    }

    SSL_set_bio(m_pstSsl, m_pstSocket, m_pstSocket);

#if defined(__DEBUG__)
    osSslSocket_DisplayCiphers(m_pstSsl);
#endif

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL OsSsl::Connect()
{
    if (m_pstSsl == IMS_NULL)
    {
        IMS_TRACE_E(0, "SSL object is null", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_SINT32 nResult = SSL_connect(m_pstSsl);

    /*
    SSL_ERROR_NONE                 0
    SSL_ERROR_SSL                  1
    SSL_ERROR_WANT_READ            2
    SSL_ERROR_WANT_WRITE           3
    SSL_ERROR_WANT_X509_LOOKUP     4
    SSL_ERROR_SYSCALL              5  // look at error stack/return value/errno
    SSL_ERROR_ZERO_RETURN          6
    SSL_ERROR_WANT_CONNECT         7
    SSL_ERROR_WANT_ACCEPT          8
    */

    if (nResult <= 0)
    {
        IMS_SINT32 nSslErr = SSL_get_error(m_pstSsl, nResult);

        if ((nSslErr == SSL_ERROR_WANT_READ) || (nSslErr == SSL_ERROR_WANT_WRITE))
        {
            IMS_TRACE_D("SSL::Connect - try again; error(ssl=%d, err=%d(%s))", nSslErr,
                    ERR_get_error(), ERR_reason_error_string(ERR_get_error()));
        }
        else
        {
            IMS_TRACE_E(0, "SSL::Connect - error(ssl=%d, err=%d(%s))", nSslErr, ERR_get_error(),
                    ERR_reason_error_string(ERR_get_error()));
        }

        return IMS_FALSE;
    }
    else
    {
        IMS_TRACE_D("SSL::Connect - OK (%d)", SSL_get_error(m_pstSsl, nResult), 0, 0);
    }

    return IMS_TRUE;
}

PUBLIC
IMS_SINT32 OsSsl::Receive(OUT IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen)
{
    if (m_pstSsl == IMS_NULL)
    {
        IMS_TRACE_E(0, "SSL is null", 0, 0, 0);
        return ISocket::RESULT_ERROR;
    }

    IMS_SINT32 nReadBytes = SSL_read(m_pstSsl, pBuffer, nBuffLen);

    if (nReadBytes > 0)
    {
        return nReadBytes;
    }

    IMS_SINT32 nErrorCode = SSL_get_error(m_pstSsl, nReadBytes);

    switch (nErrorCode)
    {
        // read complete. there are NO MORE bytes.
        case SSL_ERROR_NONE:
        {
            return nReadBytes;
        }
            // read success, and there are more bytes to read. WOULD BLOCK
        case SSL_ERROR_WANT_READ:
        {
            IMS_TRACE_D("SSL::Receive - Operation is not completed (%d)", nReadBytes, 0, 0);
            return ISocket::RESULT_WOULDBLOCK;
        }
        case SSL_ERROR_ZERO_RETURN:
        {
            IMS_TRACE_D("SSL::Receive - SSL connection has been closed", 0, 0, 0);
            return ISocket::RESULT_ERROR;
        }
        case SSL_ERROR_SYSCALL:
        {
            IMS_SINT32 nError = errno;

            IMS_TRACE_E(0, "Receive - SSL_ERROR_SYSCALL(%d, %d, %s)", ERR_get_error(), nError,
                    strerror(nError));

            if ((nError == EINTR) || (nError == EINPROGRESS) || (nError == EWOULDBLOCK))
            {
                return ISocket::RESULT_WOULDBLOCK;
            }

            return ISocket::RESULT_ERROR;
        }
        default:
        {
            IMS_SINT32 nError = errno;

            IMS_TRACE_E(0, "Receive - SSL error(%d, %s, errno=%d)", nErrorCode,
                    ERR_error_string(nErrorCode, IMS_NULL), nError);

            if ((nError == EINTR) || (nError == EINPROGRESS) || (nError == EWOULDBLOCK))
            {
                return ISocket::RESULT_WOULDBLOCK;
            }

            return ISocket::RESULT_ERROR;
        }
    }
}

PUBLIC
IMS_SINT32 OsSsl::Send(IN const IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen)
{
    if (m_pstSsl == IMS_NULL)
    {
        IMS_TRACE_E(0, "SSL is null", 0, 0, 0);
        return ISocket::RESULT_ERROR;
    }

    IMS_SINT32 nWrittenBytes = SSL_write(m_pstSsl, pBuffer, nBuffLen);

    if (nWrittenBytes > 0)
    {
        if (nWrittenBytes != nBuffLen)
        {
            IMS_TRACE_D("SSL::Send - SSL write(%d/%d)", nWrittenBytes, nBuffLen, 0);
        }

        return nWrittenBytes;
    }

    IMS_SINT32 nErrorCode = SSL_get_error(m_pstSsl, nWrittenBytes);

    switch (nErrorCode)
    {
        case SSL_ERROR_NONE:
            IMS_TRACE_D("SSL::Send - No error", 0, 0, 0);
            break;

        case SSL_ERROR_WANT_WRITE:
            // WOULDBLOCK
            IMS_TRACE_D("SSL::Send - Operation is not completed (%d)", nWrittenBytes, 0, 0);
            return ISocket::RESULT_WOULDBLOCK;

        case SSL_ERROR_ZERO_RETURN:
            IMS_TRACE_D("SSL::Send - SSL connection has been closed", 0, 0, 0);
            return ISocket::RESULT_ERROR;

        case SSL_ERROR_SYSCALL:
            IMS_TRACE_E(0, "SSL::Send - SSL_ERROR_SYSCALL", 0, 0, 0);
            return ISocket::RESULT_ERROR;

        default:
            IMS_TRACE_E(0, "SSL::Send - SSL error(%d, %s)", nErrorCode,
                    ERR_error_string(nErrorCode, IMS_NULL), 0);
            return ISocket::RESULT_ERROR;
    }

    return nWrittenBytes;
}

PUBLIC
void OsSsl::ShutDown()
{
    if (m_pstSsl != IMS_NULL)
    {
        SSL_shutdown(m_pstSsl);
    }
}

PRIVATE GLOBAL IMS_SINT32 OsSsl::GetPemPassword(IN IMS_CHAR* pszBuffer, IN IMS_SINT32 nBuffSize,
        IN IMS_SINT32 nRwFlag, IN IMS_PVOID pvUserData)
{
    // nRwFlag : 0 - reading/decryption, 1 - writing/encryption
    (void)nRwFlag;

    if (nBuffSize == 0)
    {
        return 0;
    }

    OsSsl* pSsl = reinterpret_cast<OsSsl*>(pvUserData);

    if (pSsl == IMS_NULL)
    {
        IMS_TRACE_E(0, "OsSsl is null", 0, 0, 0);
        return 0;
    }

    if (pSsl->m_objCertificate.GetPassword().GetLength() == 0)
    {
        return 0;
    }

    if (nBuffSize <= pSsl->m_objCertificate.GetPassword().GetLength())
    {
        strncpy(pszBuffer, pSsl->m_objCertificate.GetPassword().GetStr(), nBuffSize);
        pszBuffer[nBuffSize - 1] = '\0';

        return (nBuffSize - 1);
    }
    else
    {
        strncpy(pszBuffer, pSsl->m_objCertificate.GetPassword().GetStr(),
                pSsl->m_objCertificate.GetPassword().GetLength());
        pszBuffer[pSsl->m_objCertificate.GetPassword().GetLength()] = '\0';

        return pSsl->m_objCertificate.GetPassword().GetLength();
    }
}

PUBLIC
OsSslSocket::OsSslSocket(IN SslCertificate* pCertificate) :
        OsSocket(),
        m_nSslState(SSL_STATE_IDLE),
        m_nSslConnectRetryCount(0),
        m_piSslConnectRetryTimer(IMS_NULL),
        m_pSsl(new OsSsl(pCertificate))
{
    LoadLibrary();

    if (m_pSsl != IMS_NULL)
    {
        m_pSsl->Initialize();
    }
}

PUBLIC VIRTUAL OsSslSocket::~OsSslSocket()
{
    IMS_TRACE_D("Destructor :: OsSslSocket", 0, 0, 0);

    IMS_SINT32 nOption = GetOptionForShutdown();
    ShutDown((nOption < 0) ? SHUTDOWN_BOTH : nOption);

    if (m_pSsl != IMS_NULL)
    {
        delete m_pSsl;
        m_pSsl = IMS_NULL;
    }
}

PUBLIC GLOBAL void OsSslSocket::LoadLibrary()
{
    OsSsl::StartUp();
}

PROTECTED VIRTUAL ISocket::SOCKET_RESULT OsSslSocket::Open(IN SOCKET_ENTYPE eType,
        IN ISocketListener* piListener,
        IN ADDRESS_FAMILY_ENTYPE eAddrFamily /*= ADDRESS_FAMILY_INET*/)
{
    if (m_pSsl == IMS_NULL)
    {
        IMS_TRACE_E(0, "SSL is null", 0, 0, 0);
        return RESULT_ERROR;
    }

    if (OsSocket::Open(eType, piListener, eAddrFamily) == ISocket::RESULT_ERROR)
    {
        return RESULT_ERROR;
    }

    if (!m_pSsl->CreateSocket(GetSocket()))
    {
        IMS_TRACE_E(0, "Creating SSL socket failed", 0, 0, 0);
        return ISocket::RESULT_ERROR;
    }

    return RESULT_SUCCESS;
}

PROTECTED VIRTUAL ISocket::SOCKET_RESULT OsSslSocket::Open(
        IN SOCKET_ENTYPE eType, IN ADDRESS_FAMILY_ENTYPE eAddrFamily /*= ADDRESS_FAMILY_INET*/)
{
    if (m_pSsl == IMS_NULL)
    {
        IMS_TRACE_E(0, "SSL is null", 0, 0, 0);
        return RESULT_ERROR;
    }

    if (OsSocket::Open(eType, eAddrFamily) == ISocket::RESULT_ERROR)
    {
        return RESULT_ERROR;
    }

    if (!m_pSsl->CreateSocket(GetSocket()))
    {
        IMS_TRACE_E(0, "Creating SSL socket failed", 0, 0, 0);
        return RESULT_ERROR;
    }

    return RESULT_SUCCESS;
}

PROTECTED VIRTUAL IMS_SINT32 OsSslSocket::Receive(OUT IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen)
{
    if (GetSocket() == INVALID_SOCKET)
    {
        IMS_TRACE_E(0, "Socket handle is null", 0, 0, 0);
        return RESULT_ERROR;
    }

    if (m_pSsl == IMS_NULL)
    {
        IMS_TRACE_E(0, "SSL is null", 0, 0, 0);
        return RESULT_ERROR;
    }

    if (IsSslConnected())
    {
        IMS_SINT32 nReadBytes = m_pSsl->Receive(pBuffer, nBuffLen);

        if (nReadBytes < 0)
        {
            NotifyMessage(IMS_SOCKET_CLOSED);
            return RESULT_ERROR;
        }

        if (nReadBytes < nBuffLen)
        {
            SelectEventEx(FD_READ);
        }

        return nReadBytes;
    }
    else
    {
        SelectEventEx(FD_READ);

        IMS_TRACE_D("Receive() - TLS is handshaking ...", 0, 0, 0);
        return RESULT_WOULDBLOCK;
    }
}

PROTECTED VIRTUAL IMS_SINT32 OsSslSocket::Send(IN const IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen)
{
    if (GetSocket() == INVALID_SOCKET)
    {
        IMS_TRACE_E(0, "Socket handle is null", 0, 0, 0);
        return RESULT_ERROR;
    }

    if (m_pSsl == IMS_NULL)
    {
        IMS_TRACE_E(0, "SSL is null", 0, 0, 0);
        return RESULT_ERROR;
    }

    IMS_SINT32 nWrittenBytes = 0;

    if (IsSslConnected())
    {
        nWrittenBytes = m_pSsl->Send(pBuffer, nBuffLen);

        if (nWrittenBytes < 0)
        {
            NotifyMessage(IMS_SOCKET_CLOSED);
            return RESULT_ERROR;
        }
    }

    if (nWrittenBytes == ISocket::RESULT_WOULDBLOCK)
    {
        SelectEventEx(FD_WRITE);
    }

    return nWrittenBytes;
}

PROTECTED VIRTUAL void OsSslSocket::DispatchServiceMessage(
        IN IMS_UINTP nWparam, IN IMS_UINTP nLparam)
{
    switch (nWparam)
    {
        case IMS_SOCKET_SSL_HANDSHAKE:
            DoHandshake();
            break;

        default:
            OsSocket::DispatchServiceMessage(nWparam, nLparam);
            break;
    }
}

PROTECTED VIRTUAL IMS_SINT32 OsSslSocket::GetSocketState() const
{
    // If there is a problem in the TCP socket level, just return the state of TCP socket level.
    return OsSocket::GetSocketState();
}

PROTECTED VIRTUAL void OsSslSocket::NotifyConnected(IN IMS_SINT32 nErrorCode)
{
    IMS_TRACE_D("SSL::NotifyConnnected() - socket=%d, error=%d", GetSocket(), nErrorCode, 0);

    if ((nErrorCode == 0) && (GetCloseReason() == CLOSE_REASON_UNKNOWN) && !IsSslConnected())
    {
        DeselectEventEx(FD_CONNECT);

        IMS_TRACE_D("Try to connect SSL connection ...", 0, 0, 0);
        NotifyMessage(IMS_SOCKET_SSL_HANDSHAKE);
        return;
    }

    OsSocket::NotifyConnected(nErrorCode);
}

PROTECTED VIRTUAL void OsSslSocket::NotifyDataReceived(IN IMS_SINT32 nErrorCode)
{
    if (nErrorCode == 0)
    {
        if (IsSslConnected())
        {
            OsSocket::NotifyDataReceived(nErrorCode);
        }
        else
        {
            // Until the SSL handshake is completed, do not wait for the READ event
            DeselectEventEx(FD_READ);
        }
    }
    else
    {
        OsSocket::NotifyDataReceived(nErrorCode);
    }
}

PROTECTED VIRTUAL IMS_BOOL OsSslSocket::ShutDown(IN IMS_SINT32 nHow /*= SHUTDOWN_BOTH*/)
{
    OsSocket::ShutDown(nHow);

    SetSslState(SSL_STATE_IDLE);

    if (m_pSsl != IMS_NULL)
    {
        m_pSsl->ShutDown();
    }

    if (m_piSslConnectRetryTimer != IMS_NULL)
    {
        m_piSslConnectRetryTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(m_piSslConnectRetryTimer);
        m_piSslConnectRetryTimer = IMS_NULL;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL void OsSslSocket::Timer_TimerExpired(IN ITimer* piTimer)
{
    if (piTimer == IMS_NULL)
    {
        return;
    }

    if (m_piSslConnectRetryTimer == IMS_NULL)
    {
        return;
    }

    if (piTimer == m_piSslConnectRetryTimer)
    {
        ++m_nSslConnectRetryCount;

        m_piSslConnectRetryTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(m_piSslConnectRetryTimer);
        m_piSslConnectRetryTimer = IMS_NULL;

        if (m_nSslConnectRetryCount >= SSL_CONNECT_RETRY_COUNT)
        {
            IMS_TRACE_E(0, "TLS Connection fail", 0, 0, 0);
            NotifyMessage(IMS_SOCKET_CLOSED);
        }
        else
        {
            NotifyMessage(IMS_SOCKET_SSL_HANDSHAKE);
        }
    }
}

PRIVATE
void OsSslSocket::DoHandshake()
{
    if (m_pSsl == IMS_NULL)
    {
        NotifyClosed(0);
        return;
    }

    if (m_pSsl->Connect())
    {
        SelectEventEx(FD_READ);
        SetSslState(SSL_STATE_CONNECTED);

        NotifyConnected(0);
        return;
    }

    SetSslState(SSL_STATE_CONNECTING);

    // After the retry interval, try to connect again...
    m_piSslConnectRetryTimer = TimerService::GetTimerService()->CreateTimer();

    if (m_piSslConnectRetryTimer != IMS_NULL)
    {
        m_piSslConnectRetryTimer->SetTimer(SSL_CONNECT_RETRY_INTERVAL, this);
    }
    else
    {
        SetSslState(SSL_STATE_IDLE);
        NotifyClosed(0);
    }
}

PRIVATE
IMS_BOOL OsSslSocket::IsSslConnected() const
{
    return (m_nSslState == SSL_STATE_CONNECTED);
}

PRIVATE
void OsSslSocket::SetSslState(IN IMS_SINT32 nState)
{
    if (m_nSslState != nState)
    {
        IMS_TRACE_I("SSL state :: %s >> %s", SslStateToString(m_nSslState),
                SslStateToString(nState), 0);

        m_nSslState = nState;
    }
}

PRIVATE GLOBAL const IMS_CHAR* OsSslSocket::SslStateToString(IN IMS_SINT32 nState)
{
    switch (nState)
    {
        case SSL_STATE_IDLE:
            return "SSL_STATE_IDLE";
        case SSL_STATE_CONNECTING:
            return "SSL_STATE_CONNECTING";
        case SSL_STATE_CONNECTED:
            return "SSL_STATE_CONNECTED";
        default:
            return "__INVALID__";
    }
}
