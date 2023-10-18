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

#include "SipMethod.h"

PUBLIC GLOBAL const IMS_CHAR* SipMethod::NAME[] = {
        "ACK",
        "BYE",
        "CANCEL",
        "INVITE",
        "OPTIONS",
        "REGISTER",
        "PRACK",
        "SUBSCRIBE",
        "NOTIFY",
        "UPDATE",
        "MESSAGE",
        "REFER",
        "PUBLISH",
        "INFO",
        "UNKNOWN",
};

PUBLIC GLOBAL const SipMethod SipMethod::INVALID_METHOD;

PUBLIC
SipMethod::SipMethod(IN const IMS_SINT32 nMethod /* = SipMethod::INVALID*/) :
        m_nMethod(nMethod),
        m_strMethod(ConvertMethodToString(nMethod))
{
}

PUBLIC
SipMethod::SipMethod(IN const IMS_CHAR* pszMethod) :
        m_nMethod(INVALID),
        m_strMethod(pszMethod)
{
    m_nMethod = ConvertStringToMethod(m_strMethod);
}

PUBLIC
SipMethod::SipMethod(IN const AString& strMethod) :
        m_nMethod(ConvertStringToMethod(strMethod)),
        m_strMethod(strMethod)
{
}

PUBLIC
SipMethod::SipMethod(IN const SipMethod& other) :
        m_nMethod(other.m_nMethod),
        m_strMethod(other.m_strMethod)
{
}

PUBLIC
SipMethod::~SipMethod() {}

PUBLIC
SipMethod& SipMethod::operator=(IN const SipMethod& other)
{
    if (this != &other)
    {
        m_nMethod = other.m_nMethod;
        m_strMethod = other.m_strMethod;
    }

    return (*this);
}

PUBLIC
SipMethod& SipMethod::operator=(IN IMS_SINT32 nMethod)
{
    if (m_nMethod == nMethod)
    {
        return (*this);
    }

    m_nMethod = nMethod;
    m_strMethod = ConvertMethodToString(m_nMethod);

    return (*this);
}

PUBLIC
SipMethod& SipMethod::operator=(IN const IMS_CHAR* pszMethod)
{
    if (m_strMethod.Equals(pszMethod))
    {
        return (*this);
    }

    m_strMethod = pszMethod;
    m_nMethod = ConvertStringToMethod(m_strMethod);

    return (*this);
}

PUBLIC
SipMethod& SipMethod::operator=(IN const AString& strMethod)
{
    if (m_strMethod.Equals(strMethod))
    {
        return (*this);
    }

    m_strMethod = strMethod;
    m_nMethod = ConvertStringToMethod(m_strMethod);

    return (*this);
}

PUBLIC GLOBAL const IMS_CHAR* SipMethod::ToName(IN IMS_SINT32 nMethod)
{
    return ((nMethod > INVALID) && (nMethod < MAX)) ? NAME[nMethod] : "";
}

PRIVATE GLOBAL IMS_SINT32 SipMethod::ConvertStringToMethod(IN const AString& strMethod)
{
    if (strMethod.IsNULL() || strMethod.IsEmpty())
    {
        return SipMethod::INVALID;
    }
    else
    {
        for (IMS_SINT32 i = (SipMethod::INVALID + 1); i < SipMethod::MAX; i++)
        {
            if (strMethod.Equals(SipMethod::NAME[i]))
            {
                return i;
            }
        }
    }

    return SipMethod::INVALID;
}

PRIVATE GLOBAL AString SipMethod::ConvertMethodToString(IN IMS_SINT32 nMethod)
{
    if ((nMethod > SipMethod::INVALID) && (nMethod < SipMethod::MAX))
    {
        return AString(NAME[nMethod]);
    }

    return AString::ConstNull();
}
