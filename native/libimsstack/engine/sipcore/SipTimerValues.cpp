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
#include "ServiceMemory.h"

#include "SipTimerValues.h"

PUBLIC
SipTimerValues::SipTimerValues() :
        m_nFlags(0),
        m_nT1(2000),
        m_nT2(16000),
        m_nTimerB(128000),
        m_nTimerD(128000),
        m_nTimerF(128000),
        m_nTimerH(128000),
        m_nTimerI(17000),
        m_nTimerJ(128000),
        m_nTimerK(17000)
{
}

PUBLIC
SipTimerValues::SipTimerValues(IN const SipTimerValues& other) :
        m_nFlags(other.m_nFlags),
        m_nT1(other.m_nT1),
        m_nT2(other.m_nT2),
        m_nTimerB(other.m_nTimerB),
        m_nTimerD(other.m_nTimerD),
        m_nTimerF(other.m_nTimerF),
        m_nTimerH(other.m_nTimerH),
        m_nTimerI(other.m_nTimerI),
        m_nTimerJ(other.m_nTimerJ),
        m_nTimerK(other.m_nTimerK)
{
}

PUBLIC
SipTimerValues::~SipTimerValues()
{
}

PUBLIC
SipTimerValues& SipTimerValues::operator=(IN const SipTimerValues& other)
{
    if (this != &other)
    {
        m_nFlags = other.m_nFlags;
        m_nT1 = other.m_nT1;
        m_nT2 = other.m_nT2;
        m_nTimerB = other.m_nTimerB;
        m_nTimerD = other.m_nTimerD;
        m_nTimerF = other.m_nTimerF;
        m_nTimerH = other.m_nTimerH;
        m_nTimerI = other.m_nTimerI;
        m_nTimerJ = other.m_nTimerJ;
        m_nTimerK = other.m_nTimerK;
    }

    return (*this);
}

PUBLIC
IMS_SINT32 SipTimerValues::GetValue(IN IMS_SINT32 nType) const
{
    if (!IsSet(nType))
    {
        return 0;
    }

    if (nType == TIMER_T1)
    {
        return m_nT1;
    }
    else if (nType == TIMER_T2)
    {
        return m_nT2;
    }
    else if (nType == TIMER_B)
    {
        return m_nTimerB;
    }
    else if (nType == TIMER_D)
    {
        return m_nTimerD;
    }
    else if (nType == TIMER_F)
    {
        return m_nTimerF;
    }
    else if (nType == TIMER_H)
    {
        return m_nTimerH;
    }
    else if (nType == TIMER_I)
    {
        return m_nTimerI;
    }
    else if (nType == TIMER_J)
    {
        return m_nTimerJ;
    }
    else if (nType == TIMER_K)
    {
        return m_nTimerK;
    }

    return 0;
}

PUBLIC
void SipTimerValues::SetValue(IN IMS_SINT32 nType, IN IMS_UINT32 nValue)
{
    m_nFlags |= nType;

    if (nType == TIMER_T1)
    {
        m_nT1 = nValue;
    }
    else if (nType == TIMER_T2)
    {
        m_nT2 = nValue;
    }
    else if (nType == TIMER_B)
    {
        m_nTimerB = nValue;
    }
    else if (nType == TIMER_D)
    {
        m_nTimerD = nValue;
    }
    else if (nType == TIMER_F)
    {
        m_nTimerF = nValue;
    }
    else if (nType == TIMER_H)
    {
        m_nTimerH = nValue;
    }
    else if (nType == TIMER_I)
    {
        m_nTimerI = nValue;
    }
    else if (nType == TIMER_J)
    {
        m_nTimerJ = nValue;
    }
    else if (nType == TIMER_K)
    {
        m_nTimerK = nValue;
    }
}

PUBLIC GLOBAL
SipTimerValues SipTimerValues::CreateTimerValues(IN IMS_SINT32 nT1, IN IMS_SINT32 nT2)
{
    SipTimerValues objTv;

    objTv.m_nFlags = TIMER_ALL;
    objTv.m_nT1 = nT1;
    objTv.m_nT2 = nT2;
    objTv.m_nTimerB = nT1 * 64;
    objTv.m_nTimerD = nT1 * 64;
    objTv.m_nTimerF = nT1 * 64;
    objTv.m_nTimerH = nT1 * 64;
    objTv.m_nTimerI = nT2 + 1000;
    objTv.m_nTimerJ = nT1 * 64;
    objTv.m_nTimerK = nT2 + 1000;

    return objTv;
}
