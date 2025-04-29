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

package com.android.imsstack.enabler.mtc;

import android.os.Parcel;

import com.android.imsstack.jni.JniImsListener;
import com.android.imsstack.jni.JniImsProxy;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

/**
 * provides APIs for getting, releasing an interface between Java and Native for call.
 * also provides path for seding a data to Native.
 */
public class MtcJniProxy {
    private static MtcJniProxy sMtcJniProxy = null;
    private static MtcJniProxy sTestInstance = null;    // for UnitTest for mtcMediaSession

    /**
     * Creates singleton object of {@code MtcJniProxy}.
     */
    public static MtcJniProxy getInstance() {
        if (sTestInstance != null) {
            return sTestInstance;
        }
        synchronized (MtcJniProxy.class) {
            if (sMtcJniProxy == null) {
                sMtcJniProxy = new MtcJniProxy();
            }
        }
        return sMtcJniProxy;
    }

    /**
     * Sets a test instance for the MtcJniProxy singleton.
     * Should only be used for testing.
     */
    @VisibleForTesting
    public static void setInstanceForTesting(MtcJniProxy testInstance) {
        sTestInstance = testInstance;
    }

    /**
     * Creates an interface between Java and Native for call.
     *
     * @param nSlot sim slot number.
     * @param category that decides which kinds of an interface to get.
     * @param listener that will receives a data from Native.
     * @@return a memory address of Native object.
     */
    public long getJniInterfaceAndSetListener(int nSlot, int category, JniImsListener listener) {
        long nativeObj = JniImsProxy.getInterface(category, nSlot);
        if (nativeObj != 0) {
            JniImsProxy.setListener(nativeObj, listener);
        }
        return nativeObj;
    }

    /**
     * Releases an interface between Java and Native for call.
     *
     * @param nativeObj a memory address of Native object.
     * @param listener that needs to be removed.
     */
    public void releaseJniInterfaceAndrRemoveListener(long nativeObj, JniImsListener listener) {
        JniImsProxy.releaseInterface(nativeObj);
        JniImsProxy.removeListener(nativeObj);
    }

    /**
     * Sends a data to Native.
     *
     * @param nativeObj a memory address of Native object.
     * @param parcel that has a data.
     */
    public void sendDataToNative(long nativeObj, Parcel parcel) {
        if (parcel == null) {
            ImsLog.i("parcel is null");
            return;
        }

        byte[] baData = parcel.marshall();

        JniImsProxy.sendData(nativeObj, baData);

        parcel.recycle();
        parcel = null;
    }
}
