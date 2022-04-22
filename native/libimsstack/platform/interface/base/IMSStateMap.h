/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100331  joonhun.shin@             create (hwangoo.park@ base code)
    </table>

    Description

*/

#ifndef _IMS_STATEMAP_H_
#define _IMS_STATEMAP_H_

#include "ImsMessage.h"
#include "IMSStateObject.h"

#define INVALID_STATE    0xFFFFFFFF
#define INVALID_MSG        0xFFFFFFFF

#define DECLARE_STATE_MAP() \
protected: \
    virtual const StateMap* GetStateMap() const;

#define BEGIN_STATE_MAP(THIS_CLASS) \
PROTECTED const StateMap* THIS_CLASS::GetStateMap() const\
{ \
    typedef THIS_CLASS ThisClass; \
    static const StateMap STATE_MAP[] = \
    { \

        #define END_STATE_MAP() \
        { INVALID_STATE, IMS_NULL } \
    }; \
    return &STATE_MAP[0]; \
}

#define STATE_ENTRY(STATE) \
{ STATE, (GetStateMsgMap)(&ThisClass::Get##STATE##MsgMap) },

#define EMPTY_STATE_MAP(THIS_CLASS) \
PROTECTED const StateMap* THIS_CLASS::GetStateMap() const\
{ \
    static const StateMap STATE_MAP[] = \
    { \
        { INVALID_STATE, IMS_NULL } \
    }; \
    return &STATE_MAP[0]; \
}

//// Macro definition to the event map for the state
#define DECLARE_STATE_MSG_MAP(STATE) \
private: \
    static const StateMsgMap* Get##STATE##MsgMap();

#define BEGIN_STATE_MSG_MAP(THIS_CLASS, STATE) \
PRIVATE GLOBAL const StateMsgMap* THIS_CLASS::Get##STATE##MsgMap() \
{ \
    static const StateMsgMap STATE_MSG_MAP[] = \
    { \

        #define END_STATE_MSG_MAP() \
        {INVALID_MSG, IMS_NULL } \
    }; \
    return &STATE_MSG_MAP[0]; \
}

#define STATE_MSG_ENTRY(MSG, MSGHandler) \
{ MSG, (StateMsgHandler)(MSGHandler) },

typedef IMS_BOOL (IMSStateObject::*StateMsgHandler)(IMSMSG &objMsg);

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

#endif // _IMS_STATEMAP_H_
