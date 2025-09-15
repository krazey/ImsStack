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
#ifndef INTERFACE_TEXT_H
#define INTERFACE_TEXT_H

#include "ICharacterData.h"

/**
 * @brief This class represents the textual content (termed character data in XML)
 *        of an element or attribute.
 *
 * @see ICharacterData
 */
class IText : public ICharacterData
{
protected:
    ~IText() override = default;

public:
    /**
     * @brief Breaks this node into two nodes at the specified offset,
     *        keeping both in the tree as siblings.
     *
     * After being split, this node will contain all the content up to the offset point.
     *
     * @param nOffset The 16-bit unit offset at which to split, starting from 0
     * @return The new node, of the same type as this node.
     */
    virtual IText* SplitText(IN IMS_SINT32 nOffset) = 0;
};

#endif
