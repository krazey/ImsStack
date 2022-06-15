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
#ifndef EVENT_PACKAGE_H_
#define EVENT_PACKAGE_H_

#include "AStringArray.h"

class ISipHeader;

class EventPackage
{
public:
    EventPackage();
    virtual ~EventPackage();

    EventPackage(IN const EventPackage&) = delete;
    EventPackage& operator=(IN const EventPackage&) = delete;

public:
    inline IMS_UINT32 GetDefaultDuration() const { return DEFAULT_DURATION; }

    inline const AString& GetEvent() const { return m_strEvent; }
    inline const ISipHeader* GetEventHeader() const { return m_piEventHeader; }
    inline IMS_SINT32 GetDuration() const { return m_nInitialDuration; }
    inline const AStringArray& GetMimeTypes() const { return m_objMimeTypes; }

    inline void SetDuration(IN IMS_SINT32 nDuration) { m_nInitialDuration = nDuration; }
    inline void SetEvent(IN const AString& strEvent) { m_strEvent = strEvent; }
    void SetEventHeader(IN ISipHeader* piHeader);
    inline void SetMimeTypes(IN const AStringArray& objMimeTypes) { m_objMimeTypes = objMimeTypes; }

private:
    enum
    {
        DEFAULT_DURATION = 3600
    };

    // Event header
    AString m_strEvent;
    ISipHeader* m_piEventHeader;
    // In any case, MIN & MAX expiration value needs to be defined ...
    // Expires header
    //    - "Expires" header in the initial SUBSCRIBE request
    //    - We will take it as the default subscription duration for this event package
    IMS_SINT32 m_nInitialDuration;
    // Accept header
    AStringArray m_objMimeTypes;
    // Allow-Events header : It will be set from the AppConfig.
};

#endif
