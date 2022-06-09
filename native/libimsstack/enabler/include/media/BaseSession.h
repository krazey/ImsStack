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

#include "IMSList.h"
#include "IMediaSessionListener.h"
#include "MediaEnvironment.h"

class BaseSession
{
public:
    BaseSession(IN IMS_SINT32 nSlodId = 0);
    virtual ~BaseSession();
    virtual void SetServiceType(MEDIA_SERVICE_TYPE eServiceType);
    virtual void SetMediaSessionListener(IN IMediaSessionListener* pListener);
    virtual void SetMediaEnvironment(MediaEnvironment* pEnvironment);
    virtual void SetDirection(MEDIA_DIRECTION eDir);
    virtual IMS_SINT32 GetState();
    //    virtual IMS_BOOL IsSameRemoteNetwork(IPAddress address, IMS_UINT32 port);
    // do it later : IsSameRemoteNetwork is currently not used

protected:
    IMS_SINT32 m_nSlodId;
    IMediaSessionListener* m_piMediaSessionListener;  // to MediaSession
    MediaEnvironment* m_pEnvironment;
    MEDIA_DIRECTION m_eEnforcedDirection;
    IMS_SINT32 m_nState;
};

#endif