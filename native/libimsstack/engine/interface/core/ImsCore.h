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
#ifndef IMS_CORE_H_
#define IMS_CORE_H_

#include "ImsTypeDef.h"

/**
 * @brief This class defines the constant values for ICoreService ("imscore" protocol).
 */
class ImsCore
{
public:
    //// Connection related constants
    /// "imscore"
    static const IMS_CHAR CONNECTION_SCHEME[];

    //// App & Service related constants

    //// ServiceMethod related constants

    //// Media related constants
    /// "StreamMedia"
    static const IMS_CHAR MEDIA_STREAM[];
    /// "FramedMedia"
    static const IMS_CHAR MEDIA_FRAMED[];
    /// "BasicReliableMedia"
    static const IMS_CHAR MEDIA_BASIC_RELIABLE[];
    /// "BasicUnreliableMedia"
    static const IMS_CHAR MEDIA_BASIC_UNRELIABLE[];

    /// Types of media
    enum
    {
        MEDIA_TYPE_STREAM = 0,
        MEDIA_TYPE_FRAMED,
        MEDIA_TYPE_BASIC_RELIABLE,
        MEDIA_TYPE_BASIC_UNRELIABLE
    };
};

#endif
