/*
 * Copyright (C) 2024 The Android Open Source Project
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
package com.android.imsstack.its.imsservice.reg;

import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.os.RemoteException;
import android.telephony.AccessNetworkConstants;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsRegistrationAttributes;
import android.telephony.ims.RegistrationManager;
import android.telephony.ims.SipDetails;
import android.telephony.ims.aidl.IImsRegistration;
import android.telephony.ims.aidl.IImsRegistrationCallback;
import android.telephony.ims.feature.CapabilityChangeRequest;
import android.telephony.ims.feature.MmTelFeature;
import android.telephony.ims.stub.ImsRegistrationImplBase;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.android.imsstack.its.util.SingleLatch;
import com.android.imsstack.util.Log;

import java.util.List;

/**
 * IMS registration interface wrapper.
 */
public final class ImsRegistrationWrapper {
    private IImsRegistration mIImsRegistration;
    private RegistrationListener mRegistrationListener;
    private final RegistrationCallback mRegistrationCallback = new RegistrationCallback();
    private final Handler mHandler;
    private final SingleLatch mRegisteredLatch = new SingleLatch("ImsRegistered");
    private final SingleLatch mDeregisteredLatch = new SingleLatch("ImsDeregistered");
    private boolean mRegistered;

    public interface RegistrationListener {
        /**
         * Notifies when the IMS Provider is registered to the IMS network
         * with corresponding attributes.
         *
         * @param attributes The attributes associated with this IMS registration.
         */
        default void onRegistered(@NonNull ImsRegistrationAttributes attributes) {
        }

        /**
         * Notifies when the IMS Provider is trying to register the IMS network.
         *
         * @param attributes The attributes associated with this IMS registration.
         */
        default void onRegistering(@NonNull ImsRegistrationAttributes attributes) {
        }

        /**
         * Notifies when the IMS Provider is unregistered from the IMS network.
         *
         * @param info the {@link ImsReasonInfo} associated with why registration was disconnected.
         * @param suggestedAction the expected behavior of radio protocol stack.
         * @param imsRadioTech the network type on which IMS registration has failed.
         */
        default void onDeregistered(@NonNull ImsReasonInfo info,
                @RegistrationManager.SuggestedAction int suggestedAction,
                @ImsRegistrationImplBase.ImsRegistrationTech int imsRadioTech) {
        }

        /**
         * A failure has occurred when trying to handover registration to another technology type.
         *
         * @param imsTransportType The transport type that has failed to handover registration to.
         * @param info A {@link ImsReasonInfo} that identifies the reason for failure.
         */
        default void onTechnologyChangeFailed(
                @AccessNetworkConstants.TransportType int imsTransportType,
                @NonNull ImsReasonInfo info) {
        }

        /**
         * Returns a list of subscriber {@link Uri}s associated with this IMS subscription when
         * it changes. Per RFC3455, an associated URI is a URI that the service provider has
         * allocated to a user for their own usage. A user's phone number is typically one of the
         * associated URIs.
         *
         * @param uris new array of subscriber {@link Uri}s that are associated with this IMS
         *         subscription.
         */
        default void onSubscriberAssociatedUriChanged(@Nullable Uri[] uris) {
        }
    }

    /**
     * Used to enable and disable MMTEL. Sees MmTelFeature#changeEnabledCapabilities.
     */
    public static class CapabilityPair {
        private final int mCapability;
        private final int mTechnology;

        /**
         * Contains a MMTEL feature capability
         * {@link MmTelFeature.MmTelCapabilities.MmTelCapability}
         * along with an associated technology {@link ImsRegistrationImplBase.ImsRegistrationTech}
         */
        public CapabilityPair(@MmTelFeature.MmTelCapabilities.MmTelCapability int capability,
                @ImsRegistrationImplBase.ImsRegistrationTech int technology) {
            mCapability = capability;
            mTechnology = technology;
        }

        public int getCapability() {
            return mCapability;
        }

        public int getTechnology() {
            return mTechnology;
        }
    }

    /** Constructor. */
    public ImsRegistrationWrapper(@NonNull IImsRegistration registration) {
        mIImsRegistration = registration;
        mHandler = new Handler(Looper.myLooper());

        try {
            mIImsRegistration.addRegistrationCallback(mRegistrationCallback);
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /** Destroy. */
    public void destroy() {
        logi("destroy");

        mHandler.removeCallbacksAndMessages(null);

        try {
            mIImsRegistration.removeRegistrationCallback(mRegistrationCallback);
        } catch (RemoteException e) {
            loge(e.toString());
        }

        mIImsRegistration = null;
    }

    /** Sets the listener for getting the registration status. */
    public void setListener(@Nullable RegistrationListener listener) {
        mRegistrationListener = listener;
    }

    /**
     * Gets CapabilityChangeRequest to enable and disable MMTEL.
     * Sees ImsMmTelFeatureWrapper#changeCapabilitiesConfiguration.
     */
    public CapabilityChangeRequest getCapabilityChangeRequest(
            @Nullable List<CapabilityPair> enabledList,
            @Nullable List<CapabilityPair> disabledList) {
        CapabilityChangeRequest request = new CapabilityChangeRequest();

        if (enabledList != null) {
            for (CapabilityPair pair : enabledList) {
                request.addCapabilitiesToEnableForTech(pair.getCapability(), pair.getTechnology());
            }
        }

        if (disabledList != null) {
            for (CapabilityPair pair : disabledList) {
                request.addCapabilitiesToDisableForTech(pair.getCapability(), pair.getTechnology());
            }
        }

        return request;
    }

    /**
     * Requests IMS stack to perform graceful IMS deregistration before radio performing
     * network detach in the events of SIM remove, refresh or and so on.
     *
     * @param reason the reason why the deregistration is triggered.
     */
    public void triggerDeregistration(
            @ImsRegistrationImplBase.ImsDeregistrationReason int reason) {
        try {
            mIImsRegistration.triggerDeregistration(reason);
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /**
     * Checks whether IMS registration is successfully completed or not.
     *
     * @return {@code true} if IMS registration is successfully done, {@code false} otherwise.
     */
    public boolean isRegistered() {
        return mRegistered;
    }

    /**
     * Waits for IMS registered.
     */
    public void waitForRegistered() {
        mRegisteredLatch.await(SingleLatch.LONG_TIMEOUT_MS);
    }

    /**
     * Waits for IMS deregistered.
     */
    public void waitForDeregistered() {
        mDeregisteredLatch.await();
    }

    private void invokeOnRegistered(@NonNull ImsRegistrationAttributes attributes) {
        mHandler.post(() -> {
            boolean registered = mRegistered;
            mRegistered = true;
            if (mRegistrationListener != null) {
                mRegistrationListener.onRegistered(attributes);
            }
            if (!registered) {
                mRegisteredLatch.countDown();
                mDeregisteredLatch.init();
            }
        });
    }

    private void invokeOnRegistering(@NonNull ImsRegistrationAttributes attributes) {
        mHandler.post(() -> {
            if (mRegistrationListener != null) {
                mRegistrationListener.onRegistering(attributes);
            }
        });
    }

    private void invokeOnDeregistered(@NonNull ImsReasonInfo info,
            @RegistrationManager.SuggestedAction int suggestedAction,
            @ImsRegistrationImplBase.ImsRegistrationTech int imsRadioTech) {
        mHandler.post(() -> {
            boolean registered = mRegistered;
            mRegistered = false;
            if (mRegistrationListener != null) {
                mRegistrationListener.onDeregistered(info, suggestedAction, imsRadioTech);
            }
            if (registered) {
                mDeregisteredLatch.countDown();
                mRegisteredLatch.init();
            }
        });
    }

    private void invokeOnTechnologyChangeFailed(
            @AccessNetworkConstants.TransportType int imsTransportType,
            @NonNull ImsReasonInfo info) {
        mHandler.post(() -> {
            if (mRegistrationListener != null) {
                mRegistrationListener.onTechnologyChangeFailed(imsTransportType, info);
            }
        });
    }

    private void invokeOnSubscriberAssociatedUriChanged(@Nullable Uri[] uris) {
        mHandler.post(() -> {
            if (mRegistrationListener != null) {
                mRegistrationListener.onSubscriberAssociatedUriChanged(uris);
            }
        });
    }

    private static void logi(String s) {
        Log.i(Log.TAG, "ImsRegistrationWrapper: " + s);
    }

    private static void loge(String s) {
        Log.e(Log.TAG, "ImsRegistrationWrapper: " + s);
    }

    public class RegistrationCallback extends IImsRegistrationCallback.Stub {
        @Override
        public void onRegistered(@NonNull ImsRegistrationAttributes attributes) {
            invokeOnRegistered(attributes);
        }

        @Override
        public void onRegistering(@NonNull ImsRegistrationAttributes attributes) {
            invokeOnRegistering(attributes);
        }

        @Override
        public void onDeregistered(@NonNull ImsReasonInfo info,
                @RegistrationManager.SuggestedAction int suggestedAction,
                @ImsRegistrationImplBase.ImsRegistrationTech int imsRadioTech) {
            invokeOnDeregistered(info, suggestedAction, imsRadioTech);
        }

        @Override
        public void onDeregisteredWithDetails(ImsReasonInfo info,
                @RegistrationManager.SuggestedAction int suggestedAction,
                @ImsRegistrationImplBase.ImsRegistrationTech int imsRadioTech,
                @NonNull SipDetails details) {
            invokeOnDeregistered(info, suggestedAction, imsRadioTech);
        }

        @Override
        public void onTechnologyChangeFailed(
                @AccessNetworkConstants.TransportType int imsTransportType,
                @NonNull ImsReasonInfo info) {
            invokeOnTechnologyChangeFailed(imsTransportType, info);
        }

        @Override
        public void onSubscriberAssociatedUriChanged(@Nullable Uri[] uris) {
            invokeOnSubscriberAssociatedUriChanged(uris);
        }
    }
}
