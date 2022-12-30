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
package com.android.imsstack.imsservice.sipcontroller.sipdelegate;

import android.telephony.ims.FeatureTagState;
import android.telephony.ims.SipDelegateConfiguration;
import android.telephony.ims.SipDelegateManager;
import android.telephony.ims.stub.SipDelegate;
import android.util.Log;

import com.android.imsstack.imsservice.mmtel.ImsServiceManager;
import com.android.imsstack.imsservice.mmtel.ImsServiceRecord;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Set;

/**
 * This class will manage the active sip delegates {@link SipDelegateImpl}created by IMSService.
 * e.g. removing, updating feature tags, registration changes etc.
 */
public class ActiveSipDelegateManager {
    private static final String LOG_TAG = "ActiveSipDelegateManager";
    /**
     * The slotid for which this sip delegate manager is assocaiated with to manage sip delegates.
     */
    private final int mSlotId;
    /**
     * This hods list of all {@link SipDelegateImpl}'s created for applications.
     */
    private final ArrayList<SipDelegateImpl> mActiveSipDelegateList = new ArrayList<>();

    /**
     * Create the sip delegate manager for specific slot.
     */
    public ActiveSipDelegateManager(int slotId) {
        mSlotId = slotId;
    }

    /**
     * Get the all the active sip delegate.
     *
     * @return the {@link SipDelegateImpl} list.
     */
    public List<SipDelegateImpl> getActiveSipDelegateList() {
        return mActiveSipDelegateList;
    }

    /**
     * This holds the call id and mapping sip delegate.
     */
    private final LinkedHashMap<String, SipDelegateImpl> mCallIdMappingSipDelegate =
            new LinkedHashMap<>();

    /**
     * Add all active sip delegate created
     *
     * @param sipDelegate delegate objected to be managed by delegate manager.
     */
    public synchronized void addActiveSipDelegate(SipDelegateImpl sipDelegate) {
        Log.i(LOG_TAG, "addActiveSipDelegate:" + sipDelegate);
        mActiveSipDelegateList.add(sipDelegate);
        updateConfiguration(sipDelegate);
    }

    /**
     * update Configuration for sip delegate
     * @param sipDelegate delegate objected to be managed by delegate manager.
     */
    public void updateConfiguration(SipDelegateImpl sipDelegate) {
        ImsServiceRecord serviceRecord = ImsServiceManager.getServiceRecord(mSlotId);
        if (serviceRecord != null && sipDelegate != null) {
            SipDelegateConfiguration configuration =
                    serviceRecord.getSipTransport().getSipTransportConfig();
            if (configuration != null) {
                sipDelegate.updateSipDelegateConfig(configuration);
            } else {
                Log.e(LOG_TAG, "updateConfiguration is null " + configuration);
            }
        }
        //TODO :  onConfigurationChanged should be called as soon as the
        //   {@link SipDelegate} has access to these configuration parameters
    }

    /**
     * Get the active sip delegate implementation
     *
     * @return return active sip delegate
     */
    public synchronized SipDelegateImpl getActiveSipDelegateImp(SipDelegate sipDelegate) {
        Log.i(LOG_TAG, "getActiveSipDelegateImp");
        if (mActiveSipDelegateList.contains(sipDelegate)) {
            return mActiveSipDelegateList.get(mActiveSipDelegateList.indexOf(sipDelegate));
        }
        return null;
    }

    /**
     * Destroy the active sip delegate
     *
     * @param sipDelegate to be destroyed
     * @return true if removal success else false.
     */
    public synchronized boolean destroySipDelegate(SipDelegate sipDelegate) {
        Log.i(LOG_TAG, "destroySipDelegate");
        if (mActiveSipDelegateList.contains(sipDelegate)) {
            return mActiveSipDelegateList.remove(mActiveSipDelegateList.indexOf(sipDelegate))
                    != null;
        } else {
            Log.e(LOG_TAG, "destroySipDelegate object not found");
        }
        //TODO Release the resources if there is no SipDelegate release()
        return false;
    }

    /**
     * Returns the denied feature tag list
     */
    public Set<FeatureTagState> getDeniedTagListFromRequest(Set<String> requestTags) {
        Log.i(LOG_TAG, "getDeniedTagListFromRequest");
        //TODO : This API currently consider only DENIED_REASON_IN_USE_BY_ANOTHER_DELEGATE reason
        // check how to handle all the denied tags errors.
        Set<FeatureTagState> deniedTagList = new HashSet<>();
        Set<String> allSupportedTags = getAllSipDelegatSupportedFeatureTags();
        for (String featureTag : requestTags) {
            if (allSupportedTags.contains(featureTag)) {
                deniedTagList.add(new FeatureTagState(featureTag,
                        SipDelegateManager.DENIED_REASON_IN_USE_BY_ANOTHER_DELEGATE));
            }
        }
        return deniedTagList;
    }

    /**
     * Get all the delegate supported feature tags
     *
     * @return total list of feature tags supported by sip delegates
     */
    public Set<String> getAllSipDelegatSupportedFeatureTags() {
        Log.i(LOG_TAG, "getAllSipDelegatSupportedFeatureTags");
        Set<String> allSupportedTags = new HashSet<>();
        for (SipDelegateImpl sipDelegate : getActiveSipDelegateList()) {
            Log.i(LOG_TAG, "supported for feature tag:" +
                    sipDelegate.getSupportedFeatureTags());
            if (sipDelegate.getSupportedFeatureTags() != null) {
                allSupportedTags.addAll(sipDelegate.getSupportedFeatureTags());
            }
        }
        Log.i(LOG_TAG, "allSupportedTags:" + allSupportedTags);
        return allSupportedTags;
    }

    /**
     * Add the sip delegate to mapping callId. This can be used to send the message to respective
     * delegate object.
     */
    public synchronized void addActiveSipDelegate(String callId, SipDelegateImpl sipDelegate) {
        Log.i(LOG_TAG, "addActiveSipDelegate");
        mCallIdMappingSipDelegate.put(callId, sipDelegate);
    }

    /**
     * Add the sip delegate to mapping callId. This can be used to send the message to respective
     * delegate object.
     */
    public synchronized void removeActiveSipDelegate(String callId) {
        Log.i(LOG_TAG, "removeActiveSipDelegate");
        if (mCallIdMappingSipDelegate != null) {
            mCallIdMappingSipDelegate.remove(callId);
        }
    }

    /**
     * This API can be used to get the sipDelegate for the mapping callID.
     *
     * @return the delegate object mapping with callid
     */
    public SipDelegateImpl getSipDelegateForCallId(String callId) {
        Log.i(LOG_TAG, "getSipDelegateForCallId for callid:" + callId);
        return mCallIdMappingSipDelegate.get(callId);
    }

    /**
     * Get the sip delegate which can handle this feature tags
     */
    public SipDelegateImpl getSipDelegateForSupportedFeatureTag(Set<String>
            featureTagsFromAcceptContact) {
        if (featureTagsFromAcceptContact == null) {
            Log.i(LOG_TAG, "getSipDelegateForSupportedFeatureTag: "
                    + "Accept-Contact not contains feature tags");
            return null;
        }
        Log.i(LOG_TAG, "getSipDelegateForSupportedFeatureTag for feature tag:" +
                featureTagsFromAcceptContact.toString());

        for (SipDelegateImpl sipDelegate : getActiveSipDelegateList()) {
            Log.i(LOG_TAG, "supported for feature tag:" +
                    sipDelegate.getSupportedFeatureTags());
            if (sipDelegate.getSupportedFeatureTags() != null
                    && sipDelegate.getSupportedFeatureTags().stream()
                            .anyMatch(featureTagsFromAcceptContact::contains)) {
                //Found the SipDelegate Handling this feature tag:
                Log.i(LOG_TAG, "Found the sip delegate which handles feature tags"
                        + sipDelegate);
                return sipDelegate;
            }
        }
        return null;
    }

    /**
     * This API can be used to get the sipDelegate for the mapping callID.
     *
     * @return the delegate object mapping with callid
     */
    public SipDelegateImpl getSipDelegateForTransactionId(String transactionId) {
        Log.i(LOG_TAG, "getSipDelegateForTransactionId for transactionId:" + transactionId);
        for (SipDelegateImpl sipDelegate : getActiveSipDelegateList()) {
            if (sipDelegate.getViaTransactionIds() != null
                    && sipDelegate.getViaTransactionIds().contains(transactionId)) {
                //Found the SipDelegate Handling transactionId
                Log.i(LOG_TAG, "Found the sip delegate which handles transactionId"
                        + sipDelegate);
                return sipDelegate;
            }
        }
        return null;
    }

    /**
     * Get all ongoing session callid list
     *
     * @return active dialog list
     */
    public Set<String> getAllSipDelegateActiveDialogIds() {
        Log.i(LOG_TAG, "getAllSipDelegateActiveDialogIds");
        Set<String> onGoingSessions = new HashSet<>();
        for (SipDelegateImpl sipDelegate : getActiveSipDelegateList()) {
            Log.i(LOG_TAG, "ongoing sessions:" + sipDelegate.getOngoingsipsessions());
            if (sipDelegate.getOngoingsipsessions() != null) {
                sipDelegate.getOngoingsipsessions().addAll(onGoingSessions);
            }
        }
        Log.i(LOG_TAG, "onGoingSessions:" + onGoingSessions);
        return onGoingSessions;
    }

     /** when All sip delegate is destroyed/terminated
     * Notify stack to Release resources
     */
    private void release() {
        if (mActiveSipDelegateList.isEmpty()) {
            ImsServiceRecord record = ImsServiceManager.getServiceRecord(mSlotId);
            record.getSipTransport().getISipTransportRemote().release(mSlotId);
        }
    }

}
