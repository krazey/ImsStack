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

import com.android.imsstack.enabler.uce.impl.define.UceMessage;
import com.android.imsstack.enabler.uce.impl.jni.UceJNI;
import com.android.imsstack.enabler.uce.impl.utils.UceUtils;
import com.android.imsstack.enabler.uce.interf.RemoteOptionsCallback;
import com.android.internal.annotations.VisibleForTesting;

import java.util.Set;

public class UceOptionsResponseCallback implements RemoteOptionsCallback{
    public final int mKey;
    private final int mSlotId;
    private final UceJNI mUceJNI;

    public UceOptionsResponseCallback(int key, int slotId) {
        this(key, slotId, UceJNI.getInstance());
    }
    @VisibleForTesting
    public UceOptionsResponseCallback(int key, int slotId, UceJNI jni) {
        mKey = key;
        mSlotId = slotId;
        mUceJNI = jni;
    }

    @Override
    public void onRespondToCapabilityRequest(Set<String> ownCapabilities, boolean isBlocked) {
        Parcel parcel = Parcel.obtain();
        parcel.writeInt(UceMessage.UCE_SEND_OPTIONS_RESP_CMD);

        parcel.writeInt(mKey);
        parcel.writeInt(200); // response code
        parcel.writeString(""); // reason
        if (isBlocked) {
            parcel.writeLong(0); // my capabilities
        } else{
            if (ownCapabilities.isEmpty()) {
                parcel.writeLong(0); // my capabilities
            } else {
                parcel.writeLong(UceUtils.getCapabilities(ownCapabilities));
            }
        }
        mUceJNI.sendMessage(mSlotId, parcel);
    }

    @Override
    public void onRespondToCapabilityRequestWithError(int code, String reason) {
        Parcel parcel = Parcel.obtain();
        parcel.writeInt(UceMessage.UCE_SEND_OPTIONS_RESP_CMD);

        parcel.writeInt(mKey);
        parcel.writeInt(code); // response code
        parcel.writeString(reason); // reason
        parcel.writeLong(0); // my capabilities
        mUceJNI.sendMessage(mSlotId, parcel);
    }
}
