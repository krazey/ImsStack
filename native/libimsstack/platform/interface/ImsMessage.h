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
#ifndef IMS_MESSAGE_H_
#define IMS_MESSAGE_H_

#include "AString.h"
#include "ImsStrLib.h"

#define IMSMSG ImsMessage

class ImsMessage
{
public:
    // This interface will be used in the system(platform) layer
    // to handle the asynchronous operations.
    // The applications can use this interface, but it should not be overused.
    class IMessageCallback
    {
    protected:
        virtual ~IMessageCallback() = default;

    public:
        virtual void MessageCallback_OnMessage(IN ImsMessage& objMsg) = 0;
    };

public:
    inline ImsMessage() :
            nMSG(0),
            nWparam(0),
            nLparam(0),
            piCallback(IMS_NULL)
    {
        acActivityName[0] = '\0';
    }

    inline ImsMessage(IN IMS_SINT32 nMSG_, IN IMS_UINTP nWparam_, IN IMS_UINTP nLparam_,
            IN const AString& strTarget = AString::ConstEmpty()) :
            nMSG(nMSG_),
            nWparam(nWparam_),
            nLparam(nLparam_),
            piCallback(IMS_NULL)
    {
        if (strTarget.GetLength() == 0)
        {
            acActivityName[0] = '\0';
        }
        else
        {
            IMS_StrCpy(acActivityName, MAX_ACTIVITY_NAME + 1, strTarget.GetStr());
        }
    }

    inline ImsMessage(IN IMS_SINT32 nMSG_, IN IMS_UINTP nWparam_, IN IMS_UINTP nLparam_,
            IN IMessageCallback* piCallback_) :
            nMSG(nMSG_),
            nWparam(nWparam_),
            nLparam(nLparam_),
            piCallback(piCallback_)
    {
        acActivityName[0] = '\0';
    }

    inline ~ImsMessage() {}

    inline const IMS_CHAR* GetTargetName() const
    {
        return (acActivityName[0] == '\0') ? IMS_NULL : &acActivityName[0];
    }

    inline IMS_SINT32 GetName() const { return nMSG; }

    inline void SetTarget(IN const IMS_CHAR* pszName)
    {
        IMS_StrCpy(acActivityName, MAX_ACTIVITY_NAME + 1, pszName);
    }

    inline IMS_BOOL HasCallback() const { return piCallback != IMS_NULL; }
    inline IMS_BOOL IsSameCallback(IN const IMessageCallback* piCallback) const
    {
        return this->piCallback != IMS_NULL && this->piCallback == piCallback;
    }
    inline void SetCallback(IN IMessageCallback* piCallback) { this->piCallback = piCallback; }
    inline void InvokeCallback()
    {
        if (piCallback != IMS_NULL)
        {
            piCallback->MessageCallback_OnMessage(*this);
        }
    }

public:
    IMS_SINT32 nMSG;
    IMS_UINTP nWparam;
    IMS_UINTP nLparam;

private:
    static const IMS_SINT32 MAX_ACTIVITY_NAME = 64;

    IMessageCallback* piCallback;

    // Empty Activity name means that the message is a broadcast
    IMS_CHAR acActivityName[MAX_ACTIVITY_NAME + 1];
};

#endif
