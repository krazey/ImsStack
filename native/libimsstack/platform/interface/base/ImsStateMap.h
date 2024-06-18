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
#ifndef IMS_STATE_MAP_H_
#define IMS_STATE_MAP_H_

#include "ImsMessage.h"
#include "ImsStateObject.h"

#define IMS_INVALID_STATE 0xFFFFFFFF
#define IMS_INVALID_MSG   0xFFFFFFFF

// clang-format off
#define DECLARE_STATE_MAP_BASE() \
protected:                       \
    virtual const StateMap* GetStateMap() const;

#define DECLARE_STATE_MAP() \
protected:                  \
    const StateMap* GetStateMap() const override;

#define BEGIN_STATE_MAP(THIS_CLASS)                           \
    PROTECTED const StateMap* THIS_CLASS::GetStateMap() const \
    {                                                         \
        typedef THIS_CLASS ThisClass;                         \
        static const StateMap STATE_MAP[] =                   \
        {                                                     \

            #define END_STATE_MAP()                           \
            { IMS_INVALID_STATE, IMS_NULL }                   \
        };                                                    \
        return &STATE_MAP[0];                                 \
    }

#define STATE_ENTRY(STATE) \
    { STATE, static_cast<GetStateMsgMap>(&ThisClass::Get##STATE##MsgMap) },

#define EMPTY_STATE_MAP(THIS_CLASS)                           \
    PROTECTED const StateMap* THIS_CLASS::GetStateMap() const \
    {                                                         \
        static const StateMap STATE_MAP[] =                   \
        {                                                     \
            { IMS_INVALID_STATE, IMS_NULL }                   \
        };                                                    \
        return &STATE_MAP[0];                                 \
    }

//// Macro definition to the event map for the state
#define DECLARE_STATE_MSG_MAP(STATE) \
private:                             \
    static const StateMsgMap* Get##STATE##MsgMap();

#define BEGIN_STATE_MSG_MAP(THIS_CLASS, STATE)                         \
    PRIVATE GLOBAL const StateMsgMap* THIS_CLASS::Get##STATE##MsgMap() \
    {                                                                  \
        static const StateMsgMap STATE_MSG_MAP[] =                     \
        {                                                              \

            #define END_STATE_MSG_MAP()                                \
            { IMS_INVALID_MSG, IMS_NULL }                              \
        };                                                             \
        return &STATE_MSG_MAP[0];                                      \
    }

#define STATE_MSG_ENTRY(MSG, MSGHandler) \
    { MSG, (StateMsgHandler)(MSGHandler) },

// clang-format on

typedef IMS_BOOL (ImsStateObject::*StateMsgHandler)(ImsMessage& objMsg);

struct StateMsgMap
{
    IMS_UINT32 nMsg;
    StateMsgHandler pfnStateMsgHandler;
};

typedef const StateMsgMap* (*GetStateMsgMap)();

struct StateMap
{
    IMS_UINT32 nState;
    GetStateMsgMap pfnGetStateMsgMap;
};

#endif
