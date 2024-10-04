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
import android.util.ArrayMap;

import com.android.imsstack.util.Log;
import com.android.internal.annotations.Keep;

import java.io.FileDescriptor;

/**
 * This class provides the interfaces to send/receive the data from native to Java,
 * or vice versa.
 */
@Keep
public class JniIms {
    /** Indicates the success result of an operation. */
    public static final int OK = 0;
    /** A default error code. */
    public static final int ERROR = -1;
    /** Error code when there is no matched listener. */
    public static final int ERROR_NO_LISTENER = -2;
    /** Byte array representation of the failure result of the JNI operations. */
    public static final byte[] RESULT_FAILURE = new byte[] {(byte) 0};

    private class Callback {
        public int onDataReceived(long nativeObject, byte[] data) {
            return handleData(nativeObject, data);
        }

        public byte[] onDataReceivedForSystem(long nativeObject, byte[] data, FileDescriptor fd) {
            return handleDataForSystem(nativeObject, data, fd);
        }
    }

    private static Callback sCallback = null;
    private final ArrayMap<Long, JniImsListener> mListeners = new ArrayMap<>();
    private final ArrayMap<Long, JniSystemListener> mSystemListeners = new ArrayMap<>();

    /** Native APIs. */
    /** Initializes the native resources. */
    private static native void nativeInit();
    /** Deinitializes the native resources. */
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

    /* package */ JniIms() {
        initCallback();
    }

    /**
     * Defines the wrapper methods to invoke the native methods.
     */
    /** Initializes the native resources. */
    public void init() {
        nativeInit();
    }

    /** Deinitializes the native resources. */
    public void deinit() {
        nativeDeInit();
    }

    /** Creates the native object with a specified interface type and slot-id. */
    public long getInterface(int interfaceType, int slotId) {
        return nativeGetInterface(interfaceType, slotId);
    }

    /** Releases the native object. */
    public void releaseInterface(long nativeObject) {
        nativeReleaseInterface(nativeObject);
    }

    /** Sends an event from Java to native. */
    public int sendData(long nativeObject, byte[] data) {
        return nativeSendData(nativeObject, data);
    }

    /** Sends a system event from Java to native. */
    public byte[] sendDataForSystem(long nativeObject, byte[] data) {
        return nativeSendDataForSystem(nativeObject, data);
    }

    /** Sends a command to control and manage the native logics. */
    public int sendCommand(int cmd, int slotId, byte[] data) {
        return nativeSendCommand(cmd, slotId, data);
    }

    /** Enabler's interface */
    /** Sets the listener to receive an event from native. */
    public int setListener(long nativeObject, JniImsListener listener) {
        if (nativeObject == 0) {
            return ERROR;
        }

        if (listener == null) {
            return ERROR;
        }

        Long key = Long.valueOf(nativeObject);

        synchronized (mListeners) {
            mListeners.put(key, listener);
        }

        return OK;
    }

    /** Returns the listener for a specified native object. */
    public JniImsListener getListener(long nativeObject) {
        if (nativeObject == 0) {
            return null;
        }

        Long key = Long.valueOf(nativeObject);

        JniImsListener listener = null;

        synchronized (mListeners) {
            listener = mListeners.get(key);
        }

        return listener;
    }

    /** Removes the listener for a specified native object. */
    public int removeListener(long nativeObject) {
        if (nativeObject == 0) {
            return ERROR;
        }

        Long key = Long.valueOf(nativeObject);

        synchronized (mListeners) {
            mListeners.remove(key);
        }

        return OK;
    }

    /** System interface */
    /** Sets the listener to receive a system call from native. */
    public int setSystemListener(long nativeObject, JniSystemListener systemListener) {
        if (nativeObject == 0) {
            return ERROR;
        }

        if (systemListener == null) {
            return ERROR;
        }

        Long key = Long.valueOf(nativeObject);

        synchronized (mSystemListeners) {
            mSystemListeners.put(key, systemListener);
        }

        return OK;
    }

    /** Returns the listener for a specified native object. */
    public JniSystemListener getSystemListener(long nativeObject) {
        if (nativeObject == 0) {
            return null;
        }

        Long key = Long.valueOf(nativeObject);

        JniSystemListener listener = null;

        synchronized (mSystemListeners) {
            listener = mSystemListeners.get(key);
        }

        return listener;
    }

    /** Removes the listener for a specified native object. */
    public int removeSystemListener(long nativeObject) {
        if (nativeObject == 0) {
            return ERROR;
        }

        Long key = Long.valueOf(nativeObject);

        synchronized (mSystemListeners) {
            mSystemListeners.remove(key);
        }

        return OK;
    }

    private void initCallback() {
        sCallback = new Callback();
    }

    /** Handles the data from native. */
    private int handleData(long nativeObject, byte[] data) {
        JniImsListener listener = getListener(nativeObject);

        if (listener == null) {
            Log.d(this, "No listener :: nativeObject=" + nativeObject);
            return ERROR_NO_LISTENER;
        }

        Parcel parcel = Parcel.obtain();
        try {
            parcel.unmarshall(data, 0, data.length);
            parcel.setDataPosition(0);
            listener.onMessage(parcel);
        } finally {
            parcel.recycle();
        }

        return OK;
    }

    /** Handles the system data from native. */
    private byte[] handleDataForSystem(long nativeObject, byte[] data, FileDescriptor fd) {
        JniSystemListener listener = getSystemListener(nativeObject);

        if (listener == null) {
            Log.d(this, "No listener :: nativeObject=" + nativeObject);
            return RESULT_FAILURE;
        }

        byte[] result;
        Parcel parcel = Parcel.obtain();
        try {
            parcel.unmarshall(data, 0, data.length);
            parcel.setDataPosition(0);
            result = listener.onMessage(parcel, fd);
        } finally {
            parcel.recycle();
        }

        return result;
    }

    /**
     * Send a data from native to Java.
     * This is invoked from the native layer.
     */
    public static int sendDataToJava(long nativeObject, byte[] data) {
        return sCallback.onDataReceived(nativeObject, data);
    }

    /**
     * Send a data from native to Java for System interface.
     * This is invoked from the native layer.
     */
    public static byte[] sendDataToJavaForSystem(long nativeObject, byte[] data,
            FileDescriptor fd) {
        return sCallback.onDataReceivedForSystem(nativeObject, data, fd);
    }

    static {
        try {
            Log.i(JniIms.class, "Loading library... start");
            System.loadLibrary("imsstack");
            Log.i(JniIms.class, "Loading library... end");
        } catch (UnsatisfiedLinkError e) {
            Log.e(JniIms.class, "Loading library failed: libimsstack");
            e.printStackTrace();
        }
    }
}
