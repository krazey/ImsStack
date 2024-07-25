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
#include "SipConfiguration.h"

SipConfiguration* SipConfiguration::pSipConfig = SIP_NULL;

SipConfiguration::SipConfiguration()
{
    bPANIHeaderForACK = SIP_FALSE;

    /* Normal form, Single Line and loose parsing */
    m_nParseStyle = ~SipConfiguration::MSG_OPT_ENCODE_MULTI_LINE;
    m_nParseStyle &= ~SipConfiguration::MSG_OPT_ENCODE_SHORT_FORM;
    m_nParseStyle &= ~SipConfiguration::MSG_OPT_DECODE_STRICT;

    const SIP_UINT32 DEFAULT_T1 = 2000;
    const SIP_UINT32 DEFAULT_T2 = 16000;

    m_nT1 = DEFAULT_T1;
    m_nT2 = DEFAULT_T2;
    m_nT4 = DEFAULT_T2 + 1000;
    m_nTimerB = 64 * m_nT1;
    m_nTimerC = 180000;
    m_nTimerCr = 180000;
    m_nTimerD_T3 = 64 * m_nT1;
    m_nTimerF_T3 = 64 * m_nT1;
    m_nTimerH = 64 * m_nT1;
    m_nTimerI_T4 = m_nT4;
    m_nTimerJ_T3 = 64 * m_nT1;
    m_nTimerK_T4 = m_nT4;
}

SipConfiguration::~SipConfiguration() {}

SIP_VOID SipConfiguration::SetMultiLineEncoding(SIP_BOOL bEnableMultiLine)
{
    if (bEnableMultiLine == SIP_TRUE)
    {
        m_nParseStyle = m_nParseStyle | SipConfiguration::MSG_OPT_ENCODE_MULTI_LINE;
    }
    else
    {
        m_nParseStyle = m_nParseStyle & ~SipConfiguration::MSG_OPT_ENCODE_MULTI_LINE;
    }
}

SIP_VOID SipConfiguration::SetShortFormEncoding(SIP_BOOL bEncInShortForm)
{
    if (bEncInShortForm == SIP_TRUE)
    {
        m_nParseStyle = m_nParseStyle | SipConfiguration::MSG_OPT_ENCODE_SHORT_FORM;
    }
    else
    {
        m_nParseStyle = m_nParseStyle & ~SipConfiguration::MSG_OPT_ENCODE_SHORT_FORM;
    }
}

SIP_VOID SipConfiguration::SetDecodeStrictness(SIP_BOOL bEnableStrictDecode)
{
    if (bEnableStrictDecode == SIP_TRUE)
    {
        m_nParseStyle = m_nParseStyle | SipConfiguration::MSG_OPT_DECODE_STRICT;
    }
    else
    {
        m_nParseStyle = m_nParseStyle & ~SipConfiguration::MSG_OPT_DECODE_STRICT;
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

SIP_VOID SipConfiguration::DestroyInstance()
{
    if (pSipConfig != SIP_NULL)
    {
        delete pSipConfig;
        pSipConfig = SIP_NULL;
    }
}
