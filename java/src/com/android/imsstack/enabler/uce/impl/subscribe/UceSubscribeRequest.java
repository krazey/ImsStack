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

package com.android.imsstack.enabler.uce.impl.subscribe;

import android.net.Uri;
import android.os.Parcel;
import android.util.Pair;

import com.android.imsstack.enabler.uce.impl.define.UceMessage;
import com.android.imsstack.enabler.uce.impl.jni.UceJNI;
import com.android.imsstack.enabler.uce.interf.SubscribeResponse;
import com.android.imsstack.enabler.uce.interf.UceApiConstant;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.TimeUnit;

public class UceSubscribeRequest {
    private final int mSlotId;
    private final int mKey;
    private final SubscribeResponse callback;
    private final UceJNI mUceJNI;

    public UceSubscribeRequest(SubscribeResponse cb, int slotId, int key) {
        this(cb, slotId, key, UceJNI.getInstance());
    }

    @VisibleForTesting
    public UceSubscribeRequest(SubscribeResponse cb, int slotId, int key, UceJNI jni) {
        mKey = key;
        callback = cb;
        mSlotId = slotId;
        mUceJNI = jni;
        ImsLog.d("key:" + mKey);
    }

    /**
     * Send the SIP SUBSCRIBE method to get the other party's capabilities.
     * @param remoteUris A {@link ArrayList} of the {@link String}s that the framework is
     *                  requesting the UCE capabilities for.
     */
    public boolean sendRequest(ArrayList<String> remoteUris) {
        ImsLog.i("");
        if (remoteUris == null || remoteUris.size() == 0) {
            ImsLog.e("remoteUris is empty");
            informCommandError(UceApiConstant.COMMAND_CODE_INVALID_PARAM);
            return false;
        }
        Parcel parcel = Parcel.obtain();

        int size = remoteUris.size();
        if (size == 1) {
            parcel.writeInt(UceMessage.UCE_SEND_SINGLE_SUBSCRIBE_CMD);
        } else {
            parcel.writeInt(UceMessage.UCE_SEND_LIST_SUBSCRIBE_CMD);
        }
        parcel.writeInt(mKey);
        parcel.writeInt(size);
        for (int i = 0; i < size; i++) {
            parcel.writeString(remoteUris.get(i));
        }
        mUceJNI.sendMessage(mSlotId, parcel);
        return true;
    }

    /**
     * Handles responses to SUBSCRIBE requests.
     * @param responseCode the received sip response code.
     * @param reason the received sip reason value.
     * @param reasonHdrCause the received cause value of sip reason header
     * @param reasonHdrText the received text value of sip reason header
     */
    public void informNetworkResponse(int responseCode, String reason,
        int reasonHdrCause, String reasonHdrText) {
        ImsLog.d("informNetworkResponse:responseCode=" + responseCode +
            ", reasonHdrCause=" + reasonHdrCause);
        try {
            if (reasonHdrCause == 0) {
                callback.onNetworkResponse(responseCode, reason);
            } else {
                callback.onNetworkResponse(responseCode, reason, reasonHdrCause, reasonHdrText);
            }
        } catch (Exception e) {
            ImsLog.e("Exception:" + e.toString());
        }
    }

    /**
     * Send command error regarding this request.
     * @param code the command error code. it is one of the {@link UceApiConstant}.
     */
    public void informCommandError(int code) {
        ImsLog.d("informCommandError:code=" + code);
        try {
            callback.onCommandError(code);
        } catch (Exception e) {
            ImsLog.e("Exception:" + e.toString());
        }
    }

    /**
     * Inform the received pidf xmls regarding this request
     * @param pidfXmls A {@link List} of the {@link String}s that After sending list subscribe and
     *                receiving one or more pidf xml, it is delivered in list form to notify
     *                AOSP Framework.
     */
    public void informCapabilitiesUpdate(List<String> pidfXmls) {
        ImsLog.d("informCapabilitiesUpdate");
        try {
            callback.onNotifyCapabilitiesUpdate(pidfXmls);
        } catch (Exception e) {
            ImsLog.e("Exception:" + e.toString());
        }
    }

    /**
     * The subscription associated operation has been terminated.
     * @param reason The reason for the request being unable to process.
     * @param retryAfterSecond The time in second the requesting application should
     * wait before retrying, if non-zero.
     */
    public void informTerminate(String reason, int retryAfterSecond) {
        ImsLog.d("informTerminate:reason=" + reason + ",retryAfterSecond=" + retryAfterSecond);
        try {
            callback.onTerminated(reason, TimeUnit.SECONDS.toMillis(retryAfterSecond));
        } catch (Exception e) {
            ImsLog.e("Exception:" + e.toString());
        }
    }

    /**
     * Notify the framework that a resource in the RLMI XML contained in the NOTIFY response
     * for the ongoing SUBSCRIBE dialog has been terminated.
     * @param resourceInfoList The contact URIs which have been terminated. Each Resource info
     *                        in the list is the contact URI and its terminated reason.
     */
    public void informResourceTerminate(ArrayList<UceResourceInfo> resourceInfoList) {
        List<Pair<Uri, String>> uriTerminatedReason = new ArrayList<>();
        for (UceResourceInfo info : resourceInfoList) {
            uriTerminatedReason.add(new Pair<>(Uri.parse(info.getId()), info.getReason()));
        }
        try {
            callback.onResourceTerminated(uriTerminatedReason);
        } catch (Exception e) {
            ImsLog.e("Exception:" + e.toString());
        }
    }
}
