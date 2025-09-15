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
#ifndef CHARACTER_DATA_H_
#define CHARACTER_DATA_H_

#include "Node.h"

class CharacterData : public Node
{
public:
    CharacterData();
    explicit CharacterData(IN xmlNodePtr pstNode);
    ~CharacterData() override;

    CharacterData(IN const CharacterData&) = delete;
    CharacterData& operator=(IN const CharacterData&) = delete;

    // CharacterData
    virtual void AppendData(IN const AString& strData);
    virtual void DeleteData(IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount);
    virtual const AString& GetData() const;
    virtual IMS_SINT32 GetLength() const;
    virtual void InsertData(IN IMS_SINT32 nOffset, IN const AString& strData);
    virtual void ReplaceData(
            IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount, IN const AString& strData);
    virtual void SetData(IN const AString& strData);
    virtual AString SubstringData(IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount);

protected:
    AString m_strCharacterData;
};

#endif
