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

    private static Hashtable<Long, JniImsListener> sListeners =
            new Hashtable<Long, JniImsListener>();
    private static Hashtable<Long, JniSystemListener> sSystemListeners =
            new Hashtable<Long, JniSystemListener>();

    /** Native APIs. */
    /** Initialize the native layer. */
    public static native void construct();
    /** Deinitialize the native layer. */
    public static native void destruct();
    /** Creates the native object with a specified interface type and slot-id. */
    public static native long getInterface(int interfaceType, int slotId);
    /** Releases the native object. */
    public static native int releaseInterface(long nativeObj);
    /** Sends an event from Java to native. */
    public static native int sendData(long nativeObj, byte[] baData);
    /** Sends a system event from Java to native. */
    public static native byte[] sendDataEx(long nativeObj, byte[] baData);
    /** Sets a configuration. */
    public static native int setConfiguration(int event, byte[] baData);

    /** Enabler's interface */
    /** Sets the listener to receive an event from native. */
    public static int setListener(long nativeObj, JniImsListener listener) {
        if (nativeObj == 0) {
            return 0;
        }

        if (listener == null) {
            return 0;
        }

        Long key = Long.valueOf(nativeObj);

        synchronized (sListeners) {
            sListeners.put(key, listener);
        }

        return 1;
    }

    /** Returns the listener for a specified native object. */
    public static JniImsListener getListener(long nativeObj) {
        if (nativeObj == 0) {
            return null;
        }

        Long key = Long.valueOf(nativeObj);

        JniImsListener listener = null;

        synchronized (sListeners) {
            listener = sListeners.get(key);
        }

        return listener;
    }

    /** Removes the listener for a specified native object. */
    public static int removeListener(long nativeObj, JniImsListener listener) {
        if (nativeObj == 0) {
            return 0;
        }

        if (listener == null) {
            return 0;
        }

        Long key = Long.valueOf(nativeObj);

        synchronized (sListeners) {
            sListeners.remove(key);
        }

        return 1;
    }

    /** System interface */
    /** Sets the listener to receive a system call from native. */
    public static int setSystemListener(long nativeObj, JniSystemListener systemListener) {
        if (nativeObj == 0) {
            return 0;
        }

        if (systemListener == null) {
            return 0;
        }

        Long key = Long.valueOf(nativeObj);

        synchronized (sSystemListeners) {
            sSystemListeners.put(key, systemListener);
        }

        return 1;
    }

    /** Returns the listener for a specified native object. */
    public static JniSystemListener getSystemListener(long nativeObj) {
        if (nativeObj == 0) {
            return null;
        }

        Long key = Long.valueOf(nativeObj);

        JniSystemListener listener = null;

        synchronized (sSystemListeners) {
            listener = sSystemListeners.get(key);
        }

        return listener;
    }

    /** Removes the listener for a specified native object. */
    public static int removeSystemListener(long nativeObj, JniSystemListener systemListener) {
        if (nativeObj == 0) {
            return 0;
        }

        Long key = Long.valueOf(nativeObj);

        synchronized (sSystemListeners) {
            sSystemListeners.remove(key);
        }

        return 1;
    }

    /** Send a data from native to Java. */
    public static int sendData2Java(long nativeObj, byte[] baData) {
        JniImsListener listener = getListener(nativeObj);

        if (listener == null) {
            ImsLog.d("No listener :: nativeObject=" + nativeObj);
            return -1;
        }

        Parcel parcel = Parcel.obtain();
        parcel.unmarshall(baData, 0, baData.length);
        parcel.setDataPosition(0);

        listener.onMessage(parcel);

        parcel.recycle();

        return 1;
    }

    /** Send a data from native to Java for System interface. */
    public static byte[] sendData2JavaEx(long nativeObj, byte[] baData, FileDescriptor fd) {
        JniSystemListener listener = getSystemListener(nativeObj);

        if (listener == null) {
            ImsLog.d("No listener :: nativeObject=" + nativeObj);
            return new byte[] {(byte) 0};
        }

        Parcel parcel = Parcel.obtain();
        parcel.unmarshall(baData, 0, baData.length);
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
