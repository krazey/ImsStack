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

package com.android.imsstack.enabler.uce.impl.options;

import android.os.Parcel;
import android.text.TextUtils;

import com.android.imsstack.enabler.uce.impl.define.UceMessage;
import com.android.imsstack.enabler.uce.impl.jni.UceJNI;
import com.android.imsstack.enabler.uce.impl.utils.UceUtils;
import com.android.imsstack.enabler.uce.interf.OptionsResponse;
import com.android.imsstack.enabler.uce.interf.UceApiConstant;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;

/**
 * The UceOptionsRequest will be handled the request related to the OPTIONS.
 * This class will be created per each request related to the OPTIONS.
 */
public class UceOptionsRequest {
    private final int mSlotId;
    private final int mKey;
    private final OptionsResponse callback;
    private UceJNI mUceJNI;

    public UceOptionsRequest(OptionsResponse cb, int slotId, int key) {
        this (cb, slotId, key, UceJNI.getInstance());
    }

    @VisibleForTesting
    public UceOptionsRequest(OptionsResponse cb, int slotId, int key, UceJNI jni) {
        mSlotId = slotId;
        mKey = key;
        callback = cb;
        mUceJNI = jni;
    }

    /**
     * Push one's own capabilities to a remote user via the SIP OPTIONS presence exchange mechanism
     * in order to receive the capabilities of the remote user in response.
     * @param remoteUri The URI of the remote user that we wish to get the capabilities of.
     * @param myCapabilities The capabilities of this device to send to the remote user.
     * @return true if the request was handle successful.
     */
    public boolean sendRequest(String remoteUri, Set<String> myCapabilities) {
        ImsLog.i(mSlotId, "");
        if (TextUtils.isEmpty(remoteUri)) {
            ImsLog.e(mSlotId, "remoteUri is empty");
            informCommandError(UceApiConstant.COMMAND_CODE_INVALID_PARAM);
            return false;
        }
        long capabilities = UceUtils.getCapabilities(myCapabilities);
        if (capabilities == 0) {
            ImsLog.e(mSlotId, "capabilities is empty");
            informCommandError(UceApiConstant.COMMAND_CODE_INVALID_PARAM);
            return false;
        }
        Parcel parcel = Parcel.obtain();
        parcel.writeInt(UceMessage.UCE_SEND_OPTIONS_CMD);
        parcel.writeInt(mKey);
        parcel.writeString(remoteUri);
        parcel.writeLong(capabilities);
        mUceJNI.sendMessage(mSlotId, parcel);
        return true;
    }

    /**
     * Handles responses to OPTIONS requests.
     * @param responseCode The SIP response code that was sent by the network in response
     * to the request.
     * @param reason The optional SIP response reason sent by the network.
     * @param capabilities the contact's UCE capabilities associated with the
     * capability request.
     */
    public void informNetworkResponse(int responseCode, String reason, long capabilities) {
        ImsLog.d(mSlotId, "informNetworkResponse:responseCode=" + responseCode
                + ", reason:" + reason);
        Set<String> caps = UceUtils.getFeatureTags(capabilities);
        List<String> theirCaps = new ArrayList<>(caps);
        try {
            callback.onNetworkResponse(responseCode, reason, theirCaps);
        } catch (Exception e) {
            ImsLog.e(mSlotId, "Exception:" + e.toString());
        }
    }

    /**
     * Send command error regarding this request.
     * @param code The reason why the associated command has failed.
     */
    public void informCommandError(int code) {
        ImsLog.d(mSlotId, "informCommandError:code=" + code);
        try {
            callback.onCommandError(code);
        } catch (Exception e) {
            ImsLog.e(mSlotId, "Exception:" + e.toString());
        }
    }
}
