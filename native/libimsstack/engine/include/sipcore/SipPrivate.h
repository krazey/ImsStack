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
#ifndef SIP_PRIVATE_H_
#define SIP_PRIVATE_H_

#include "ServiceTrace.h"

#include "Sip.h"

class SipPrivate
{
public:
    SipPrivate() = delete;

public:
    static void Init();
    static void Init(IN IMS_SINT32 nSlotId, IN IMS_SINT32 nEncodingOptions);

    /*
     Sets an error code which has been occurred in the J180 function block lastly
    */
    static void SetLastError(IN IMS_SINT32 nErrorCode);

    /*
     Returns a last error code
    */
    static IMS_SINT32 GetLastError();

    // Returns the SIP encoding options
    static IMS_SINT32 GetEncodingOptions();

private:
    /*
     Sets an error code which has been occurred in the J180 function block lastly
    */
    static void SetLastError(IN IMS_SINT32 nErrorCode, IN IMS_SINT32 nSlotId);

    /*
     Returns a last error code
    */
    static IMS_SINT32 GetLastError(IN IMS_SINT32 nSlotId);

    // Returns the SIP encoding options
    static IMS_SINT32 GetEncodingOptions(IN IMS_SINT32 nSlotId);

public:
    /// Result values for the received message handling
    enum
    {
        /// Message is valid
        MESSAGE_VALID = 0,
        /// Forked response message received
        MESSAGE_VALID_FORKED,
        /// Invalid message received in the current state
        MESSAGE_INVALID,
        MESSAGE_INVALID_400,
        /// Method not allowed
        MESSAGE_INVALID_405,
        /// Invalid message received & needs to be sent 481 response
        MESSAGE_INVALID_481,
        MESSAGE_INVALID_500,
        /// Message received is to be discarded. Reasons include:
        /// - Lower CSeq request
        /// - Response received with topmost Via having a "sent-by" field
        ///  which is not same as local host & port
        /// - Local retransmission of the final response for a transaction that was completed.
        ///
        MESSAGE_DISCARDED,
        /// API failed due to some error while processing
        MESSAGE_FAILED
    };

    /// Common options
    enum
    {
        OPT_REORDERHOP = 0x010,
        OPT_AUTHCANONICAL = 0x020,
        OPT_CLEN = 0x040,
        OPT_RETRANSCALLBACK = 0x080,
        OPT_PERMSGRETRANS = 0x100,
        OPT_DIRECTBUFFER = 0x200,
        OPT_PERMSGRETRANSCOUNT = 0x400
    };

    /// Encoding options
    enum
    {
        OPT_E_FULLFORM = 0x001,
        OPT_E_SHORTFORM = 0x002,
        OPT_E_COMMASEPARATED = 0x004,
        OPT_E_SINGLE = 0x008,
    };

    /// Decoding options
    enum
    {
        OPT_D_BADMESSAGE = 0x01,
        OPT_D_NOTIMER = 0x02,
        OPT_D_NOPARSEBODY = 0x04,
        OPT_D_PARTIAL = 0x08
    };

    /// Configuration options (on the basis of transaction)
    enum
    {
        /*
         * SIP_OPT_NOTIMER (TXN layer is used)
         * SIP_OPT_BADMESSAGE (Bad message is parsed)
         * SIP_OPT_NOPARSEBODY (Message body is not parsed)
         * SIP_OPT_CLEN (Invalid buffer handling)
         *
         * Use the NOTIMER option of the core stack. This will disable sip_decodeMessage from
         * directly callint fast_stopTimer. This is because we would like to do some validations
         * on the SIP message (like whether it arrived on the correct port) and then stop the
         * timer only if our validations succeed.
         */
        OPTIONS_D = (OPT_D_BADMESSAGE | OPT_D_NOPARSEBODY | OPT_D_NOTIMER | OPT_CLEN),
        OPTIONS_D_PARTIAL = (OPTIONS_D | OPT_D_PARTIAL),
        // OPT_E_FULLFORM / OPT_E_SHORTFORM
        OPTIONS_E = (OPT_CLEN | OPT_REORDERHOP)
    };

    // Max hops for the request
    static const IMS_UINT32 MAX_HOP = 70;

    // The max buckets of the transaction table storing remote retransmission elements (257 / 521)
    static const IMS_UINT32 MAX_TRANSACTION = 257;

    // Invalid CSeq number value
    static const IMS_UINT32 INVALID_SEQ_NUM = 0xFFFFFFFF;

    // The max value for an initial CSeq.
    // This is used to cross check the initial CSeq got on a fresh request.
    static const IMS_UINT32 MAX_CSEQ_VALUE = 2147483647;

private:
    // Variable for global error code of sipcore
    static IMS_SINT32* s_pnErrorCode;  // 0 means there is no error

    // SIP encoding options
    static IMS_SINT32* s_pnEncodingOptions;
};

#endif
