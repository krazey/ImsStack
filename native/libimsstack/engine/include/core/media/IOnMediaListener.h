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
#ifndef INTERFACE_ON_MEDIA_LISTENER_H_
#define INTERFACE_ON_MEDIA_LISTENER_H_

#include "ImsTypeDef.h"

class Media;

/**
 * @brief A listener type for receiving notification of when some mode property has changed
 *        on the media flow.
 */
class IOnMediaListener
{
public:
    /**
     * @brief The method is called when the fictitious media is created.
     *
     * The fictitious media that is only meant to track changes that are about to be made
     * to the media.
     *
     * NOTE:
     * After the Session has been accepted or rejected, this proposal media should be considered
     * discarded.
     *
     * @param pMedia The concerned media object
     */
    virtual void OnMedia_FictitiousMediaCreated(IN Media* pMedia) = 0;

    /**
     * @brief The method is called when the fictitious media is destroyed.
     *
     * The fictitious media that is only meant to track changes that are about to be made
     * to the media.
     *
     * NOTE:
     * After the Session has been accepted or rejected, this proposal media should be considered
     * discarded.
     *
     * @param pMedia The concerned Media object
     */
    virtual void OnMedia_FictitiousMediaDestroyed(IN Media* pMedia) = 0;
};

#endif
