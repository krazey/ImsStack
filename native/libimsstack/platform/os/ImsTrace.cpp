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
#include <string.h>
#include <cutils/log.h>

#include "ISystemProperty.h"
#include "ITraceOption.h"
#include "ImsStrLib.h"
#include "ImsTrace.h"
#include "ServiceMemory.h"
#include "ServiceUtil.h"

PRIVATE GLOBAL const IMS_CHAR* ImsTrace::START[ITrace::TEXT_MAX] = {
        "TEXT_ANY_START\n",
        "TEXT_SDP_START\n",
        "TEXT_SIP_START\n",
        "TEXT_XML_START\n",
};

PRIVATE GLOBAL const IMS_CHAR* ImsTrace::END[ITrace::TEXT_MAX] = {
        "TEXT_ANY_END\n\n",
        "TEXT_SDP_END\n\n",
        "TEXT_SIP_END\n\n",
        "TEXT_XML_END\n\n",
};

PRIVATE GLOBAL const IMS_CHAR* ImsTrace::TEXT_LOG_TAG[ITrace::TEXT_MAX] = {
        IMS_LOG_TAG "-TXT",
        IMS_LOG_TAG "-SDP",
        IMS_LOG_TAG "-SIP",
        IMS_LOG_TAG "-XML",
};

PUBLIC
ImsTrace::ImsTrace() :
        m_nOption(ITraceOption::OPT_DEFAULT),
        m_nTracedModules(IMS_TRACE_MODULE_ALL)
{
    if (IMS_UTIL_SYS_PROP_IS_USER_MODE())
    {
        m_nOption = ITraceOption::RELEASE_OPT_DEFAULT;
    }
}

PUBLIC
void ImsTrace::SetOption(IN IMS_UINT32 nOption, IN IMS_UINT32 nModule)
{
    m_nOption = nOption;
    m_nTracedModules = nModule;
}

PUBLIC VIRTUAL IMS_BOOL ImsTrace::IsTraceEnabled(IN IMS_SINT32 nCategory, IN IMS_UINT32 nModule)
{
    // Check the trace category
    if ((m_nOption & ITraceOption::OPT_CAT_ALL) == ITraceOption::OPT_CAT_NONE)
    {
        // All Trace Disabled
        return IMS_FALSE;
    }

    if (IsOptionEnabled(nCategory))
    {
        if (nCategory == ITrace::CAT_E)
        {
            return IMS_TRUE;
        }

        if (IsModuleEnabled(nModule))
        {
            return IMS_TRUE;
        }
    }

    // Trace disabled
    return IMS_FALSE;
}

PUBLIC
// NOLINTNEXTLINE(cert-dcl50-cpp)
void ImsTrace::OutP(IN IMS_SINT32 nCategory, IN const IMS_CHAR* pszTag, IN IMS_UINT32 nModule,
        IN const IMS_CHAR* pszFormat, ...)
{
    va_list args;

    va_start(args, pszFormat);
    OutV(nCategory, pszTag, nModule, IMS_NULL, 0, pszFormat, args);
    va_end(args);
}

PUBLIC
IMS_CHAR* ImsTrace::EncryptPrivacyLog(IN_OUT IMS_CHAR* pszPrivacy, IN const IMS_CHAR* pszArg)
{
    IMS_UINT32 nLength = IMS_StrLen(pszArg);

    if (nLength == 0)
    {
        return pszPrivacy;
    }

    if (nLength > PRIVACY_MAX_SIZE)
    {
        HideArgs("%s", pszPrivacy, 0);
        return pszPrivacy;
    }

    HideArgs("%s", pszPrivacy, 0);

    return pszPrivacy;
}

PUBLIC GLOBAL IMS_SINT32 ImsTrace::IsLoggable(IN IMS_SINT32 nCategory)
{
    IMS_SINT32 nPriority = ANDROID_LOG_VERBOSE;

    if (nCategory == ITrace::CAT_D)
    {
        nPriority = ANDROID_LOG_DEBUG;
    }
    else if (nCategory == ITrace::CAT_I)
    {
        nPriority = ANDROID_LOG_INFO;
    }
    else if (nCategory == ITrace::CAT_E)
    {
        nPriority = ANDROID_LOG_ERROR;
    }

    return __android_log_is_loggable(nPriority, IMS_LOG_TAG, ANDROID_LOG_INFO);
}

PROTECTED VIRTUAL void ImsTrace::OutputString(IN IMS_SINT32 /*nCategory*/,
        IN IMS_CHAR* /*pszTrace*/, IN IMS_UINT32 /*nLength*/,
        IN const IMS_CHAR* /*pszLogTag = IMS_NULL*/)
{
}

PROTECTED
IMS_BOOL ImsTrace::IsModuleEnabled(IN IMS_UINT32 nModule) const
{
    // Check the trace filter
    if ((m_nTracedModules & nModule) != 0)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
IMS_BOOL ImsTrace::IsOptionEnabled(IN IMS_SINT32 nCategory) const
{
    switch (nCategory)
    {
        case ITrace::CAT_D:
            if ((m_nOption & ITraceOption::OPT_CAT_D) == ITraceOption::OPT_CAT_D)
            {
                return IMS_TRUE;
            }
            break;
        case ITrace::CAT_I:
            if ((m_nOption & ITraceOption::OPT_CAT_I) == ITraceOption::OPT_CAT_I)
            {
                return IMS_TRUE;
            }
            break;
        case ITrace::CAT_E:
            if ((m_nOption & ITraceOption::OPT_CAT_E) == ITraceOption::OPT_CAT_E)
            {
                return IMS_TRUE;
            }
            break;
        default:
            break;
    }

    // Trace disabled
    return IMS_FALSE;
}

// NOLINTNEXTLINE(cert-dcl50-cpp)
PRIVATE VIRTUAL void ImsTrace::Out(IN const IMS_CHAR* pszFormat, ...)
{
    va_list args;

    va_start(args, pszFormat);

    if ((m_nOption & ITraceOption::OPT_HIDE_PRIVACY) != 0)
    {
        IMS_CHAR acBuffer[MAX_TEXT_SIZE + 1];
        HideArgs(pszFormat, acBuffer);
        OutV(ITrace::CAT_D, __IMS_TRACE_DEFAULT_NAME__, IMS_TRACE_MODULE_DEFAULT, IMS_NULL, 0,
                acBuffer, args);
    }
    else
    {
        OutV(ITrace::CAT_D, __IMS_TRACE_DEFAULT_NAME__, IMS_TRACE_MODULE_DEFAULT, IMS_NULL, 0,
                pszFormat, args);
    }

    va_end(args);
}

// NOLINTNEXTLINE(cert-dcl50-cpp)
PRIVATE VIRTUAL void ImsTrace::Out(IN IMS_SINT32 nCategory, IN const IMS_CHAR* pszTag,
        IN IMS_UINT32 nModule, IN const IMS_CHAR* pszFile, IN IMS_UINT32 nLine,
        IN const IMS_CHAR* pszFormat, ...)
{
    va_list args;

    va_start(args, pszFormat);

    if ((m_nOption & ITraceOption::OPT_HIDE_PRIVACY) != 0)
    {
        IMS_CHAR acBuffer[MAX_TEXT_SIZE + 1];
        HideArgs(pszFormat, acBuffer);
        OutV(nCategory, pszTag, nModule, pszFile, nLine, acBuffer, args);
    }
    else
    {
        OutV(nCategory, pszTag, nModule, pszFile, nLine, pszFormat, args);
    }

    va_end(args);
}

// NOLINTNEXTLINE(cert-dcl50-cpp)
PRIVATE VIRTUAL void ImsTrace::OutE(IN IMS_SINT32 nErrorCode, IN const IMS_CHAR* pszTag,
        IN IMS_UINT32 nModule, IN const IMS_CHAR* pszFile, IN const IMS_CHAR* pszFunc,
        IN IMS_UINT32 nLine, IN const IMS_CHAR* pszFormat, ...)
{
    va_list args;

    va_start(args, pszFormat);
    OutV(ITrace::CAT_E, pszTag, nModule, pszFile, nLine, pszFormat, args);
    va_end(args);

    Out(ITrace::CAT_E, pszTag, nModule, pszFile, nLine, "E_CODE(%d) AT (%s)", nErrorCode, pszFunc);
}

PRIVATE VIRTUAL void ImsTrace::OutText(IN IMS_UINT32 nModule, IN IMS_SINT32 nType,
        IN const IMS_CHAR* pszDescription, IN const IMS_CHAR* pszText, IN IMS_UINT32 nTextSize)
{
    if ((m_nOption & ITraceOption::OPT_CAT_TEXT) != ITraceOption::OPT_CAT_TEXT)
    {
        return;
    }

    if (!IsModuleEnabled(nModule))
    {
        return;
    }

    const IMS_CHAR* pszLogTag = TEXT_LOG_TAG[nType];
    IMS_CHAR acBuffer[MAX_SUMMARY_SIZE + 1];
    IMS_SINT32 nLength = 0;

    IMS_MEM_Memcpy(&acBuffer[nLength], START[nType], START_SIZE);
    nLength = START_SIZE;

    if (pszDescription != IMS_NULL)
    {
        nLength += IMS_Sprintf(
                &acBuffer[nLength], MAX_SUMMARY_SIZE - nLength, "%s\n\n", pszDescription);
    }

    // Display Start & Info.
    OutputString(ITrace::CAT_I, acBuffer, nLength, pszLogTag);

    IMS_SINT32 nTotalLength = static_cast<IMS_SINT32>(nTextSize);
    IMS_CHAR* pszTextStart = const_cast<IMS_CHAR*>(pszText);

    if (nType == ITrace::TEXT_SIP)
    {
        // Find double CRLF - end of SIP header fields.
        IMS_CHAR* pszBodyStart = IMS_StrStr(pszTextStart, "\r\n\r\n");
        // Skip double CRLF if present
        pszBodyStart = (pszBodyStart != IMS_NULL) ? pszBodyStart + 4 : IMS_NULL;

        // NOTE: The SIP core can control the text to be printed in the logging message
        // based on a specified length, so need to consider the length of the message to be printed.
        if (pszBodyStart != IMS_NULL && ((pszBodyStart - pszTextStart) <= nTotalLength))
        {
            IMS_CHAR* pszHeaderStart = pszTextStart;

            while (IMS_TRUE)
            {
                IMS_CHAR* pszLineEnd = IMS_StrChr(pszHeaderStart, '\n');

                OutputString(ITrace::CAT_I, pszHeaderStart, pszLineEnd - pszHeaderStart, pszLogTag);

                pszHeaderStart = pszLineEnd + 1;  // Skip LF

                if (pszHeaderStart == pszBodyStart)
                {
                    // End of SIP header fields.
                    break;
                }
            }

            nTotalLength -= (pszBodyStart - pszTextStart);
            pszTextStart = pszBodyStart;
        }
    }

    // Print other text messages or SIP message body parts here.
    if (nTotalLength > 0)
    {
        OutputString(ITrace::CAT_I, pszTextStart, nTotalLength, pszLogTag);
    }

    // Display END
    IMS_MEM_Memcpy(acBuffer, END[nType], END_SIZE);
    acBuffer[END_SIZE] = '\0';
    OutputString(ITrace::CAT_I, acBuffer, END_SIZE, pszLogTag);
}

PRIVATE
void ImsTrace::HideArgs(
        IN const IMS_CHAR* pszFormat, OUT IMS_CHAR* pszBuffer, IN IMS_SINT32 nIgnore /* = 2 */)
{
    const IMS_CHAR SECRET = '*';
    const IMS_CHAR SKIP_CHAR[] = "lL-+ #0123456789*.";

    // nIgnore = 2 (Show filename and line when option is enabled)
    IMS_CHAR* pszCursor = pszBuffer;

    // Replace format str(%...) to secret char
    while (*pszFormat != '\0')
    {
        if (*pszFormat == '%' && nIgnore-- <= 0)
        {
            pszFormat++;

            if (*pszFormat != '%')
            {
                pszFormat += strspn(pszFormat, SKIP_CHAR);
                *pszCursor++ = SECRET;
            }
            else
            {
                // Don't replace escaped %
                *pszCursor++ = '%';
                *pszCursor++ = '%';
            }
            pszFormat++;
        }
        else
        {
            *pszCursor++ = *pszFormat++;
        }
    }
    *pszCursor = '\0';
}
