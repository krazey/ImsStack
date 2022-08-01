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

#ifndef _UCE_RLMI_COMPOSER_H_
#define _UCE_RLMI_COMPOSER_H_

#include "AString.h"

class IXmlStreamWriter;
class XmlFactory;

class UceRlmiComposer
{
    /* ------------------------------------------------------------------------------------------
        Constructor, Destructor, Operator Overloading
    ---------------------------------------------------------------------------------------------
  */
public:
    UceRlmiComposer();

    ~UceRlmiComposer();
    /* ------------------------------------------------------------------------------------------
        Methods
    ---------------------------------------------------------------------------------------------
  */
public:
    AString ComposeRLMIList(IN IMSList<AString>& pContactInfoList);  //
private:
    void EncodeResourceXMLNameSpace(IN IXmlStreamWriter*& piWriter);  //
    /* ------------------------------------------------------------------------------------------
        Variables
    ---------------------------------------------------------------------------------------------
  */
private:
    XmlFactory* m_pXMLOutputFactory;
};
#endif  // _UCE_RLMI_COMPOSER_H_
