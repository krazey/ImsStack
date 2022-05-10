#ifndef __SIP_CONFIGURATION_H__
#define __SIP_CONFIGURATION_H__

#include "sip_pf_datatypes.h"

typedef enum _SipEn_MsgOption
{
    ESIPMSGOPT_NONE = 0,
    ESIPMSGOPT_ENCMULTILINE = (1 << 0),
    ESIPMSGOPT_ENCSHORTFORM = (1 << 1),
    ESIPMSGOPT_DECSTRICT = (1 << 2),
    ESIPMSGOPT_END,
    ESIPMSGOPT_INVALID = SIP_INVALID
} SipEn_MsgOption;


class SipConfiguration
{
public:
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

    static SipConfiguration* GetInstance();

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
};

#endif  //__SIP_CONFIGURATION_H__
