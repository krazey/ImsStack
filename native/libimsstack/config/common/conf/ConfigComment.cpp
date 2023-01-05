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
#include "ServiceMemory.h"
#include "TextParser.h"

#include "conf/ConfigComment.h"

PUBLIC
AString ConfigComment::ToString() const
{
    if (m_objComments.IsEmpty())
    {
        return AString::ConstNull();
    }

    AString strTmpVal;

    for (IMS_SINT32 i = 0; i < m_objComments.GetCount(); ++i)
    {
        strTmpVal.Append(m_objComments.GetElementAt(i));
        strTmpVal.Append(TextParser::STR_CRLF);
    }

    return strTmpVal;
}
