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
#ifndef __SIP_STACK_MANAGER_H__
#define __SIP_STACK_MANAGER_H__

#include "SipDatatypes.h"

class SipMessage;
class SipTxnKey;
class SipUtil;
class ISipUserData;
class SipTransportInfo;
class SipTransportParameter;

/**
  This is the entry point for the SIP stack. This can handle transaction and transport.
  The message to be sent is encoded and sent. It processes the received responses. It starts
  suitable transaction timers.
 */
class SipStackManager
{
    SipStackManager();

public:
    virtual ~SipStackManager();
    SipUtil* GetSipUtil();
    static SipStackManager* GetInstance();
    static void Destruct();

    /*!
     * @brief This API is called by stack user to send SIP message(request/response) to other end.
     *    This function handle transaction, transport and issue callback for sending sip message to
     * network
     *
     * @param[in,out] pSipMsg        : SIP message object used for forming raw SIP message
     * @param[in]     pTranspParam   : For Request message it contains transport information where
     *    request to be send. For response, remote transport information is fetched from the Via
     * header
     * @param[in]     pUserData        : For request message, it contains user data which will be
     *    returned in the SendToNetwork call back.
     * @param[out]     ppTxnKey        : For request message, new key will be formed and return to
     *    user in this parameter. for response message, ket which is available in the Txn object is
     * returned.
     *
     * @return Status indicator
     * @retval SIP_TRUE If successful
     * @retval SIP_FALSE If function processing failed.
     * @retval Appropriate error code as defined in SipEn_ErrorTypes in case of failure
     *
     * Re-transmission of 2XX for INVITE is handled by stack user. 2XX and successful ACK for INVITE
     * is not handled by the transaction layer and is directly passed to transport layer for
     * encoding and sending to network.
     */
    SIP_BOOL SendMsg(SipMessage* pSipMsg, IN_OUT SipTransportParameter* pTranspParam,
            IN ISipUserData* pUserData, IN const SIP_CHAR* pSipBuffer, IN SIP_UINT32 nSipBufferLen,
            IN SipTxnKey** ppTxnKey, OUT SIP_UINT16* pnError);

    /**
      @param pSipMsg        : Received SIP Msg (Req/Resp)
      @param pTranspParam    : source address from where data is received
      and response to be sent.
      @param peTranspStatus    : status of handling of received msg.
     */
    SIP_BOOL OnRecvMessage(SipMessage* pSipMsg, IN_OUT SipTransportParameter* pTranspParam,
            IN ISipUserData* pUserData, IN SIP_INT32* peTxnStatus, OUT SipTxnKey** ppTxnKey,
            OUT SIP_UINT16* pnError);

    /**
      This function is used to convey transport errors to the stack. When application
      detects transport errors, it will inform the stack using this function.
      @param eTranspError        : Received transport Error
      @param pTxnKey        : key of Txn's, for which transport error occurred.
     */
    SIP_BOOL OnRecvTanspError(SIP_INT32 eTranspError, SipTxnKey* pTxnKey, SIP_UINT16* pnError);

    /**
      Any ongoing transaction can be terminated by the application using this function with
      the help of the transaction key
      @param pTxnKey        : key of Txn's for terminating.
     */
    SIP_BOOL TerminateTxn(SipTxnKey* pTxnKey);

private:
    PRIVATE SIP_BOOL SendToNetwork(IN SipTransportInfo* pTranspInfo, IN ISipUserData* pUserData);
};

#endif  //__SIP_STACK_MANAGER_H__
