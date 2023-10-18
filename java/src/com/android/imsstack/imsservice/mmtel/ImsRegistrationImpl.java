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
import android.telephony.DataFailCause;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsRegistrationAttributes;
import android.telephony.ims.RegistrationManager;
import android.telephony.ims.stub.ImsRegistrationImplBase;

import com.android.imsstack.enabler.aos.IAosRegistration;
import com.android.imsstack.enabler.aos.IAosRegistrationListener;
import com.android.imsstack.imsservice.mmtel.reg.IRegistrationNotifier;
import com.android.imsstack.imsservice.sipcontroller.ISipTransportBaseRegistrationListener;
import com.android.imsstack.util.ImsLog;

import java.util.Set;

public final class ImsRegistrationImpl extends ImsRegistrationImplBase
        implements IRegistrationNotifier {

    ImsRegistrationTracker mRegTracker;

    private ISipTransportBaseRegistrationListener mSipTransportBaseRegListener;

    public ImsRegistrationImpl() {
    }

    public void setRegistrationTracker(ImsRegistrationTracker regTracker) {
        mRegTracker = regTracker;
    }

    @Override
    public void notifyRegistered(int networkType, @NonNull Set<String> featureTags) {
        logi("notifyRegistered: " + featureTags);

        ImsRegistrationAttributes regAttributes =
                new ImsRegistrationAttributes.Builder(networkType)
                .setFeatureTags(featureTags)
                .build();
        onRegistered(regAttributes);
    }

    @Override
    public void notifyRegistering(int networkType, @NonNull Set<String> featureTags) {
        logi("notifyRegistering: " + featureTags);

        ImsRegistrationAttributes regAttributes =
                new ImsRegistrationAttributes.Builder(networkType)
                .setFeatureTags(featureTags)
                .build();
        onRegistering(regAttributes);
    }

    @Override
    public void notifyDeregistered(int networkType, int reason, String message) {
        int suggestedAction = RegistrationManager.SUGGESTED_ACTION_NONE;

        if (reason == IAosRegistrationListener.ReasonCode.CODE_PLMN_BLOCK) {
            suggestedAction = RegistrationManager.SUGGESTED_ACTION_TRIGGER_PLMN_BLOCK;
        } else if (reason == IAosRegistrationListener.ReasonCode.CODE_PLMN_BLOCK_WITH_TIMEOUT) {
            suggestedAction = RegistrationManager.SUGGESTED_ACTION_TRIGGER_PLMN_BLOCK_WITH_TIMEOUT;
        }
        onDeregistered(getReasonInfo(reason, message), suggestedAction, networkType);
    }

    @Override
    public void notifyTechnologyChangeFailed(int networkType, int reason, String message) {
        onTechnologyChangeFailed(networkType, getReasonInfo(reason, message));
    }

    public void notifyAssociatedUriChanged(Uri[] uris) {
        onSubscriberAssociatedUriChanged(uris);
    }

    private ImsReasonInfo getReasonInfo(int reason, String message) {
        if (reason == DataFailCause.IWLAN_IKEV2_AUTH_FAILURE) {
            return new ImsReasonInfo(
                    ImsReasonInfo.CODE_EPDG_TUNNEL_ESTABLISH_FAILURE,
                    ImsReasonInfo.CODE_IKEV2_AUTH_FAILURE, null);
        } else if (message != null && !message.trim().equals("")) {
            return new ImsReasonInfo(
                    ImsReasonInfo.CODE_REGISTRATION_ERROR,
                    ImsReasonInfo.CODE_UNSPECIFIED, message);
        } else {
            return new ImsReasonInfo(
                    ImsReasonInfo.CODE_REGISTRATION_ERROR,
                    ImsReasonInfo.CODE_UNSPECIFIED, null);
        }
    }

    /**
     * Sip Delegate feature tag configuration is changed hence requested network registration for
     * all the sip delegates created by this ImsService.
     */
    @Override
    public void updateSipDelegateRegistration() {
        logi("updateSipDelegateRegistration");
        //TODO communicate with sip transport for requesting the network registration
        if (mSipTransportBaseRegListener != null) {
            mSipTransportBaseRegListener.triggerSipTransportDelegateRegistration();
        } else {
            logi("updateSipDelegateRegistration: mSipTransportBaseRegListener is not set");
        }

        if (mRegTracker != null) {
            mRegTracker.updateSipDelegateRegistration();
        }
    }

    @Override
    public void triggerSipDelegateDeregistration() {
        logi("triggerSipDelegateDeregistration");
        if (mSipTransportBaseRegListener != null) {
            mSipTransportBaseRegListener.triggerSipTransportDelegateDeregistration();
        } else {
            logi("triggerSipDelegateDeregistration: mSipTransportBaseRegListener is not set");
        }

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

    public void setSipTransportBaseRegListener(
            ISipTransportBaseRegistrationListener sipTransportImplListener) {
        mSipTransportBaseRegListener = sipTransportImplListener;
    }

    private int convertToAosReasonCause(int reason) {
        switch (reason) {
            case REASON_SIM_REMOVED:
                return IAosRegistration.Cause.RADIO_SIM_REMOVED;
            case REASON_SIM_REFRESH:
                return IAosRegistration.Cause.RADIO_SIM_REFRESH;
            case REASON_ALLOWED_NETWORK_TYPES_CHANGED:
                return IAosRegistration.Cause.RADIO_ALLOWED_NETWORK_TYPES_CHANGED;
            case REASON_NON_IMS_CAPABLE_NETWORK:
                return IAosRegistration.Cause.NON_IMS_CAPABLE_NETWORK;
            case REASON_RADIO_POWER_OFF:
                return IAosRegistration.Cause.RADIO_POWER_OFF;
            case REASON_HANDOVER_FAILED:
                return IAosRegistration.Cause.HANDOVER_FAILED;
            case REASON_VOPS_NOT_SUPPORTED:
                return IAosRegistration.Cause.VOPS_NOT_SUPPORTED;
            default:
                return IAosRegistration.Cause.UNKNOWN;
        }
    }

    private static void logi(String s) {
        ImsLog.i("[GII-IMPL] " + s);
    }
}
