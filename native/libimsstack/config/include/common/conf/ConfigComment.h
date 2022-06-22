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
#ifndef CONFIG_COMMENT_H_
#define CONFIG_COMMENT_H_

#include "AStringArray.h"

class ConfigComment
{
public:
    inline ConfigComment() {}
    inline ~ConfigComment() {}

    ConfigComment(IN const ConfigComment&) = delete;
    ConfigComment& operator=(IN const ConfigComment&) = delete;

public:
    inline void Add(IN const AString& strComment) { m_objComments.AddElement(strComment); }
    AString ToString() const;

private:
    AStringArray m_objComments;
};

#endif
