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
#ifndef IMS_TRACE_NODE_H_
#define IMS_TRACE_NODE_H_

#include <stdarg.h>

#include "AString.h"

class ImsTraceNode
{
public:
    explicit ImsTraceNode(IN const IMS_CHAR* pszTag);
    virtual ~ImsTraceNode();

    ImsTraceNode(IN const ImsTraceNode&) = delete;
    ImsTraceNode& operator=(IN const ImsTraceNode&) = delete;

public:
    inline IMS_CHAR* GetBuffer() { return m_pBuffer; }
    inline IMS_UINT32 GetLength() const { return m_nLength; }

    void Format(IN const IMS_CHAR* pszFormat, IN va_list args,
            IN const AString& strSuffix = AString::ConstNull());
    static AString GetComponentName(IN const IMS_CHAR* pszComponent);

protected:
    virtual IMS_SINT32 Vsnprintf(OUT IMS_CHAR* pszBuffer, IN IMS_UINT32 nBuffSize,
            IN const IMS_CHAR* pszFormat, IN va_list args) = 0;

private:
    static constexpr IMS_SINT32 MAX_BUFF_SIZE = 512;

    IMS_BOOL m_bAlloc;
    IMS_SINT32 m_nHeaderLength;

    IMS_SINT32 m_nLength;
    IMS_CHAR m_acBuffer[MAX_BUFF_SIZE + 1];
    IMS_CHAR* m_pBuffer;
};

#endif
