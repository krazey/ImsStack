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

#include "AStringBuffer.h"
#include "ImsStrLib.h"
#include "ImsTrace.h"
#include "ServiceMemory.h"
#include "ServiceSystemTime.h"

// SDP/SIP/XML: SP is added before first CRLF to align a size of start string
PRIVATE GLOBAL const IMS_CHAR* ImsTrace::START[ITrace::TEXT_MAX] = {
        "\r\nTEXT_TEXT_START\r\n\r\n",
        " \r\nTEXT_SDP_START\r\n\r\n",
        " \r\nTEXT_SIP_START\r\n\r\n",
        " \r\nTEXT_XML_START\r\n\r\n",
};

// SDP/SIP/XML: SP is added before last CRLF to align a size of end string
PRIVATE GLOBAL const IMS_CHAR* ImsTrace::END[ITrace::TEXT_MAX] = {
        "\r\nTEXT_TEXT_END\r\n\r\n",
        "\r\nTEXT_SDP_END\r\n \r\n",
        "\r\nTEXT_SIP_END\r\n \r\n",
        "\r\nTEXT_XML_END\r\n \r\n",
};

PUBLIC
ImsTrace::ImsTrace() :
        m_nOption(ITraceOption::OPT_DEFAULT),
        m_nTracedModules(IMS_TRACE_MODULE_ALL)
{
}

PUBLIC VIRTUAL ImsTrace::~ImsTrace() {}

PUBLIC VIRTUAL const IMS_CHAR* ImsTrace::GetFileName(IN const IMS_CHAR* /*pszFileName*/)
{
    return "__NULL__";
}

PUBLIC VIRTUAL const IMS_CHAR* ImsTrace::GetFileName(
        IN_OUT IMS_CHAR* /*pszOutFileName*/, IN const IMS_CHAR* /*pszFileName*/)
{
    return "__NULL__";
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
void ImsTrace::OutP(IN IMS_SINT32 nCategory, IN const IMS_CHAR* pszTag, IN IMS_UINT32 nModule,
        IN const IMS_CHAR* pszFormat, ...)
{
    va_list args;

    va_start(args, pszFormat);
    OutV(nCategory, pszTag, nModule, pszFormat, args);
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

PROTECTED VIRTUAL void ImsTrace::OutputString(
        IN IMS_SINT32 /*nCategory*/, IN IMS_CHAR* /*pszTrace*/, IN IMS_UINT32 /*nLength*/)
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
#if 0
    // Check the trace category
    if ((m_nOption & ITraceOption::OPT_CAT_ALL) == ITraceOption::OPT_CAT_NONE)
    {
        // All Trace Disabled
        return IMS_FALSE;
    }
#endif

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

    // Trace Disabed...
    return IMS_FALSE;
}

PRIVATE VIRTUAL void ImsTrace::Out(IN const IMS_CHAR* pszFormat, ...)
{
    va_list args;

    va_start(args, pszFormat);

    if ((m_nOption & ITraceOption::OPT_HIDE_PRIVACY) != 0)
    {
        IMS_CHAR acBuffer[MAX_TEXT_SIZE + 1];
        HideArgs(pszFormat, acBuffer);
        OutV(ITrace::CAT_D, __IMS_TRACE_DEFAULT_NAME__, IMS_TRACE_MODULE_DEFAULT, acBuffer, args);
    }
    else
    {
        OutV(ITrace::CAT_D, __IMS_TRACE_DEFAULT_NAME__, IMS_TRACE_MODULE_DEFAULT, pszFormat, args);
    }

    va_end(args);
}

PRIVATE VIRTUAL void ImsTrace::Out(IN IMS_SINT32 nCategory, IN const IMS_CHAR* pszTag,
        IN IMS_UINT32 nModule, IN const IMS_CHAR* pszFormat, ...)
{
    va_list args;

    va_start(args, pszFormat);

    if ((m_nOption & ITraceOption::OPT_HIDE_PRIVACY) != 0)
    {
        IMS_CHAR acBuffer[MAX_TEXT_SIZE + 1];
        HideArgs(pszFormat, acBuffer);
        OutV(nCategory, pszTag, nModule, acBuffer, args);
    }
    else
    {
        OutV(nCategory, pszTag, nModule, pszFormat, args);
    }

    va_end(args);
}

PRIVATE VIRTUAL void ImsTrace::OutE(IN IMS_SINT32 nErrorCode, IN const IMS_CHAR* pszFunction,
        IN IMS_UINT16 nLine, IN const IMS_CHAR* pszTag, IN IMS_UINT32 nModule,
        IN const IMS_CHAR* pszFormat, ...)
{
#if 0
    if ((m_nOption & ITraceOption::OPT_CAT_E) != ITraceOption::OPT_CAT_E)
    {
        // Do nothing ...
        return;
    }
#endif

    va_list args;

    va_start(args, pszFormat);
    OutV(ITrace::CAT_E, pszTag, nModule, pszFormat, args);
    va_end(args);

    Out(ITrace::CAT_E, pszTag, nModule, "E_CODE (%d) AT (%s, %d)", nErrorCode, pszFunction, nLine);
}

PRIVATE VIRTUAL void ImsTrace::OutText(IN IMS_UINT32 nModule, IN IMS_SINT32 nType,
        IN const IMS_CHAR* pszDescription, IN const IMS_CHAR* pszText, IN IMS_UINT32 nTextSize,
        IN IMS_BOOL bBinaryBody /*= IMS_FALSE*/)
{
    if ((m_nOption & ITraceOption::OPT_CAT_TEXT) != ITraceOption::OPT_CAT_TEXT)
    {
        return;
    }

    if (!IsModuleEnabled(nModule))
    {
        return;
    }

    IMS_CHAR acBuffer[MAX_TEXT_SIZE + 1];
    IMS_SINT32 nLength = 0;
    IMS_UINT32 nTextEnd = nTextSize;

    if (nType == ITrace::TEXT_SIP)
    {
        if (bBinaryBody)
        {
            // Find a CRLFCRLF
            IMS_CHAR* pszTmp = IMS_StrStr(pszText, "\r\n\r\n");

            if (pszTmp != IMS_NULL)
            {
                nTextEnd = pszTmp - pszText + 4 /*CRLF CRLF*/;
            }
        }

        // Display a time string
        AString strTime = IMS_SYS_GetTimeString();

        nLength = IMS_Sprintf(acBuffer, MAX_TEXT_SIZE,
                "%s"
                "TIME: %s\r\n",
                START[nType], strTime.GetStr());
    }
    else
    {
        IMS_MEM_Memcpy(&acBuffer[nLength], START[nType], START_SIZE);
        nLength = START_SIZE;
    }

    if ((pszDescription != IMS_NULL) && (nLength != -1))
    {
        nLength += IMS_Sprintf(
                &acBuffer[nLength], MAX_TEXT_SIZE - nLength, "%s\r\n\r\n", pszDescription);
    }

    if (nTextEnd <= (MAX_TEXT_SIZE - MAX_SPARE_SIZE))
    {
        IMS_MEM_Memcpy(&acBuffer[nLength], pszText, nTextEnd);
        nLength += nTextEnd;
        nLength += IMS_Sprintf(&acBuffer[nLength], MAX_TEXT_SIZE - nLength, "%s", END[nType]);

        OutputString(ITrace::CAT_I, acBuffer, nLength);
    }
    else
    {
        // Display Start & Info.
        OutputString(ITrace::CAT_I, acBuffer, nLength);

        // Display a protocol message
        IMS_UINT32 nTotalLength = nTextEnd;
        const IMS_CHAR* pszTextStart = pszText;

        if (nTotalLength > MAX_TEXT_SIZE)
        {
            nLength = MAX_TEXT_SIZE;
        }
        else
        {
            nLength = nTotalLength;
        }

        while (nTotalLength > 0)
        {
            IMS_MEM_Memcpy(acBuffer, pszTextStart, nLength);

            pszTextStart += nLength;
            nTotalLength -= nLength;

            acBuffer[nLength] = '\0';

            OutputString(ITrace::CAT_I, acBuffer, nLength);

            if (nTotalLength > MAX_TEXT_SIZE)
            {
                nLength = MAX_TEXT_SIZE;
            }
            else
            {
                nLength = nTotalLength;
            }
        }

        // Display END
        IMS_MEM_Memcpy(acBuffer, END[nType], END_SIZE);
        acBuffer[END_SIZE] = '\0';
        OutputString(ITrace::CAT_I, acBuffer, END_SIZE);
    }
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
