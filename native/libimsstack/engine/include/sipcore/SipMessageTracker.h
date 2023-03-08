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
#ifndef SIP_MESSAGE_TRACKER_H_
#define SIP_MESSAGE_TRACKER_H_

#include "ISipMessageTracker.h"

class SipMessageTracker : public ISipMessageTracker
{
public:
    inline SipMessageTracker() :
            m_piListener(IMS_NULL)
    {
    }
    inline virtual ~SipMessageTracker() {}

    SipMessageTracker(IN const SipMessageTracker&) = delete;
    SipMessageTracker& operator=(IN const SipMessageTracker&) = delete;

public:
    inline IMS_BOOL IsMessageTrackerEnabled() const { return (m_piListener != IMS_NULL); }

    void NotifyMessageReceived(
            IN const SipMethod& objMethod, IN IMS_SINT32 nStatusCode, IN const AString& strCallId);
    void NotifyMessageSent(IN const SipMethod& objMethod, IN IMS_SINT32 nStatusCode,
            IN const AString& strCallId, IN IMS_SINT32 nErrorCode = 0);

private:
    // ISipMessageTracker class
    IMS_BOOL AddFilter(IN const SipMethod& objMethod, IN IMS_SINT32 nStatusCode,
            IN IMS_BOOL bOutgoing) override;
    void RemoveFilter(IN const SipMethod& objMethod) override;
    void RemoveFilter(IN const SipMethod& objMethod, IN IMS_SINT32 nStatusCode,
            IN IMS_BOOL bOutgoing) override;
    void RemoveAllFilters() override;
    inline void SetListener(IN ISipMessageTrackerListener* piListener) override
    {
        m_piListener = piListener;
    }

private:
    class MessageFilter
    {
    public:
        inline MessageFilter() :
                m_objMethod(SipMethod()),
                m_nStatusCode(0)
        {
        }
        inline MessageFilter(IN const SipMethod& objMethod, IN IMS_SINT32 nStatusCode) :
                m_objMethod(objMethod),
                m_nStatusCode(nStatusCode)
        {
        }
        inline ~MessageFilter() {}

        MessageFilter(IN const MessageFilter&) = delete;
        MessageFilter& operator=(IN const MessageFilter&) = delete;

    public:
        inline IMS_BOOL Equals(IN const SipMethod& objMethod, IN IMS_SINT32 nStatusCode)
        {
            if (!m_objMethod.Equals(objMethod))
            {
                return IMS_FALSE;
            }

            if (m_nStatusCode != nStatusCode)
            {
                return IMS_FALSE;
            }

            return IMS_TRUE;
        }

        inline const SipMethod& GetMethod() const { return m_objMethod; }
        inline IMS_SINT32 GetStatusCode() const { return m_nStatusCode; }

    private:
        SipMethod m_objMethod;
        IMS_SINT32 m_nStatusCode;
    };

    ImsList<MessageFilter*> m_objIncomingFilters;
    ImsList<MessageFilter*> m_objOutgoingFilters;
    ISipMessageTrackerListener* m_piListener;
};

#endif
