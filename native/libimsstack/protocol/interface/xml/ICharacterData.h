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
#ifndef INTERFACE_CHARACTER_DATA_H_
#define INTERFACE_CHARACTER_DATA_H_

#include "INode.h"

/**
 * @brief This class extends INode with a set of attributes and methods
 *        for accessing character data in the DOM.
 *
 * For clarity this set is defined here rather than on each object that uses these attributes
 * and methods.
 * No DOM objects correspond directly to CharacterData, though Text and others do inherit
 * the interface from it. All offsets in this interface start from 0.
 *
 * @see INode
 */
class ICharacterData : public INode
{
protected:
    ~ICharacterData() override = default;

public:
    /**
     * @brief Appends the string to the end of the character data of the node.
     *
     * @param strData The string to append.
     */
    virtual void AppendData(IN const AString& strData) = 0;

    /**
     * @brief Removes a range of 16-bit units from the node.
     *
     * @param nOffset The offset from which to start removing
     * @param nCount The number of 16-bit units to delete\n
     *               If the sum of offset and count exceeds length
     *               then all 16-bit units from offset to the end of the data are deleted.
     */
    virtual void DeleteData(IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount) = 0;

    /**
     * @brief Returns the character data of this node.
     *
     * @return The character data as string.
     */
    virtual const AString& GetData() const = 0;

    /**
     * @brief Returns the number of 16-bit units that are available in this node.
     *
     * @return The number of 16-bit units available in this node.
     */
    virtual IMS_SINT32 GetLength() const = 0;

    /**
     * @brief Inserts a string at the specified 16-bit unit offset.
     *
     * @param nOffset The character offset at which to insert
     * @param strData The string to insert
     */
    virtual void InsertData(IN IMS_SINT32 nOffset, IN const AString& strData) = 0;

    /**
     * @brief Replaces the characters starting at the specified 16-bit unit offset
     *        with the specified string.
     *
     * @param nOffset The offset from which to start replacing
     * @param nCount The number of 16-bit units to replace\n
     *               If the sum of offset and count exceeds length,
     *               then all 16-bit units to the end of the data are replaced.
     * @param strData The string with which the range must be replaced
     */
    virtual void ReplaceData(
            IN IMS_SINT32 nOffSet, IN IMS_SINT32 nCount, IN const AString& strData) = 0;

    /**
     * @brief Sets the character data of this node.
     *
     * @param strData The character data to set to the node
     */
    virtual void SetData(IN const AString& strData) = 0;

    /**
     * @brief Extracts a range of data from the node.
     *
     * @param nOffset The start offset of substring to extract
     * @param nCount The number of 16-bit units to extract
     * @return The substring of this character data.
     */
    virtual AString SubstringData(IN IMS_SINT32 nOffSet, IN IMS_SINT32 nCount) = 0;
};

#endif
