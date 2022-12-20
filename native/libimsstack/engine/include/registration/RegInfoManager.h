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
#ifndef REG_INFO_MANAGER_H_
#define REG_INFO_MANAGER_H_

#include "AString.h"
#include "ImsMap.h"

#include "IRegInfoParserListener.h"
#include "RegKey.h"

class IDocument;
class IMutex;
class RegInfo;
class RegInfoParser;

class RegInfoManager : public IRegInfoParserListener
{
private:
    RegInfoManager();
    virtual ~RegInfoManager();

public:
    RegInfoManager(IN const RegInfoManager& other) = delete;
    RegInfoManager& operator=(IN const RegInfoManager& other) = delete;

public:
    IMS_BOOL CreateRegInfo(IN const RegKey& objRegKey);
    void DestroyRegInfo(IN const RegKey& objRegKey);
    RegInfo* GetRegInfo(IN const RegKey& objRegKey);
    const RegInfo* GetRegInfo(IN const RegKey& objRegKey) const;
    IMS_BOOL Initialize();
    IMS_BOOL Update(IN const RegKey& objRegKey, IN const AString& strRegInfo);

    // Debugging ...
    void DisplayRegInfo();

    static RegInfoManager* GetInstance();

private:
    // IRegInfoParserListener interface
    void RegInfoParser_ParsingCompleted(
            IN RegInfoParser* pParser, IN IDocument* piDocument) override;
    void RegInfoParser_ParsingFailed(IN RegInfoParser* pParser) override;

    IMS_BOOL AddRegInfoParser(IN RegInfoParser* pParser);
    void RemoveRegInfoParser(IN RegInfoParser*& pParser);

private:
    IMutex* m_piLock;

    // List of "reginfo" parser
    IMSList<RegInfoParser*> m_objParsers;
    // < RegKey , RegInfo* >
    IMSMap<RegKey, RegInfo*> m_objRegInfos;
};

#endif
