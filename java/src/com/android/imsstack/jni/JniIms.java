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

import android.os.Parcel;

import com.android.imsstack.util.ImsLog;

import java.io.FileDescriptor;
import java.util.Hashtable;

/**
 * This class provides the interfaces to send/receive the data from native to Java,
 * or vice versa.
 */
public class JniIms {
    /** Indicates the success result of an operation. */
    public static final int OK = 0;
    /** A default error code. */
    public static final int ERROR = -1;
    /** Error code when there is no matched listener. */
    public static final int ERROR_NO_LISTENER = -2;

    private static Hashtable<Long, JniImsListener> sListeners =
            new Hashtable<Long, JniImsListener>();
    private static Hashtable<Long, JniSystemListener> sSystemListeners =
            new Hashtable<Long, JniSystemListener>();

    /** Native APIs. */
    /** Initialize the native layer. */
    private static native void nativeInit();
    /** Deinitialize the native layer. */
    private static native void nativeDeInit();
    /** Creates the native object with a specified interface type and slot-id. */
    private static native long nativeGetInterface(int interfaceType, int slotId);
    /** Releases the native object. */
    private static native void nativeReleaseInterface(long nativeObject);
    /** Sends an event from Java to native. */
    private static native int nativeSendData(long nativeObject, byte[] data);
    /** Sends a system event from Java to native. */
    private static native byte[] nativeSendDataForSystem(long nativeObject, byte[] data);
    /** Sends a command to control and manage the native logics. */
    private static native int nativeSendCommand(int cmd, int slotId, byte[] data);

    /**
     * Defines the wrapper methods to invoke the native methods.
     */

    /** Initialize the native layer. */
    public static void init() {
        nativeInit();
    }

    /** Deinitialize the native layer. */
    public static void deinit() {
        nativeDeInit();
    }

    /** Creates the native object with a specified interface type and slot-id. */
    public static long getInterface(int interfaceType, int slotId) {
        return nativeGetInterface(interfaceType, slotId);
    }

    /** Releases the native object. */
    public static void releaseInterface(long nativeObject) {
        nativeReleaseInterface(nativeObject);
    }

    /** Sends an event from Java to native. */
    public static int sendData(long nativeObject, byte[] data) {
        return nativeSendData(nativeObject, data);
    }

    /** Sends a system event from Java to native. */
    public static byte[] sendDataForSystem(long nativeObject, byte[] data) {
        return nativeSendDataForSystem(nativeObject, data);
    }

    /** Sends a command to control and manage the native logics. */
    public static int sendCommand(int cmd, int slotId, byte[] data) {
        return nativeSendCommand(cmd, slotId, data);
    }

    /** Sets a configuration. */
    public static int setConfiguration(int event, byte[] data) {
        return sendCommand(event, -1, data);
    }

    /** Enabler's interface */
    /** Sets the listener to receive an event from native. */
    public static int setListener(long nativeObject, JniImsListener listener) {
        if (nativeObject == 0) {
            return ERROR;
        }

        if (listener == null) {
            return ERROR;
        }

        Long key = Long.valueOf(nativeObject);

        synchronized (sListeners) {
            sListeners.put(key, listener);
        }

        return OK;
    }

    /** Returns the listener for a specified native object. */
    public static JniImsListener getListener(long nativeObject) {
        if (nativeObject == 0) {
            return null;
        }

        Long key = Long.valueOf(nativeObject);

        JniImsListener listener = null;

        synchronized (sListeners) {
            listener = sListeners.get(key);
        }

        return listener;
    }

    /** Removes the listener for a specified native object. */
    public static int removeListener(long nativeObject) {
        if (nativeObject == 0) {
            return ERROR;
        }

        Long key = Long.valueOf(nativeObject);

        synchronized (sListeners) {
            sListeners.remove(key);
        }

        return OK;
    }

    /** System interface */
    /** Sets the listener to receive a system call from native. */
    public static int setSystemListener(long nativeObject, JniSystemListener systemListener) {
        if (nativeObject == 0) {
            return ERROR;
        }

        if (systemListener == null) {
            return ERROR;
        }

        Long key = Long.valueOf(nativeObject);

        synchronized (sSystemListeners) {
            sSystemListeners.put(key, systemListener);
        }

        return OK;
    }

    /** Returns the listener for a specified native object. */
    public static JniSystemListener getSystemListener(long nativeObject) {
        if (nativeObject == 0) {
            return null;
        }

        Long key = Long.valueOf(nativeObject);

        JniSystemListener listener = null;

        synchronized (sSystemListeners) {
            listener = sSystemListeners.get(key);
        }

        return listener;
    }

    /** Removes the listener for a specified native object. */
    public static int removeSystemListener(long nativeObject) {
        if (nativeObject == 0) {
            return ERROR;
        }

        Long key = Long.valueOf(nativeObject);

        synchronized (sSystemListeners) {
            sSystemListeners.remove(key);
        }

        return OK;
    }

    /**
     * Send a data from native to Java.
     * This is invoked from the native layer.
     */
    public static int sendDataToJava(long nativeObject, byte[] data) {
        JniImsListener listener = getListener(nativeObject);

        if (listener == null) {
            ImsLog.d("No listener :: nativeObject=" + nativeObject);
            return ERROR_NO_LISTENER;
        }

        Parcel parcel = Parcel.obtain();
        parcel.unmarshall(data, 0, data.length);
        parcel.setDataPosition(0);

        listener.onMessage(parcel);

        parcel.recycle();

        return OK;
    }

    /**
     * Send a data from native to Java for System interface.
     * This is invoked from the native layer.
     */
    public static byte[] sendDataToJavaForSystem(long nativeObject, byte[] data,
            FileDescriptor fd) {
        JniSystemListener listener = getSystemListener(nativeObject);

        if (listener == null) {
            ImsLog.d("No listener :: nativeObject=" + nativeObject);
            return new byte[] {(byte) 0};
        }

        Parcel parcel = Parcel.obtain();
        parcel.unmarshall(data, 0, data.length);
        parcel.setDataPosition(0);

        byte[] result = listener.onMessage(parcel, fd);
        parcel.recycle();

        return result;
    }

    static {
        try {
            ImsLog.i("Loading library... start");
            System.loadLibrary("imsstack");
            ImsLog.i("Loading library... end");
        } catch (UnsatisfiedLinkError e) {
            ImsLog.e("Loading library failed: libimsstack");
            e.printStackTrace();
        }
    }
}
