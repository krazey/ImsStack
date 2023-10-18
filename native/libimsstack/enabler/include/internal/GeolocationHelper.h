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
#ifndef GEOLOCATION_HELPER_H_
#define GEOLOCATION_HELPER_H_

#include "GeolocationPidfCreator.h"

class GeolocationHelperPrivate;

class GeolocationHelper
{
private:
    GeolocationHelper();
    ~GeolocationHelper();

public:
    GeolocationHelper(IN const GeolocationHelper&) = delete;
    GeolocationHelper& operator=(IN const GeolocationHelper&) = delete;

public:
    void CreatePidfCreator(IN IMS_SINT32 nSlotId);
    void DestroyPidfCreator(IN IMS_SINT32 nSlotId);
    GeolocationPidfCreator* GetPidfCreator(IN IMS_SINT32 nSlotId);

    static GeolocationHelper* GetInstance();
    // Creates an identifier for Content-ID header field
    static AString CreateContentId(
            IN IMS_SINT32 nSlotId, IN const AString& strDomain = AString::ConstNull());
    // Returns the recent country or newly updated country
    static const AString& GetCountry(IN IMS_SINT32 nSlotId, IN IMS_BOOL bLocationUpdate);

private:
    static GeolocationHelper* s_pGeolocationHelper;

    GeolocationHelperPrivate* m_pPrivate;
};

#endif
