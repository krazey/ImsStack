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
#ifndef REG_SUBJECT_H_
#define REG_SUBJECT_H_

#include "IMSList.h"

#include "RegObserver.h"

class RegSubject
{
public:
    RegSubject();
    virtual ~RegSubject();

    RegSubject(IN const RegSubject&) = delete;
    RegSubject& operator=(IN const RegSubject&) = delete;

public:
    virtual void RegisterObserver(IN RegObserver* pObserver);
    virtual void RemoveObserver(IN RegObserver* pObserver);

protected:
    virtual void NotifyObservers(IN IMS_SINT32 nWhat);

private:
    IMSList<RegObserver*> m_objObservers;
};

#endif
