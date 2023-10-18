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
#ifndef IMS_TIMER_H_
#define IMS_TIMER_H_

#include "IThread.h"
#include "ITimer.h"

class ImsTimer : public ITimer
{
public:
    inline ImsTimer() :
            m_piOwner(IMS_NULL),
            m_piListener(IMS_NULL)
    {
    }
    inline virtual ~ImsTimer() {}

public:
    inline IThread* GetOwner() const { return m_piOwner; }

    inline virtual void Destroy() { delete this; }

    virtual IMS_UINTP GetTimerId() const = 0;
    virtual void DispatchServiceMessage(IN IMS_UINTP nWparam, IN IMS_UINTP nLparam) = 0;

public:
    // To destroy a timer object on owner thread
    enum
    {
        MSG_PARAM_DESTROY = 0
    };

protected:
    IThread* m_piOwner;
    ITimerListener* m_piListener;
};

#endif
