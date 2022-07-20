/**
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

#ifndef _IMS_BASE_SESSION_H_
#define _IMS_BASE_SESSION_H_

#include "ImsList.h"
#include "IMediaSessionListener.h"
#include "MediaEnvironment.h"

class BaseSession
{
public:
    BaseSession(IN IMS_SINT32 nSlodId = 0);
    virtual ~BaseSession();

    /**
     * @brief Set the media service type of the sesison
     *
     * @param eServiceType Defined MEDIA_SERVICE_TYPE in MediaDef.h
     */
    virtual void SetServiceType(MEDIA_SERVICE_TYPE eServiceType);

    /**
     * @brief Set the listener to MediaSession
     *
     * @param pListener The listener instance to set
     */
    virtual void SetMediaSessionListener(IN IMediaSessionListener* pListener);

    /**
     * @brief Set the MediaEnvironment instance to get the common parameters of the call
     *
     * @param pEnvironment The instance to set
     */
    virtual void SetMediaEnvironment(MediaEnvironment* pEnvironment);

    /**
     * @brief Set the media direction
     *
     * @param eDir the media direction to set
     */
    virtual void SetDirection(MEDIA_DIRECTION eDir);

    /**
     * @brief Get the session state
     *
     * @return IMS_SINT32 The state
     */
    virtual IMS_SINT32 GetState();

    /**
     * @brief Set the session state
     *
     * @param state The state to set
     */
    virtual void SetState(IMS_SINT32 state);

protected:
    IMS_SINT32 m_nSlodId;
    IMediaSessionListener* m_piMediaSessionListener;
    MediaEnvironment* m_pEnvironment;
    MEDIA_DIRECTION m_eEnforcedDirection;
    IMS_SINT32 m_nState;
};

#endif