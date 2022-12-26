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
#ifndef SIP_DIALOG_USAGE_H_
#define SIP_DIALOG_USAGE_H_

#include "SipDialogBase.h"

/**
 * @brief This class defines a dialog usage for SIP.
 *
 * Several methods in the SIP can create an association between endpoints known as a dialog.
 * Some of these methods can also create a different, but related, association
 * within an existing dialog.
 * These multiple associations, or dialog usages, require carefully coordinated processing
 * as they have independent life-cycles, but share common dialog state.
 *
 * There are two dialog usages.
 *  - A dialog initiated with an INVITE request has an invite usage.
 *  - A dialog initiated with a SUBSCRIBE request has a subscribe usage.
 *  - A dialog initiated with a REFER request has a subscribe usage.
 */
class SipDialogUsage
{
public:
    inline explicit SipDialogUsage(IN SipDialogBase* pDialogBase) :
            m_nType(TYPE_EPHEMERAL),
            m_pDialogBase(pDialogBase)
    {
    }
    inline SipDialogUsage(IN IMS_SINT32 nType, IN SipDialogBase* pDialogBase) :
            m_nType(nType),
            m_pDialogBase(pDialogBase)
    {
    }
    inline SipDialogUsage(IN const SipDialogUsage& other) :
            m_nType(other.m_nType),
            m_pDialogBase(other.m_pDialogBase)
    {
    }
    virtual ~SipDialogUsage();

    SipDialogUsage() = delete;
    SipDialogUsage& operator=(IN const SipDialogUsage&) = delete;

public:
    virtual IMS_BOOL InitDialogUsage(IN const SipMessageInfo& objMsgInfo);
    inline virtual SipDialogUsage* Clone() const { return new SipDialogUsage(*this); }
    inline virtual IMS_BOOL CompareTo(IN const SipMessageInfo& /*objMsgInfo*/) const
    {
        return IMS_TRUE;
    }
    virtual IMS_BOOL Equals(IN SipDialogUsage* pDialogUsage) const;
    virtual AString ToString() const;
    virtual IMS_SINT32 UpdateUsageDetails(IN const SipMessageInfo& objMsgInfo);

    inline IMS_SINT32 GetType() const { return m_nType; }

protected:
    virtual IMS_SINT32 GetActionNTrigger(
            IN const SipMessageInfo& objMsgInfo, OUT IMS_SINT32& nTrigger);
    inline virtual IMS_BOOL IsUsageTerminated(
            IN IMS_SINT32 /*nState*/, IN IMS_SINT32 /*nTrigger*/) const
    {
        return IMS_TRUE;
    }

    IMS_SINT32 GetActionForResponse(IN const SipMessageInfo& objMsgInfo);
    IMS_SINT32 GetState() const;

    virtual const IMS_CHAR* TriggerToString(IN IMS_SINT32 nTrigger) const;
    static const IMS_CHAR* ActionToString(IN IMS_SINT32 nAction);

public:
    enum
    {
        TYPE_EPHEMERAL = 0,
        TYPE_INVITE,
        TYPE_SUBSCRIBE
    };

private:
    // Usage type: invite / subscribe / register(pseudo)
    IMS_SINT32 m_nType;
    SipDialogBase* m_pDialogBase;
};

#endif
