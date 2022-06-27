package com.android.imsstack.imsservice.mmtel;

import android.annotation.NonNull;
import android.annotation.Nullable;
import android.net.Uri;
import android.telephony.DataFailCause;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsRegistrationAttributes;
import android.telephony.ims.stub.ImsRegistrationImplBase;

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
    public void notifyDeregistered(int reason) {
        onDeregistered(getReasonInfo(reason));
    }

    @Override
    public void notifyTechnologyChangeFailed(int networkType, int reason) {
        onTechnologyChangeFailed(networkType, getReasonInfo(reason));
    }

    public void notifyAssociatedUriChanged(Uri[] uris) {
        onSubscriberAssociatedUriChanged(uris);
    }

    private ImsReasonInfo getReasonInfo(int reason) {
        if (reason == DataFailCause.IWLAN_IKEV2_AUTH_FAILURE) {
            return new ImsReasonInfo(
                    ImsReasonInfo.CODE_EPDG_TUNNEL_ESTABLISH_FAILURE,
                    ImsReasonInfo.CODE_IKEV2_AUTH_FAILURE, null);
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

    public void setSipTransportBaseRegListener(
            ISipTransportBaseRegistrationListener sipTransportImplListener) {
        mSipTransportBaseRegListener = sipTransportImplListener;
    }

    private static void logi(String s) {
        ImsLog.i("[GII-IMPL] " + s);
    }
}
