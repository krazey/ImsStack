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
#ifndef IMS_TRACE_H_
#define IMS_TRACE_H_

#include "ITrace.h"
#include "ITraceOption.h"

class ImsTrace : public ITrace
{
public:
    ImsTrace();
    ~ImsTrace() override = default;

    ImsTrace(IN const ImsTrace&) = delete;
    ImsTrace& operator=(IN const ImsTrace&) = delete;

public:
    inline IMS_UINT32 GetOption() const { return m_nOption; }

    void SetOption(IN IMS_UINT32 nOption, IN IMS_UINT32 nModule);
    IMS_BOOL IsTraceEnabled(IN IMS_SINT32 nCategory, IN IMS_UINT32 nModule) override;
    void OutP(IN IMS_SINT32 nCategory, IN const IMS_CHAR* pszTag, IN IMS_UINT32 nModule,
            IN const IMS_CHAR* pszFormat, ...);

    static IMS_CHAR* EncryptPrivacyLog(IN_OUT IMS_CHAR* pszPrivacy, IN const IMS_CHAR* pszArg);
    static IMS_SINT32 IsLoggable(IN IMS_SINT32 nCategory);

protected:
    virtual const IMS_CHAR* GetDirName() const = 0;
    virtual void OutputString(IN IMS_SINT32 nCategory, IN IMS_CHAR* pszTrace, IN IMS_UINT32 nLength,
            IN const IMS_CHAR* pszLogTag = IMS_NULL);

    IMS_BOOL IsModuleEnabled(IN IMS_UINT32 nModule) const;
    IMS_BOOL IsOptionEnabled(IN IMS_SINT32 nCategory) const;

private:
    void Out(IN const IMS_CHAR* pszFormat, ...) override;
    void Out(IN IMS_SINT32 nCategory, IN const IMS_CHAR* pszTag, IN IMS_UINT32 nModule,
            IN const IMS_CHAR* pszFile, IN IMS_UINT32 nLine, IN const IMS_CHAR* pszFormat,
            ...) override;
    void OutE(IN IMS_SINT32 nErrorCode, IN const IMS_CHAR* pszTag, IN IMS_UINT32 nModule,
            IN const IMS_CHAR* pszFile, IN const IMS_CHAR* pszFunc, IN IMS_UINT32 nLine,
            IN const IMS_CHAR* pszFormat, ...) override;
    //// For a large TEXT message (HTTP, MSRP, SDP, SIP, XML, ...)
    void OutText(IN IMS_UINT32 nModule, IN IMS_SINT32 nType, IN const IMS_CHAR* pszDescription,
            IN const IMS_CHAR* pszText, IN IMS_UINT32 nTextSize) override;

    static void HideArgs(
            IN const IMS_CHAR* pszFormat, OUT IMS_CHAR* pszBuffer, IN IMS_SINT32 nIgnore = 2);

private:
    enum
    {
        END_SIZE = 14,
        START_SIZE = 15,
        MAX_SUMMARY_SIZE = 512,
        MAX_TEXT_SIZE = 4096
    };

    enum
    {
        ROTATE_COUNT = 4,
        PRIVACY_MAX_SIZE = 128
    };

    static const IMS_CHAR* START[ITrace::TEXT_MAX];
    static const IMS_CHAR* END[ITrace::TEXT_MAX];
    static const IMS_CHAR* TEXT_LOG_TAG[ITrace::TEXT_MAX];

    IMS_UINT32 m_nOption;
    IMS_UINT32 m_nTracedModules;
};

#endif
