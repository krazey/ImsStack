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
#ifndef __SIP_CONFIGURATION_H__
#define __SIP_CONFIGURATION_H__

#include "SipDatatypes.h"

class SipConfiguration
{
public:
    enum
    {
        MSG_OPT_ENCODE_NONE = 0,
        MSG_OPT_ENCODE_MULTI_LINE = (1 << 0),
        MSG_OPT_ENCODE_SHORT_FORM = (1 << 1),
        MSG_OPT_DECODE_STRICT = (1 << 2)
    };

    SipConfiguration();
    virtual ~SipConfiguration();

    inline SIP_VOID EnablePANIHeaderForACK(SIP_BOOL bPANIHeader)
    {
        bPANIHeaderForACK = bPANIHeader;
    }
    inline SIP_BOOL IsPANIHeaderReqdForACK() const { return bPANIHeaderForACK; }

    /* TODO: Need to rething how to provide parsing option for easy operation */
    SIP_VOID SetMultiLineEncoding(SIP_BOOL bEnableMultiLine);
    SIP_VOID SetShortFormEncoding(SIP_BOOL bEncInShortForm);
    SIP_VOID SetDecodeStrictness(SIP_BOOL bEnableStrictDecode);

    /* Timer Values A-K */

    /* T1: RTT Estimate */
    inline SIP_VOID SetT1(SIP_UINT32 nDur) { m_nT1 = nDur; }
    inline SIP_UINT32 GetT1() const { return m_nT1; }

    /* The Max Retransmit interval for non-INVITE Req and INVITE Resp */
    inline SIP_VOID SetT2(SIP_UINT32 nDur) { m_nT2 = nDur; }
    inline SIP_UINT32 GetT2() const { return m_nT2; }

    /* Maximum duration a message will remain in the network */
    inline SIP_VOID SetT4(SIP_UINT32 nDur) { m_nT4 = nDur; }
    inline SIP_UINT32 GetT4() const { return m_nT4; }

    inline SIP_UINT32 GetStackSettings() const { return m_nParseStyle; }

    /*TimerB  INVITE transaction timeout timer*/
    inline SIP_VOID SetTimerB(SIP_UINT32 nDur) { m_nTimerB = nDur; }
    inline SIP_UINT32 GetTimerB() const { return m_nTimerB; }

    /*timer C proxy INVITE transaction timeout*/
    inline SIP_VOID SetTimerC(SIP_UINT32 nDur) { m_nTimerC = nDur; }
    inline SIP_UINT32 GetTimerC() const { return m_nTimerC; }

    inline SIP_VOID SetTimerCr(SIP_UINT32 nDur) { m_nTimerCr = nDur; }
    inline SIP_UINT32 GetTimerCr() const { return m_nTimerCr; }

    /*Timer D Wait time for response retransmits*/
    inline SIP_VOID SetTimerD(SIP_UINT32 nDur) { m_nTimerD_T3 = nDur; }
    inline SIP_UINT32 GetTimerD() const { return m_nTimerD_T3; }

    /*Timer F non-INVITE transaction timeout timer*/
    inline SIP_VOID SetTimerF(SIP_UINT32 nDur) { m_nTimerF_T3 = nDur; }
    inline SIP_UINT32 GetTimerF() const { return m_nTimerF_T3; }

    /*Timer H Wait time for ACK receipt*/
    inline SIP_VOID SetTimerH(SIP_UINT32 nDur) { m_nTimerH = nDur; }
    inline SIP_UINT32 GetTimerH() const { return m_nTimerH; }

    /*timer I Wait time for ACK retransmits*/
    inline SIP_VOID SetTimerI(SIP_UINT32 nDur) { m_nTimerI_T4 = nDur; }
    inline SIP_UINT32 GetTimerI() const { return m_nTimerI_T4; }

    /*Timer J Wait time for non-INVITE request retransmits*/
    inline SIP_VOID SetTimerJ(SIP_UINT32 nDur) { m_nTimerJ_T3 = nDur; }
    inline SIP_UINT32 GetTimerJ() const { return m_nTimerJ_T3; }

    /*Timer K Wait time for response retransmits*/
    SIP_VOID SetTimerK(SIP_UINT32 nDur) { m_nTimerK_T4 = nDur; }
    inline SIP_UINT32 GetTimerK() const { return m_nTimerK_T4; }

    /*Timer L Wait time for absorbing INVITE retransmits and ACK receipt */
    SIP_VOID SetTimerL(SIP_UINT32 nDur) { m_nTimerL = nDur; }
    inline SIP_UINT32 GetTimerL() const { return m_nTimerL; }

    /*Timer M Wait time for retransmission of 2xx or forked 2xx */
    SIP_VOID SetTimerM(SIP_UINT32 nDur) { m_nTimerM = nDur; }
    inline SIP_UINT32 GetTimerM() const { return m_nTimerM; }

    static SipConfiguration* GetInstance();

    // Only for testing
    static SIP_VOID DestroyInstance();

private:
    static SipConfiguration* pSipConfig;

    SIP_BOOL bPANIHeaderForACK;
    SIP_UINT32 m_nParseStyle;

    /*Timer T1 RTT Estimate*/
    SIP_UINT32 m_nT1;
    /*Timer T2 The maximum retransmit interval for non-INVITE requests and INVITE responses*/
    SIP_UINT32 m_nT2;
    /* Maximum duration a message will remain in the network */
    SIP_UINT32 m_nT4;
    /*TimerB  INVITE transaction timeout timer*/
    SIP_UINT32 m_nTimerB;
    /*timer C proxy INVITE transaction timeout*/
    SIP_UINT32 m_nTimerC;
    SIP_UINT32 m_nTimerCr;
    /*Timer D Wait time for response retransmits*/
    SIP_UINT32 m_nTimerD_T3;
    /*Timer F non-INVITE transaction timeout timer*/
    SIP_UINT32 m_nTimerF_T3;
    /*Timer H Wait time for ACK receipt*/
    SIP_UINT32 m_nTimerH;
    /*timer I Wait time for ACK retransmits*/
    SIP_UINT32 m_nTimerI_T4;
    /*Timer J Wait time for non-INVITE request retransmits*/
    SIP_UINT32 m_nTimerJ_T3;
    /*Timer K Wait time for response retransmits*/
    SIP_UINT32 m_nTimerK_T4;
    /*Timer L Wait time for absorbing INVITE retransmits and ACK receipt*/
    SIP_UINT32 m_nTimerL;
    /*Timer M Wait time for retransmission of 2xx or forked 2xx*/
    SIP_UINT32 m_nTimerM;
};

#endif  //__SIP_CONFIGURATION_H__
