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
#include "ServiceTrace.h"
#include "ServiceMemory.h"
#include "ServiceTimer.h"
#include "ImsMap.h"
#include "interface/IAosMsgHandlerListener.h"
#include "provider/AosUtil.h"
#include "provider/AosMsgHandler.h"

//__IMS_TRACE_TAG_USER_DECL__("AOS");

class AosMessage
{
public:
    AosMessage() :
            piListener(IMS_NULL),
            piTimer(IMS_NULL),
            nMessage(-1),
            nDuration(0),
            bDead(IMS_FALSE)
    {
        IMS_TRACE_MEM("AOS_MEM", "AOS_M : AosMessage = %" PFLS_u "/%" PFLS_x, sizeof(AosMessage),
                this, 0);
    }

    AosMessage(IN CONST IAosMsgHandlerListener* piListener, IN CONST ITimer* piTimer,
            IN IMS_SINT32 nMessage, IN IMS_SINT32 nDuration) :
            piListener(piListener),
            piTimer(piTimer),
            nMessage(nMessage),
            nDuration(nDuration),
            bDead(IMS_FALSE)
    {
        IMS_TRACE_MEM("AOS_MEM", "AOS_M : AosMessage = %" PFLS_u "/%" PFLS_x, sizeof(AosMessage),
                this, 0);
    }

    virtual ~AosMessage()
    {
        IMS_TRACE_MEM("AOS_MEM", "AOS_F : AosMessage = %" PFLS_u "/%" PFLS_x, sizeof(AosMessage),
                this, 0);
    }

    IMS_BOOL IsEqual(IN CONST IAosMsgHandlerListener* piListener, IN IMS_SINT32 nMessage)
    {
        if ((!bDead) && (this->piListener == piListener) && (this->nMessage == nMessage))
        {
            return IMS_TRUE;
        }

        return IMS_FALSE;
    }

    void HandleMessage()
    {
        if (piListener != IMS_NULL)
        {
            bDead = IMS_TRUE;

            const_cast<IAosMsgHandlerListener*>(piListener)->HandleMessage(nMessage);

            delete this;
        }
    }

    const ITimer* GetTimer() { return piTimer; }
    IMS_SINT32 GetDuration() { return nDuration; }

private:
    const IAosMsgHandlerListener* piListener;
    const ITimer* piTimer;
    IMS_SINT32 nMessage;
    IMS_SINT32 nDuration;
    IMS_BOOL bDead;
};

/*

Remarks

*/
PUBLIC
AosMsgHandler::AosMsgHandler() :
        objMessages(IMSMap<ITimer*, AosMessage*>())
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : AosMsgHandler = %" PFLS_u "/%" PFLS_x, sizeof(AosMsgHandler),
            this, 0);
}

/*

Remarks

*/
PUBLIC VIRTUAL AosMsgHandler::~AosMsgHandler()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : AosMsgHandler = %" PFLS_u "/%" PFLS_x, sizeof(AosMsgHandler),
            this, 0);

    for (IMS_UINT32 i = 0; i < objMessages.GetSize(); ++i)
    {
        AosMessage* pMsg = objMessages.GetValueAt(i);

        if (pMsg != IMS_NULL)
        {
            ITimer* piTimer = const_cast<ITimer*>(pMsg->GetTimer());
            if (piTimer != IMS_NULL)
            {
                StopTimer(piTimer);
            }

            delete pMsg;
        }
    }

    objMessages.Clear();
}

/*

Remarks

*/
PRIVATE
IMS_BOOL AosMsgHandler::HasMessage(
        IN CONST IAosMsgHandlerListener* piListener, IN IMS_SINT32 nMessage, OUT IMS_SINT32& nAt)
{
    if (objMessages.IsEmpty())
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < objMessages.GetSize(); ++i)
    {
        AosMessage* pMsg = objMessages.GetValueAt(i);

        if (pMsg != IMS_NULL)
        {
            if (pMsg->IsEqual(piListener, nMessage))
            {
                nAt = i;
                return IMS_TRUE;
            }
        }
    }

    return IMS_FALSE;
}

/*

Remarks

*/
PRIVATE
ITimer* AosMsgHandler::StartTimer(IN IMS_SINT32 nDuration)
{
    if (nDuration <= 0)
    {
        return IMS_NULL;
    }

    return AosUtil::GetInstance()->StartTimer(nDuration, this, "AosMsgHandler");
}

/*

Remarks

*/
PRIVATE
void AosMsgHandler::StopTimer(IN ITimer* piTimer)
{
    AosUtil::GetInstance()->StopTimer(piTimer, "AosMsgHandler");
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL AosMsgHandler::SendEmptyMessageDelayed(
        IN CONST IAosMsgHandlerListener* piListener, IN IMS_SINT32 nMessage,
        IN IMS_SINT32 nDuration)
{
    if (nDuration < 0)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nAt = (-1);

    if (HasMessage(piListener, nMessage, nAt))
    {
        return IMS_FALSE;
    }

    ITimer* piTimer = StartTimer(nDuration);

    if (piTimer == IMS_NULL)
    {
        return IMS_FALSE;
    }

    AosMessage* pMsg = new AosMessage(piListener, piTimer, nMessage, nDuration);

    if (pMsg == IMS_NULL)
    {
        StopTimer(piTimer);
        return IMS_FALSE;
    }

    objMessages.Add(piTimer, pMsg);

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC VIRTUAL void AosMsgHandler::RemoveMessages(
        IN CONST IAosMsgHandlerListener* piListener, IN IMS_SINT32 nMessage)
{
    IMS_SINT32 nAt = (-1);
    if (HasMessage(piListener, nMessage, nAt))
    {
        AosMessage* pMsg = objMessages.GetValueAt(nAt);

        if (pMsg != IMS_NULL)
        {
            StopTimer(const_cast<ITimer*>(pMsg->GetTimer()));
            delete pMsg;
        }

        objMessages.RemoveAt(nAt);
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL AosMsgHandler::HasMessages(
        IN CONST IAosMsgHandlerListener* piListener, IN IMS_SINT32 nMessage)
{
    if (objMessages.IsEmpty())
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < objMessages.GetSize(); ++i)
    {
        AosMessage* pMsg = objMessages.GetValueAt(i);

        if (pMsg->IsEqual(piListener, nMessage))
        {
            return IMS_TRUE;
        }

        continue;
    }

    return IMS_FALSE;
}

/*

Remarks

*/
PUBLIC VIRTUAL void AosMsgHandler::Timer_TimerExpired(IN ITimer* piTimer)
{
    if (piTimer == IMS_NULL)
    {
        return;
    }

    IMS_SLONG nIndex = objMessages.GetIndexOfKey(piTimer);
    if (nIndex < 0)
    {
        StopTimer(piTimer);
        return;
    }

    AosMessage* pMsg = objMessages.GetValueAt(nIndex);

    if (pMsg != IMS_NULL)
    {
        pMsg->HandleMessage();
        objMessages.Remove(piTimer);
    }

    StopTimer(piTimer);
}
