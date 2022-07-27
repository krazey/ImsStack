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
#ifndef OS_PARCEL_H_
#define OS_PARCEL_H_

#include <binder/Parcel.h>

#include "ImsParcel.h"

class OsParcel : public ImsParcel
{
public:
    inline OsParcel() {}
    inline virtual ~OsParcel() {}

    OsParcel(IN const OsParcel&) = delete;
    OsParcel& operator=(IN const OsParcel&) = delete;

public:
    const android::Parcel& GetParcel() const { return m_objParcel; }
    android::Parcel& GetParcel() { return m_objParcel; }

private:
    android::Parcel m_objParcel;
};

#endif
