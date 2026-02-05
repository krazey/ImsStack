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

package com.android.imsstack.imsservice.mmtel;

import android.annotation.NonNull;
import android.annotation.Nullable;
import android.net.Uri;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsRegistrationAttributes;
import android.telephony.ims.RegistrationManager;
import android.telephony.ims.stub.ImsRegistrationImplBase;
import android.util.Pair;

import com.android.imsstack.enabler.aos.IAosRegistration;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.ReasonCode;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.ReasonCodeMap;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.RegistrationType;
import com.android.imsstack.imsservice.mmtel.reg.IRegistrationNotifier;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.util.Set;

public class ImsRegistrationImpl extends ImsRegistrationImplBase
        implements IRegistrationNotifier {

    ImsRegistrationTracker mRegTracker;

    public ImsRegistrationImpl() {
    }

    public void setRegistrationTracker(ImsRegistrationTracker regTracker) {
        mRegTracker = regTracker;
    }

    @Override
    public void notifyRegistered(int regType, int networkType, @NonNull Set<String> featureTags) {
        logi("notifyRegistered: [" + RegistrationType.toString(regType) + "]" + featureTags);

        ImsRegistrationAttributes.Builder attrBuilder =
                new ImsRegistrationAttributes.Builder(networkType)
                .setFeatureTags(featureTags);

        if (regType == RegistrationType.EMERGENCY) {
            attrBuilder.setFlagRegistrationTypeEmergency();
        } else if (regType == RegistrationType.FAKE) {
            attrBuilder.setFlagRegistrationTypeEmergency();
            attrBuilder.setFlagVirtualRegistrationForEmergencyCall();
        }

        onRegistered(attrBuilder.build());
    }

    @Override
    public void notifyRegistering(int regType, int networkType, @NonNull Set<String> featureTags) {
        logi("notifyRegistering: [" + RegistrationType.toString(regType) + "]" + featureTags);

        ImsRegistrationAttributes.Builder attrBuilder =
                new ImsRegistrationAttributes.Builder(networkType)
                .setFeatureTags(featureTags);

        if (regType == RegistrationType.EMERGENCY) {
            attrBuilder.setFlagRegistrationTypeEmergency();
        } else if (regType == RegistrationType.FAKE) {
            attrBuilder.setFlagRegistrationTypeEmergency();
            attrBuilder.setFlagVirtualRegistrationForEmergencyCall();
        }

        onRegistering(attrBuilder.build());
    }

    @Override
    public void notifyDeregistered(
            int regType, int networkType, ReasonCode reason, String message, int dataFailCause) {
        logi("notifyDeregistered: [" + RegistrationType.toString(regType) + "]");

        if (regType == RegistrationType.NORMAL) {
            onDeregistered(getReasonInfo(reason, message, dataFailCause),
                    getSuggestedAction(reason),
                    networkType);
            return;
        }

        ImsRegistrationAttributes.Builder attrBuilder =
                new ImsRegistrationAttributes.Builder(networkType)
                        .setFlagRegistrationTypeEmergency();

        if (regType == RegistrationType.FAKE) {
            attrBuilder.setFlagVirtualRegistrationForEmergencyCall();
        }

        onDeregistered(getReasonInfo(reason, message, dataFailCause),
                RegistrationManager.SUGGESTED_ACTION_NONE, attrBuilder.build());
    }

    @Override
    public void notifyTechnologyChangeFailed(
            int regType, int networkType, ReasonCode reason, String message) {
        logi("notifyTechnologyChangeFailed: [" + RegistrationType.toString(regType) + "]");

        if (regType == RegistrationType.NORMAL) {
            onTechnologyChangeFailed(networkType,
                    getReasonInfo(reason, message, android.telephony.DataFailCause.NONE));
            return;
        }

        if (regType == RegistrationType.EMERGENCY) {
            ImsRegistrationAttributes regAttributes =
                    new ImsRegistrationAttributes.Builder(networkType)
                            .setFlagRegistrationTypeEmergency()
                            .build();

            onTechnologyChangeFailed(
                    getReasonInfo(reason, message, android.telephony.DataFailCause.NONE),
                    regAttributes);
        }
    }

    public void notifyAssociatedUriChanged(Uri[] uris) {
        onSubscriberAssociatedUriChanged(uris);
    }

    @VisibleForTesting
    protected ImsReasonInfo getReasonInfo(ReasonCode reason, String message, int dataFailCause) {
        Pair<Integer, Integer> pair = ReasonCodeMap.getImsReasonPair(reason, dataFailCause);

        return new ImsReasonInfo(pair.first, pair.second,
                (message != null && !message.trim().isEmpty()) ? message : null);
    }

    private int getSuggestedAction(ReasonCode reason) {
        logi("getSuggestedAction for reason:" + reason.toString());
        return switch (reason) {
            case PLMN_BLOCK -> RegistrationManager.SUGGESTED_ACTION_TRIGGER_PLMN_BLOCK;
            case PLMN_BLOCK_WITH_TIMEOUT,
                 PLMN_BLOCK_WITH_TIMEOUT_BY_VOPS_NOT_SUPPORTED,
                 PLMN_BLOCK_WITH_TIMEOUT_BY_SSAC_BARRED ->
                    RegistrationManager.SUGGESTED_ACTION_TRIGGER_PLMN_BLOCK_WITH_TIMEOUT;
            case RAT_BLOCK -> RegistrationManager.SUGGESTED_ACTION_TRIGGER_RAT_BLOCK;
            case CLEAR_RAT_BLOCKS -> RegistrationManager.SUGGESTED_ACTION_TRIGGER_CLEAR_RAT_BLOCKS;
            default -> RegistrationManager.SUGGESTED_ACTION_NONE;
        };
    }

    /**
     * Sip Delegate feature tag configuration is changed hence requested network registration for
     * all the sip delegates created by this ImsService.
     */
    @Override
    public void updateSipDelegateRegistration() {
        logi("updateSipDelegateRegistration");
        if (mRegTracker != null) {
            mRegTracker.updateSipDelegateRegistration();
        }
    }

    @Override
    public void triggerSipDelegateDeregistration() {
        logi("triggerSipDelegateDeregistration");
        if (mRegTracker != null) {
            mRegTracker.triggerSipDelegateDeregistration();
        }
    }

    @Override
    public void triggerFullNetworkRegistration(int sipCode, @Nullable String sipReason) {
        logi("triggerFullNetworkRegistration for sip reason:" + sipReason);

        if (mRegTracker != null) {
            mRegTracker.triggerFullNetworkRegistration(sipCode, sipReason);
        }
    }

    @Override
    public void triggerDeregistration(@ImsDeregistrationReason int reason) {
        if (mRegTracker != null) {
            mRegTracker.onDeregistrationTriggered(convertToAosReasonCause(reason));
        }
    }

    private int convertToAosReasonCause(int reason) {
        return switch (reason) {
            case REASON_SIM_REMOVED -> IAosRegistration.Cause.RADIO_SIM_REMOVED.getValue();
            case REASON_SIM_REFRESH -> IAosRegistration.Cause.RADIO_SIM_REFRESH.getValue();
            case REASON_ALLOWED_NETWORK_TYPES_CHANGED ->
                    IAosRegistration.Cause.RADIO_ALLOWED_NETWORK_TYPES_CHANGED.getValue();
            case REASON_NON_IMS_CAPABLE_NETWORK ->
                    IAosRegistration.Cause.NON_IMS_CAPABLE_NETWORK.getValue();
            case REASON_RADIO_POWER_OFF -> IAosRegistration.Cause.RADIO_POWER_OFF.getValue();
            case REASON_HANDOVER_FAILED -> IAosRegistration.Cause.HANDOVER_FAILED.getValue();
            case REASON_VOPS_NOT_SUPPORTED -> IAosRegistration.Cause.VOPS_NOT_SUPPORTED.getValue();
            default -> IAosRegistration.Cause.UNKNOWN.getValue();
        };
    }

    private static void logi(String s) {
        ImsLog.i("[ISIL] " + s);
    }
}
