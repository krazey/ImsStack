/*
 * Copyright (C) 2021 The Android Open Source Project
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

package com.android.imsstack.imsservice.sipcontroller;

import android.annotation.NonNull;
import android.content.Context;
import android.telephony.SubscriptionManager;
import android.telephony.ims.DelegateMessageCallback;
import android.telephony.ims.DelegateRegistrationState;
import android.telephony.ims.DelegateRequest;
import android.telephony.ims.DelegateStateCallback;
import android.telephony.ims.FeatureTagState;
import android.telephony.ims.SipDelegateConfiguration;
import android.telephony.ims.stub.SipDelegate;
import android.telephony.ims.stub.SipTransportImplBase;
import android.util.ArraySet;
import android.util.Log;

import com.android.imsstack.enabler.sipcontroller.impl.SipControllerAgent;
import com.android.imsstack.imsservice.mmtel.ImsRegistrationImpl;
import com.android.imsstack.imsservice.sipcontroller.remote.ISipTransportRemote;
import com.android.imsstack.imsservice.sipcontroller.remote.SipTransportRemoteListener;
import com.android.imsstack.imsservice.sipcontroller.sipdelegate.ActiveSipDelegateManager;
import com.android.imsstack.imsservice.sipcontroller.sipdelegate.SipDelegateImpl;
import com.android.imsstack.imsservice.sipcontroller.sipdelegate.SipDelegateRequestData;
import com.android.imsstack.util.MessageExecutor;

import java.util.Set;
import java.util.concurrent.Executor;

/**
 * The implementation of sip transport which manages creation and destruction of sip delegates.
 */

public class ImsSipTransport extends SipTransportImplBase {
    //Use this tag to print logs related to sip transport module
    public static final String LOG_TAG = "ImsSipTransport";

    //Below instance variables for each sip transport object which is created for each slot
    private final int mSlotId;
    private final Context mContext;
    //New message executor for framework callback interfaces
    private Executor mCallBackExecutor;
    //Ims base registration implementation
    private final ImsRegistrationImpl mImsRegistrationBaseImpl;
    //Object used to send request to IMS native
    public ISipTransportRemote mISipTransportRemote;
    /**
     * The active sip delegate manager for mSlotID.
     */
    private ActiveSipDelegateManager mActiveSipDelegateManager;
    //This is the latest configuration from IMS remote which will be shared to an applications
    SipDelegateConfiguration mSipTransportConfig;

    //RCS base registration handler to handle sip delegate call backs
    private SipTransportBaseRegistrationHandler mSipBaseTransportRegistrationHandler;
    /**
     * Feature tags associated with this sipTransport received from IMS core
     */
    private ArraySet<String> mSipTransportRegisteredTags = new ArraySet<>();
    private final ArraySet<FeatureTagState> mSipTransportDeregisteringTags = new ArraySet<>();
    private final ArraySet<FeatureTagState> mSipTransportDeregisteredTags = new ArraySet<>();
    private SipTransportRemoteListener mRemoteListener;

    //START constructor methods

    /**
     * Constructor for SipTransport
     *
     * @param slotId for which sip transport is required
     * @param context context
     * @param sipTransportRequestExecutor executor used by sip transport
     * @param callBackExecutor executor used for calling framework callback methods
     * @param registration base ims registration to get registration updates
     */
    public ImsSipTransport(int slotId, Context context, Executor sipTransportRequestExecutor,
            Executor callBackExecutor, ImsRegistrationImpl registration) {
        super(sipTransportRequestExecutor);
        mCallBackExecutor = callBackExecutor;
        mSlotId = slotId;
        mContext = context;
        mImsRegistrationBaseImpl = registration;
        initializeSipTransport();
    }
    //END constructor methods

    //START base implementation

    /**
     * Create sip delegate for an application.
     *
     * @param subscriptionId The subscription ID associated with the requested
     * {@link SipDelegate}.
     * @param request A SIP delegate request containing the parameters that the remote
     * RCS application wishes to use.
     * @param delegateStateCallback A callback back to the remote application to be used
     * to communicate state callbacks for the SipDelegate.
     * @param delegateMessageCallback A callback back to the remote application to be used
     * to send SIP messages to the
     */
    @Override
    public void createSipDelegate(int subscriptionId, @NonNull DelegateRequest request,
            @NonNull DelegateStateCallback delegateStateCallback,
            @NonNull DelegateMessageCallback delegateMessageCallback) {
        if (SubscriptionManager.isValidSubscriptionId(subscriptionId) == false) {
            throw new IllegalArgumentException("Invalid subId: " + subscriptionId);
        }
        if (request == null || delegateStateCallback == null || delegateMessageCallback == null) {
            throw new IllegalArgumentException("Null parameter passed");
        }
        createSipDelegateInternal(subscriptionId, request, delegateStateCallback,
                delegateMessageCallback);
    }

    /**
     * Destroy  sip delegate created for an application.
     *
     * @param delegate The delegate to be destroyed.
     * @param reason The reason the remote connection to this {@link SipDelegate} is being
     */
    @Override
    public void destroySipDelegate(@NonNull SipDelegate delegate, int reason) {
        if (delegate == null) {
            throw new IllegalArgumentException("Null parameter passed");
        }
        destroySipDelegateInternal(delegate, reason);
    }

    /**
     * Create SipTransport for specific slot id.
     *
     * @param slotId for which sip transport belongs to
     * @param context ims service context
     * @param callBackExecutor executor used for callback method execution
     * @param registration base registration to interact with sip transport
     * @return sip transport implementation for specified slot id.
     */
    public static ImsSipTransport createImsSipTransport(int slotId, Context context,
            Executor callBackExecutor, ImsRegistrationImpl registration) {
        return new ImsSipTransport(slotId, context, new MessageExecutor("ImsSipTransport"),
                callBackExecutor, registration);
    }

    /**
     * Update config for all the active sip delegates.
     */
    public void updateSipTransportConfigurationValues() {
        //Request from framework notify with latest configuration values.
        //The remote application will not be able to proceed sending SIP messages until after this
        //configuration is sent the first time.
        if (mActiveSipDelegateManager != null && mSipTransportConfig != null) {
            for (SipDelegateImpl sipDelegate :
                    mActiveSipDelegateManager.getActiveSipDelegateList()) {
                sipDelegate.updateSipDelegateConfig(mSipTransportConfig);
            }
        }
    }

    /**
     * Update the latest configuration value.
     */
    public void updateSipTransportConfig(SipDelegateConfiguration sipTransportConfig) {
        mSipTransportConfig = sipTransportConfig;
    }

    /**
     * Keep the status of each feature tag supported by IMS.
     * @param registrationState contains state of the IMS feature tags associated with a SipDelegate
     */
    public void updateSipTransportFeatureTagsStates(DelegateRegistrationState
            registrationState) {
        Log.i(LOG_TAG, "updatedSipTransportFeatureTagsStates");
        if (registrationState != null) {
            mSipTransportRegisteredTags.clear();
            mSipTransportRegisteredTags.addAll(registrationState.getRegisteredFeatureTags());

            mSipTransportDeregisteredTags.clear();
            mSipTransportDeregisteredTags.addAll(registrationState.getDeregisteredFeatureTags());

            mSipTransportDeregisteringTags.clear();
            mSipTransportDeregisteringTags.addAll(registrationState.getDeregisteringFeatureTags());
        } else {
            Log.d(LOG_TAG, "updatedSipTransportFeatureTagsStates: registrationState is null");
        }
    }

    /**
     * Get the active dialog list
     *
     * @return the active dialog list set
     */
    public Set<String> getActiveDialogList() {
        return mActiveSipDelegateManager.getAllSipDelegateActiveDialogIds();
    }

    /**
     * Get the ims core object which required to send commands to native
     *
     * @return ims code object to send sip transport requests to native
     */
    public ISipTransportRemote getISipTransportRemote() {
        return mISipTransportRemote;
    }

    /**
     * Return the sip delegate manager handling sip delegate belongs to this sip transport.
     *
     * @return active sip delegate manager of the sip transport.
     */
    public ActiveSipDelegateManager getActiveSipDelegateManager() {
        return mActiveSipDelegateManager;
    }

    /**
     * Get the sip delegate configuration associated with this sip transport
     *
     * @return latest configuration
     */
    public SipDelegateConfiguration getSipTransportConfig() {
        return mSipTransportConfig;
    }

    /**
     * Get the latest configuration version which required to send the sip messages.
     */
    public long getLatestConfigurationVersion() {
        long configVersion = -1;
        if (mSipTransportConfig != null) {
            configVersion = mSipTransportConfig.getVersion();
        }
        return configVersion;
    }

    /**
     * Get the registered feature tags of all the sip delegates
     *
     * @return the list of registered tags by this sip transport
     */
    public ArraySet<String> getSipTransportRegisteredTags() {
        return mSipTransportRegisteredTags;
    }

    /**
     * Initialize the sip transport layer
     */
    private void initializeSipTransport() {
        Log.i(LOG_TAG, "initializeSipTransport");
        mActiveSipDelegateManager = new ActiveSipDelegateManager(mSlotId);
        mSipBaseTransportRegistrationHandler =
                new SipTransportBaseRegistrationHandler(mSlotId);
        if (mImsRegistrationBaseImpl != null) {
            //Set the listener for Sip Transport registration related trigger
            mImsRegistrationBaseImpl.setSipTransportBaseRegListener(
                    mSipBaseTransportRegistrationHandler);
        } else {
            Log.w(LOG_TAG, "mImsRegistrationBaseImpl is null");
        }
        //SipTransportController initialization
        mISipTransportRemote = SipControllerAgent.getInstance(mSlotId);
        mRemoteListener = new SipTransportRemoteListener();
        mISipTransportRemote.setSipTransportListener(mRemoteListener);
    }

    /**
     * Create required sip delegate and notify caller with created sip delegate object
     *
     * @param subscriptionId sub id for which SipDelegate needs to be created
     * @param request Request containing rcs tags for which delegate to be created.
     * @param delegateStateCallback state call back
     * @param delegateMessageCallback message call back
     */
    private void createSipDelegateInternal(int subscriptionId, DelegateRequest request,
            DelegateStateCallback delegateStateCallback,
            DelegateMessageCallback delegateMessageCallback) {
        Log.i(LOG_TAG, "createSipDelegateInternal");
        // create SipDelegateRequestData object which holds request for this sipdelegate.
        SipDelegateRequestData delegateRequestData = new SipDelegateRequestData(subscriptionId,
                request.getFeatureTags(), delegateStateCallback, delegateMessageCallback);
        Set<FeatureTagState> deniedTags = mActiveSipDelegateManager.
                getDeniedTagListFromRequest(request.getFeatureTags());
        SipDelegateImpl sipDelegate = new SipDelegateImpl(delegateRequestData, mSlotId,
                mCallBackExecutor);
        mActiveSipDelegateManager.addActiveSipDelegate(sipDelegate);
        mCallBackExecutor.execute(() -> {
            delegateStateCallback.onCreated(sipDelegate, deniedTags);
        });
    }

    /**
     * Destroy the sip delegated
     *
     * @param delegate to be destroyed
     * @param reason for which sip delegate will be requested to destroy
     */
    private void destroySipDelegateInternal(SipDelegate delegate, int reason) {
        Log.i(LOG_TAG, "destroySipDelegateInternal");
        //Remove the sip delegate from the active sip delegate list.
        SipDelegateImpl destroySipDelegate =
                mActiveSipDelegateManager.getActiveSipDelegateImp(delegate);

        if (destroySipDelegate != null) {
            mActiveSipDelegateManager.destroySipDelegate(delegate);
            mCallBackExecutor.execute(() -> {
                destroySipDelegate.getDelegateRequestData().
                        getDelegateStateCallback().onDestroyed(reason);
            });
        } else {
            //Error : delegate connection requested for deletion is not exists.
            Log.w(LOG_TAG, "destroySipDelegateInternal: Delegate not found");
        }
    }
}
