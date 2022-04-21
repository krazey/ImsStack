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
#ifndef AOS_BUILD_DIRECTOR_H_
#define AOS_BUILD_DIRECTOR_H_

#include "IMSMap.h"

class AString;

class IAosBuilder;
class IAosAppContext;

class IAosConnection;
class IAosNetTracker;
class AosStaticProfile;

class AosBuildDirector
{
public:
    AosBuildDirector(IN IAosBuilder* piBuilder, IN IMS_SINT32 nSlotId);
    virtual ~AosBuildDirector();

    IAosAppContext* ConstructAos(IN AosStaticProfile* pProfile);
    void ConstructProvider();
    void DestructAos();
    void DestructProvider();

private:
    IMS_SINT32 m_nSlotId;
    IAosBuilder* m_piBuilder;

    IMSMap<IMS_SINT32, IAosConnection*> m_objConnection;
    IMSMap<IMS_SINT32, IAosNetTracker*> m_objNetTracker;
    IMSMap<AString, IAosAppContext*> m_objAppContext;
};
#endif // AOS_BUILD_DIRECTOR_H_
