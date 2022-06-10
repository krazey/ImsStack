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
#ifndef INTERFACE_REG_INFO_PARSER_LISTENER_H_
#define INTERFACE_REG_INFO_PARSER_LISTENER_H_

class IDocument;
class RegInfoParser;

class IRegInfoParserListener
{
public:
    virtual void RegInfoParser_ParsingCompleted(
            IN RegInfoParser* pParser, IN IDocument* piDocument) = 0;
    virtual void RegInfoParser_ParsingFailed(IN RegInfoParser* pParser) = 0;
};

#endif
