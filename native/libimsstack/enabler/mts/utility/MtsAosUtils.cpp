/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "ImsAosParameter.h"
#include "MtsDef.h"
#include "ServiceTrace.h"
#include "utility/MtsAosUtils.h"

__IMS_TRACE_TAG_COM_MTS__;

PUBLIC
MtsAosUtils::MtsAosUtils()
{
    IMS_TRACE_I("+MtsAosUtils", 0, 0, 0);
}

PUBLIC
MtsAosUtils::~MtsAosUtils()
{
    IMS_TRACE_I("~MtsAosUtils", 0, 0, 0);
}

PUBLIC
IMS_SINT32 MtsAosUtils::ConvertToAosControl(IN IMS_SINT32 nPolicy)
{
    // Mapping ImsAosControl enum to internal Mts enum to avoid the need to update Mts assets
    // in case ImsAosControl values are re-arranged.
    switch (static_cast<MtsRegRecoveryPolicy::Policy>(nPolicy))
    {
        case MtsRegRecoveryPolicy::REGISTER_START:
            return ImsAosControl::REGISTER_START;

        case MtsRegRecoveryPolicy::REGISTER_START_WITH_WLAN:
            return ImsAosControl::REGISTER_START_WITH_WLAN;

        case MtsRegRecoveryPolicy::REGISTER_REFRESH:
            return ImsAosControl::REGISTER_REFRESH;

        case MtsRegRecoveryPolicy::REGISTER_STOP:
            return ImsAosControl::REGISTER_STOP;

        case MtsRegRecoveryPolicy::REGISTER_STOP_BY_ROAMING:
            return ImsAosControl::REGISTER_STOP_BY_ROAMING;

        case MtsRegRecoveryPolicy::REGISTER_REINITIATE:
            return ImsAosControl::REGISTER_REINITIATE;

        case MtsRegRecoveryPolicy::REGISTER_REINITIATE_BY_CSFB:
            return ImsAosControl::REGISTER_REINITIATE_BY_CSFB;

        case MtsRegRecoveryPolicy::E_REGISTER_FAKE_WITH_NEXT_PCSCF:
            return ImsAosControl::E_REGISTER_FAKE_WITH_NEXT_PCSCF;

        case MtsRegRecoveryPolicy::PCSCF_NEXT:
            return ImsAosControl::PCSCF_NEXT;

        case MtsRegRecoveryPolicy::PCSCF_NEXT_WITH_DISCOVERY:
            return ImsAosControl::PCSCF_NEXT_WITH_DISCOVERY;

        case MtsRegRecoveryPolicy::IPSEC_DISABLED:
            return ImsAosControl::IPSEC_DISABLED;

        case MtsRegRecoveryPolicy::RETRY_COUNT_INCREASE:
            return ImsAosControl::RETRY_COUNT_INCREASE;

        case MtsRegRecoveryPolicy::RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION:
            return ImsAosControl::RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION;

        case MtsRegRecoveryPolicy::UPDATE_SIP_DELEGATE_REGISTRATION:
            return ImsAosControl::UPDATE_SIP_DELEGATE_REGISTRATION;

        case MtsRegRecoveryPolicy::TRIGGER_SIP_DELEGATE_DEREGISTRATION:
            return ImsAosControl::TRIGGER_SIP_DELEGATE_DEREGISTRATION;

        case MtsRegRecoveryPolicy::TRIGGER_FULL_NETWORK_REGISTRATION:
            return ImsAosControl::TRIGGER_FULL_NETWORK_REGISTRATION;

        case MtsRegRecoveryPolicy::PLMN_BLOCK_WITH_TIMEOUT:
            return ImsAosControl::PLMN_BLOCK_WITH_TIMEOUT;

        case MtsRegRecoveryPolicy::E_REGISTER_FAKE_WITH_SAME_PCSCF:
            return ImsAosControl::E_REGISTER_FAKE_WITH_SAME_PCSCF;

        default:
            return MtsRegRecoveryPolicy::MTS_REG_RECOVERY_POLICY_NONE;
    }
}
