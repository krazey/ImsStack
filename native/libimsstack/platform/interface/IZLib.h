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
#ifndef INTERFACE_ZLIB_H_
#define INTERFACE_ZLIB_H_

#include "ByteArray.h"

class IZLib
{
public:
    /**
     * @brief Compresses the specified raw data with a default options.
     *
     * @param objData The raw data to be compressed
     * @param objCompData The compressed data
     * @return IMS_TRUE if the compression is successfully done, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL Compress(IN const ByteArray& objData, OUT ByteArray& objCompData) = 0;

    /**
     * @brief Uncompresses the compressed data with a default options.
     *
     * @param objCompData The data to be uncompressed
     * @param objData The uncompressed data
     * @return IMS_TRUE if the uncompression is successfully done, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL Uncompress(IN const ByteArray& objCompData, OUT ByteArray& objData) = 0;
};

#endif
