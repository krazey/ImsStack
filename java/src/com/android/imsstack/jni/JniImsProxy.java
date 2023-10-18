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
package com.android.imsstack.jni;

import com.android.internal.annotations.VisibleForTesting;

/**
 * This class is a proxy class to access and control the native ImsStack's interfaces.
 */
public final class JniImsProxy {
    /** Indicates the success result of an operation. */
    public static final int OK = JniIms.OK;
    /** A default error code. */
    public static final int ERROR = JniIms.ERROR;
    /** Error code when there is no matched listener. */
    public static final int ERROR_NO_LISTENER = JniIms.ERROR_NO_LISTENER;
    /** Byte array representation of the failure result of the JNI operations. */
    public static final byte[] RESULT_FAILURE = JniIms.RESULT_FAILURE;

    private static JniIms sJniIms = null;

    @VisibleForTesting
    public static void setJniIms(JniIms jniIms) {
        sJniIms = jniIms;
    }

    /** Initializes the native resources.*/
    public static void init() {
        getJniIms().init();
    }

    /** Deinitializes the native resources. */
    public static void deinit() {
        getJniIms().deinit();
    }

    /** Creates the native object with a specified interface type and slot-id. */
    public static long getInterface(int interfaceType, int slotId) {
        return getJniIms().getInterface(interfaceType, slotId);
    }

    /** Releases the native object. */
    public static void releaseInterface(long nativeObject) {
        getJniIms().releaseInterface(nativeObject);
    }

    /** Sends an event from Java to native. */
    public static int sendData(long nativeObject, byte[] data) {
        return getJniIms().sendData(nativeObject, data);
    }

    /** Sends a system event from Java to native. */
    public static byte[] sendDataForSystem(long nativeObject, byte[] data) {
        return getJniIms().sendDataForSystem(nativeObject, data);
    }

    /** Sends a command to control and manage the native logics. */
    public static int sendCommand(int cmd, int slotId, byte[] data) {
        return getJniIms().sendCommand(cmd, slotId, data);
    }

    /** Enabler's interface */
    /** Sets the listener to receive an event from native. */
    public static int setListener(long nativeObject, JniImsListener listener) {
        return getJniIms().setListener(nativeObject, listener);
    }

    /** Returns the listener for a specified native object. */
    public static JniImsListener getListener(long nativeObject) {
        return getJniIms().getListener(nativeObject);
    }

    /** Removes the listener for a specified native object. */
    public static int removeListener(long nativeObject) {
        return getJniIms().removeListener(nativeObject);
    }

    /** System interface */
    /** Sets the listener to receive a system call from native. */
    public static int setSystemListener(long nativeObject, JniSystemListener systemListener) {
        return getJniIms().setSystemListener(nativeObject, systemListener);
    }

    /** Returns the listener for a specified native object. */
    public static JniSystemListener getSystemListener(long nativeObject) {
        return getJniIms().getSystemListener(nativeObject);
    }

    /** Removes the listener for a specified native object. */
    public static int removeSystemListener(long nativeObject) {
        return getJniIms().removeSystemListener(nativeObject);
    }

    private static JniIms getJniIms() {
        if (sJniIms == null) {
            sJniIms = new JniIms();
        }
        return sJniIms;
    }

    private JniImsProxy() {}
}
