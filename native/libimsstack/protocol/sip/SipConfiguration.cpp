#include "sip_pf_datatypes.h"
#include "platform/sip_pf_memory.h"
#include "platform/sip_pf_string.h"

#include "SipTrace.h"
#include "sip_error.h"

#include "SipConfiguration.h"

// 3GPP-based timer intervals
#define DEFAULT_T1      2000
#define DEFAULT_T2      16000

SipConfiguration* SipConfiguration::pSipConfig = SIP_NULL;

SipConfiguration::SipConfiguration()
{
    bPANIHeaderForACK = SIP_FALSE;

    /* Normal form, Single Line and loose parsing */
    m_nParseStyle = ~ESIPMSGOPT_ENCMULTILINE;
    m_nParseStyle &= ~ESIPMSGOPT_ENCSHORTFORM;
    m_nParseStyle &= ~ESIPMSGOPT_DECSTRICT;

    m_nT1                  = DEFAULT_T1;
    m_nT2                  = DEFAULT_T2;
    m_nT4                  = DEFAULT_T2 + 1000;
    m_nTimerB              = 64*m_nT1;
    m_nTimerC              = 180000;
    m_nTimerCr             = 180000;
    m_nTimerD_T3           = 64*m_nT1;
    m_nTimerF_T3           = 64*m_nT1;
    m_nTimerH              = 64*m_nT1;
    m_nTimerI_T4           = m_nT4;
    m_nTimerJ_T3           = 64*m_nT1;
    m_nTimerK_T4           = m_nT4;
}

SipConfiguration::~SipConfiguration()
{
}

SIP_VOID SipConfiguration::SetMultiLineEncoding(SIP_BOOL bEnableMultiLine)
{
    if (bEnableMultiLine == SIP_TRUE)
    {
        m_nParseStyle = m_nParseStyle | ESIPMSGOPT_ENCMULTILINE;
    }
    else
    {
        m_nParseStyle = m_nParseStyle & ESIPMSGOPT_ENCMULTILINE;
    }
}

SIP_VOID SipConfiguration::SetShortFormEncoding(SIP_BOOL bEncInShortForm)
{
    if (bEncInShortForm == SIP_TRUE)
    {
        m_nParseStyle = m_nParseStyle | ESIPMSGOPT_ENCSHORTFORM;
    }
    else
    {
        m_nParseStyle = m_nParseStyle & ESIPMSGOPT_ENCSHORTFORM;
    }
}

SIP_VOID SipConfiguration::SetDecodeStrictness(SIP_BOOL bEnableStrictDecode)
{
    if (bEnableStrictDecode == SIP_TRUE)
    {
        m_nParseStyle = m_nParseStyle & ESIPMSGOPT_DECSTRICT;
    }
    else
    {
        m_nParseStyle = m_nParseStyle & ~ESIPMSGOPT_DECSTRICT;
    }
}

SipConfiguration* SipConfiguration::GetInstance()
{
    if (pSipConfig == SIP_NULL)
    {
        pSipConfig = new SipConfiguration();
    }

    return pSipConfig;
}
