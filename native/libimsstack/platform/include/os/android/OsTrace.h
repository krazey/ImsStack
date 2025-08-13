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
#ifndef OS_TRACE_H_
#define OS_TRACE_H_

#include "AString.h"
#include "ImsTrace.h"

class IMutex;
class OsTraceNode;

class OsTrace : public ImsTrace
{
public:
    OsTrace();
    ~OsTrace() override;

    OsTrace(IN const OsTrace&) = delete;
    OsTrace& operator=(IN const OsTrace&) = delete;

public:
    void OutV(IN IMS_SINT32 nCategory, IN const IMS_CHAR* pszTag, IN IMS_UINT32 nModule,
            IN const IMS_CHAR* pszFile, IN IMS_UINT32 nLine, IN const IMS_CHAR* pszFormat,
            IN va_list args) override;

protected:
    const IMS_CHAR* GetDirName() const override;
    void OutputString(IN IMS_SINT32 nCategory, IN IMS_CHAR* pszTrace, IN IMS_UINT32 nLength,
            IN const IMS_CHAR* pszLogTag = IMS_NULL) override;

    void AddTraceNode(IN OsTraceNode* pNode);
    IMS_SINT32 GetTraceNodeCount();

    IMS_BOOL IsLogging();
    void SetLogging(IN IMS_BOOL bLogging);

public:
    IMutex* m_piMutex;
    IMS_BOOL m_bLogging;
    ImsList<OsTraceNode*> m_objTraceNodes;
};

#endif
