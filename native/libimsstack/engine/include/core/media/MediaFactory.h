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
#ifndef MEDIA_FACTORY_H_
#define MEDIA_FACTORY_H_

#include "media/Media.h"

class ISdpOaState;
class Service;

class MediaFactory
{
public:
    MediaFactory() = delete;

public:
    static Media* CreateOutgoingMedia(IN const AString& strMType, IN IMS_SINT32 nDirection,
            IN Service* pService, IN ISdpOaState* piOaState, IN IMS_SINT32 nCountOfDescriptor);

    static Media* CreateIncomingMedia(IN IMS_SINT32 nTransportProtocol, IN Service* pService,
            IN ISdpOaState* piOaState, IN const IMSList<IMS_SINT32>& objMids);

    static void DestroyMedia(IN Media*& pMedia);
};

#endif
