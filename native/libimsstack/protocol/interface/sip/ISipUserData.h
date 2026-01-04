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
#ifndef __INTERFACE_SIP_USER_DATA_H__
#define __INTERFACE_SIP_USER_DATA_H__

#include "SipConfiguration.h"
#include "SipDatatypes.h"
#include "msg/SipMessage.h"
/****************************************************************************
ISipUserData: Class Declaration Starts
 *****************************************************************************/
class ISipUserData
{
private:
    SIP_UINT32 m_nMsgOptions;
    SIP_BOOL m_bDeleteFlag;
    SipMessage* m_pSipMsg;  // Tracks the SIP message sent by the SIP stack.
    SIP_VOID* m_pvUserData; /* Stack User Specific Data */

public:
    inline ISipUserData() :
            m_nMsgOptions(SipConfiguration::MSG_OPT_ENCODE_NONE),
            m_bDeleteFlag(SIP_FALSE),
            m_pSipMsg(SIP_NULL),
            m_pvUserData(SIP_NULL)
    {
    }

    inline explicit ISipUserData(SIP_VOID* pvUserData) :
            m_nMsgOptions(SipConfiguration::MSG_OPT_ENCODE_NONE),
            m_bDeleteFlag(SIP_FALSE),
            m_pSipMsg(SIP_NULL),
            m_pvUserData(pvUserData)
    {
    }

    inline SIP_VOID* GetUserData() const { return m_pvUserData; }

    inline void SetUserData(SIP_VOID* pvUserData) { m_pvUserData = pvUserData; }

    inline SIP_BOOL GetDeleteFlag() const { return m_bDeleteFlag; }

    inline void SetDeleteFlag(SIP_BOOL bFlag) { m_bDeleteFlag = bFlag; }

    inline SIP_UINT32 GetMsgOptions() const { return m_nMsgOptions; }

    inline void SetMsgOptions(SIP_UINT32 nMsgOptions) { m_nMsgOptions = nMsgOptions; }

    inline SipMessage* GetSipMsg() const { return m_pSipMsg; }

    inline void SetSipMsg(SipMessage* pSipMsg)
    {
        if (m_pSipMsg != SIP_NULL)
        {
            m_pSipMsg->SipDelete();
        }

        m_pSipMsg = pSipMsg;

        if (m_pSipMsg != SIP_NULL)
        {
            m_pSipMsg->Increment();
        }
    }

    inline virtual ~ISipUserData()
    {
        if (m_pSipMsg != SIP_NULL)
        {
            m_pSipMsg->SipDelete();
        }
    }
};

#endif  //__INTERFACE_SIP_USER_DATA_H__
