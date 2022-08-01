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
#ifndef AOS_MESSAGE_HANDLER_H_
#define AOS_MESSAGE_HANDLER_H_

class AosMessage;
class IAosMsgHandlerListener;

#include "interface/IAosMsgHandler.h"
#include "ITimer.h"

class AosMsgHandler : public IAosMsgHandler, public ITimerListener
{
public:
    AosMsgHandler();
    virtual ~AosMsgHandler();

public:
    virtual IMS_BOOL SendEmptyMessageDelayed(IN const IAosMsgHandlerListener* piListener,
            IN IMS_SINT32 nMessage, IN IMS_SINT32 nDuration);
    virtual void RemoveMessages(
            IN const IAosMsgHandlerListener* piListener, IN IMS_SINT32 nMessage);
    virtual IMS_BOOL HasMessages(
            IN const IAosMsgHandlerListener* piListener, IN IMS_SINT32 nMessage);

    virtual void Timer_TimerExpired(IN ITimer* piTimer);

private:
    IMS_BOOL HasMessage(IN const IAosMsgHandlerListener* piListener, IN IMS_SINT32 nMessage,
            OUT IMS_SINT32& nAt);
    ITimer* StartTimer(IN IMS_SINT32 nDuration);
    void StopTimer(IN ITimer* piTimer);

private:
    IMSMap<ITimer*, AosMessage*> objMessages;
};

#endif  // AOS_MESSAGE_HANDLER_H_
