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
#ifndef __SIP_MSG_BODY_H__
#define __SIP_MSG_BODY_H__

#include "SipDatatypes.h"
#include "SipRefBase.h"
#include "msg/SipContentTypeHeader.h"
#include "msg/SipHeaderList.h"

class SipMsgBody;

class SipMIMEHdrs : public SipRefBase
{
public:
    enum
    {
        CONTENT_TYPE,
        CONTENT_ENCODING,
        CONTENT_DISPOSITION,
        UNKNOWN,
        END,
        INVALID
    };

private:
    SipContentTypeHeader* m_pContentType;
    SipHeaderBase* m_pContentEncoding;
    SipHeaderBase* m_pContentDisposition;
    SipHeaderList* m_pUnKnownHdrList;

public:
    SipMIMEHdrs();
    SipMIMEHdrs(const SipMIMEHdrs& objMimeHdr);

    SIP_BOOL EncodeMIMEHdrs(SIP_CHAR** ppCurrPos);

    SIP_BOOL DecodeMIMEHdrs(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);

    SipHeaderBase* GetNewMIMEHdrObj(SIP_INT32 eHdrType);

    SipHeaderBase* GetUnknownHdr(SIP_UINT32 nIndex);
    inline SIP_UINT32 GetUnknownHdrCount() const
    {
        return (m_pUnKnownHdrList != SIP_NULL) ? m_pUnKnownHdrList->GetSize() : SIP_ZERO;
    }

    SIP_BOOL SetMimeHdrs(SipHeaderBase* pHdr);

    SipHeaderBase* GetMimeHdrObj(SIP_INT32 eIndex);

private:
    virtual ~SipMIMEHdrs();
};

/*Class for message body list*/
class SipMsgBodyList : public SipRefBase
{
    SipVector<SipMsgBody*> m_objBodyList;

public:
    SipMsgBodyList();
    SipMsgBodyList(const SipMsgBodyList& objMsgBodyList);

    SipMsgBody* GetBodyByIndex(SIP_UINT32 nIndex);

    SIP_BOOL AddBody(SipMsgBody* pMsgBody);

    inline SIP_UINT32 GetMsgBodyCount() const { return m_objBodyList.GetSize(); }

    SIP_BOOL GetEncodedMessageBody(
            SIP_CHAR** ppMsgBufer, SIP_UINT32& nMsgLen, const SIP_CHAR* pszBoundary = SIP_NULL);

    SIP_BOOL EncodeBody(SIP_CHAR** ppMsgBuffCurrPos, const SIP_CHAR* pszBoundary);

    SIP_BOOL DecodeMIMEBody(
            const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt, const SIP_CHAR* pszBoundary);
    /*Function for decoding of headers*/
    SIP_BOOL DecodeSingleBody(const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt);

private:
    ~SipMsgBodyList();
};

class SipMsgBody : public SipRefBase
{
public:
    enum
    {
        SINGLE_BODY,
        MULTI_PART_BODY,
        INVALID_BODY
    };

private:
    /*Body type*/
    SIP_INT32 m_eBodyType;
    /*Set of Mime Headers*/
    SipMIMEHdrs* m_pMIMEHdrs;
    /* Msg body buffer*/
    SIP_CHAR* m_pBuffer;
    /* Content Buffer Length*/
    SIP_UINT32 m_nBufLen;
    /*List of self referential objects*/
    SipMsgBodyList* m_pBodyList;  // list of SipMsgBody
    /*Flag to disable encoding of MIME header*/
    SIP_BOOL m_bEncodeMime;

public:
    /*Constructor*/
    SipMsgBody();
    explicit SipMsgBody(SIP_INT32 eBodyType);

    SipMsgBody(const SipMsgBody& objMsgBody);

    /*Function for encoding*/
    SIP_BOOL EncodeSingleMsgBody(SIP_CHAR** ppCurrPos);

    SIP_BOOL EncodeMIMEMsgBody(SIP_CHAR** ppCurrPos);

    SIP_BOOL EncodeBody(SIP_CHAR** ppCurrPos);

    /*Function for decoding*/
    SIP_BOOL DecodeSingleMsgBody(const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt);

    SIP_BOOL DecodeMIMEMsgBody(const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt);

    inline SIP_BOOL IsMimeEncoding() const { return m_bEncodeMime; }
    /*Set Methods*/
    SIP_BOOL SetMimeHdr(SipHeaderBase* pHdrBase);

    SIP_BOOL SetMsgBuffer(const SIP_CHAR* pMsgBuffer, SIP_UINT32 nBufLen);

    /*Get Methods*/

    inline SIP_INT32 GetBodyType() const { return m_eBodyType; }

    SipMsgBodyList* GetMessageBodyList();

    SipContentTypeHeader* GetContentType();

    SipHeaderBase* GetContentEncoding();

    SipHeaderBase* GetContentDisposition();

    SIP_UINT32 GetUnknownHdrCount() const
    {
        return (m_pMIMEHdrs != SIP_NULL) ? m_pMIMEHdrs->GetUnknownHdrCount() : SIP_ZERO;
    }

    SIP_BOOL IsMessageBodySDP();

    SIP_BOOL GetMsgBuffer(SIP_CHAR** ppMsgBuffer);

    inline SIP_VOID GetMsgBuffLen(SIP_UINT32* pnMsgBufLen) const { *pnMsgBufLen = m_nBufLen; }

    SipHeaderBase* GetMimeHdr(SIP_INT32 eHdrType, SIP_UINT32 nIndex);

    inline SIP_CHAR* GetBuffer() const { return m_pBuffer; }
    inline SIP_UINT32 GetBufferLength() const { return m_nBufLen; }

private:
    ~SipMsgBody();
};

#endif  //__SIP_MSG_BODY_H__
