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
#ifndef INTERFACE_COMMENT_H_
#define INTERFACE_COMMENT_H_

/**
 * @brief This class represents a comment.
 *
 * This interface inherits from CharacterData and represents the content of a comment,
 * i.e., all the characters between the starting ' <!--' and ending '-->'.
 * Note that this is the definition of a comment in XML, and, in practice, HTML,
 * although some HTML tools may implement the full SGML comment structure.
 *
 * @see ICharacterData, INode
 */
class IComment : public ICharacterData, public INode
{
protected:
    ~IComment() override = default;
};

#endif
