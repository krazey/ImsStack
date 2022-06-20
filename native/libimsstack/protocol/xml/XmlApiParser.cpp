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
#include "XmlApiParser.h"

void XmlApi_CleanupParser()
{
    xmlCleanupParser();
}

xmlDocPtr XmlApi_ReadDoc(const xmlChar* cur, const char* url, const char* encoding, int options)
{
    return xmlReadDoc(cur, url, encoding, options);
}

xmlDocPtr XmlApi_ReadMemory(
        const char* buffer, int size, const char* url, const char* encoding, int options)
{
    return xmlReadMemory(buffer, size, url, encoding, options);
}
