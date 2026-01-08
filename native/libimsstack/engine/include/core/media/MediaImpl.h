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
#ifndef MEDIA_IMPL_H_
#define MEDIA_IMPL_H_

#include "media/IMedia.h"

class Media;

class MediaImpl
{
public:
    MediaImpl() = default;
    virtual ~MediaImpl() = default;

public:
    virtual IMS_BOOL Equals(IN const IMedia* piMedia) const = 0;
    virtual IMedia* GetInterface() = 0;
    virtual Media* GetMedia() const = 0;
};

#endif
