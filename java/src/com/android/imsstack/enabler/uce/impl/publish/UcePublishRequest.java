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

package com.android.imsstack.enabler.uce.impl.publish;

import android.os.Parcel;
import android.text.TextUtils;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.PreferenceInterface;
import com.android.imsstack.enabler.uce.impl.define.UceConstant;
import com.android.imsstack.enabler.uce.impl.define.UceMessage;
import com.android.imsstack.enabler.uce.impl.jni.UceJNI;
import com.android.imsstack.enabler.uce.interf.PublishResponse;
import com.android.imsstack.enabler.uce.interf.UceApiConstant;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;
 /**
 * The UcePublishRequest will be handled the request related to the PUBLISH.
 * This class will be created per each request related to the PUBLISH.
 */
public class UcePublishRequest {
    private final int mSlotId;
    private final int mKey;
    private final PublishResponse callback;
    private final boolean mIsUseExpiredEtag;

    private String mPidfXml;
    private int mExtended;
    private long mCapability;
    private UceJNI mUceJNI;
    private String mEtag;

    private final PreferenceInterface mPf;

    public UcePublishRequest(PublishResponse cb, int slotId, int key, boolean useExpiredEtag) {
        this(cb, slotId, key, useExpiredEtag, UceJNI.getInstance(), "",
                AgentFactory.getInstance().getAgent(PreferenceInterface.class));
    }

    @VisibleForTesting
    public UcePublishRequest(PublishResponse cb, int slotId, int key, boolean useExpiredEtag,
            UceJNI jni, String eTag, PreferenceInterface pf) {
        mKey = key;
        callback = cb;
        mSlotId = slotId;
        mUceJNI = jni;
        mEtag = eTag;
        mPf = pf;
        mIsUseExpiredEtag = useExpiredEtag;
        if (mIsUseExpiredEtag) {
            if (mPf != null) {
                mEtag = mPf.getString(UceConstant.PREFERENCE_ETAG, slotId);
            }
        }
    }

    /**
     * Set up data for sending publish requests.
     * @param pidfXml The XML PIDF document containing the capabilities of this device to be sent
     * to the carrier’s presence server.
     * @param bAvailability Whether the time value is extended when the expire header is set.
     * @param capability The value converted from pidf xml to long. When the publish is successful,
     *                   this is set to the latest capability.
     */
    public void setRequestInfo(String pidfXml, boolean bAvailability, long capability) {
        ImsLog.d(mSlotId, "setRequestInfo:bAvailability=" + bAvailability
                + ", capability=" + capability);
        mPidfXml = pidfXml;
        mExtended = bAvailability == true ? 1 : 0;
        mCapability = capability;
    }

    /**
     * The capabilities of this device have been updated and should be published to the network.
     * @return true if the request is successfully processed.
     */
    public boolean sendRequest() {
        if (TextUtils.isEmpty(mPidfXml)) {
            ImsLog.e(mSlotId, "pidfXml is empty");
            informCommandError(UceApiConstant.COMMAND_CODE_INVALID_PARAM);
            return false;
        }

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(UceMessage.UCE_SEND_PUBLISH_CMD);
        parcel.writeInt(mKey);
        parcel.writeString(mPidfXml);
        parcel.writeInt(mExtended);
        parcel.writeLong(mCapability);
        if (TextUtils.isEmpty(mEtag)) {
            parcel.writeInt(0);
        } else {
            parcel.writeInt(1);
            parcel.writeString(mEtag);
        }

        mUceJNI.sendMessage(mSlotId, parcel);
        return true;
    }

    /**
     * Handles responses to PUBLISH requests.
     * @param responseCode the received sip response code.
     * @param reason the received sip reason value.
     * @param reasonHdrCause the received cause value of sip reason header
     * @param reasonHdrText the received text value of sip reason header
     * @param eTag the received sip etag value.
     */
    public void informNetworkResponse(int responseCode, String reason,
        int reasonHdrCause, String reasonHdrText, String eTag) {
        ImsLog.d(mSlotId, "informNetworkResponse:responseCode=" + responseCode
                + ", reasonHdrCause=" + reasonHdrCause);
        try {
            if (reasonHdrCause <= 0) {
                callback.onNetworkResponse(responseCode, reason);
            } else {
                callback.onNetworkResponse(responseCode, reason, reasonHdrCause, reasonHdrText);
            }
        } catch (Exception e) {
            ImsLog.e(mSlotId, "Exception:" + e.toString());
        }
        if (!TextUtils.isEmpty(eTag)) {
            if (mPf != null) {
                mPf.putString(UceConstant.PREFERENCE_ETAG, eTag, mSlotId);
            }
        }
    }

    /**
     * Send command error regarding this request.
     * @param code the command error code. it is one of the {@link UceApiConstant}.
     */
    public void informCommandError(int code) {
        ImsLog.d(mSlotId, "informCommandError:code=" + code);
        try {
            callback.onCommandError(code);
        } catch (Exception e) {
            ImsLog.e(mSlotId, "Exception:" + e.toString());
        }
    }

    /**
     * Get the my request key.
     * @return The requested key.
     */
    public int getKey() {
        return mKey;
    }

     /**
      * Get the current eTag
      * @return the current etag value
      */
    public String getEtag() {
        return mEtag;
    }
}
