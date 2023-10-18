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

package com.android.imsstack.imsservice.sipcontroller;

import android.util.Log;

import com.android.imsstack.imsservice.mmtel.ImsServiceManager;
import com.android.imsstack.imsservice.mmtel.ImsServiceRecord;
import com.android.imsstack.imsservice.sipcontroller.remote.ISipTransportRemote;
import com.android.imsstack.imsservice.sipcontroller.sipdelegate.ActiveSipDelegateManager;

/**
 * The{@link ISipTransportBaseRegistrationListener} interface implementation is to update
 * the ims registration with new values for all sip delegates
 */
public class SipTransportBaseRegistrationHandler implements ISipTransportBaseRegistrationListener {
    private final int mSlotId;

    public SipTransportBaseRegistrationHandler(int slotId) {
        mSlotId = slotId;
    }

    /**
     * Called when feature tag registration is required and also when feature tag configuration
     * is changed.
     */
    @Override
    public void triggerSipTransportDelegateRegistration() {
        Log.i(ImsSipTransport.LOG_TAG, "triggerSipTransportNetworkRegistration");
        ImsServiceRecord serviceRecord = ImsServiceManager.getServiceRecord(mSlotId);
        ActiveSipDelegateManager activeDelegateManager;
        ISipTransportRemote remoteTransport;
        if (serviceRecord != null) {
            activeDelegateManager = serviceRecord.getSipTransport().getActiveSipDelegateManager();
            remoteTransport = serviceRecord.getSipTransport().getISipTransportRemote();
            if (remoteTransport != null) {
                remoteTransport.updateSipDelegateRegistration(
                        activeDelegateManager.getAllSipDelegatSupportedFeatureTags());
            } else {
                Log.e(ImsSipTransport.LOG_TAG, "remoteTransport is null");
            }
        } else {
            Log.e(ImsSipTransport.LOG_TAG, "serviceRecord is null");
        }
    }

    /**
     * This method will request de-registration for all sip delegates belongs to this sip transport
     */
    @Override
    public void triggerSipTransportDelegateDeregistration() {
        Log.i(ImsSipTransport.LOG_TAG, "triggerSipTransportDelegateDeregistration");
        ImsServiceRecord serviceRecord = ImsServiceManager.getServiceRecord(mSlotId);
        ISipTransportRemote remoteTransport;
        if (serviceRecord != null && serviceRecord.getSipTransport().getActiveDialogList() != null
                && serviceRecord.getSipTransport().getActiveDialogList().isEmpty()) {
            remoteTransport = serviceRecord.getSipTransport().getISipTransportRemote();
            if (remoteTransport != null) {
                remoteTransport.triggerSipDelegateDeRegistration();
            } else {
                Log.e(ImsSipTransport.LOG_TAG, "remoteTransport is null");
            }
        } else {
            Log.e(ImsSipTransport.LOG_TAG, "active dialogs may be present");
        }
    }
}
