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
package com.android.imsstack.core;

import android.content.Context;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.config.FeatureConfig;
import com.android.imsstack.enabler.aos.AosFactory;
import com.android.imsstack.jni.JNIIms;
import com.android.imsstack.system.JNIUpCallEvtManager;
import com.android.imsstack.system.SystemConfig;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.test.ImsTestMode;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.ImsUtils;
import com.android.imsstack.util.Log;
import com.android.imsstack.util.MSimUtils;

import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;

public class CommonStarter {
    public static final int STATE_IDLE = 0;
    public static final int STATE_CREATED = 1;
    public static final int STATE_READY = 2;

    private static final String TAG = "ImsStack_CommonStarter";

    private static CommonStarter sCS = null;

    private boolean mJNIReady = false;
    private boolean mCommonAgentReady = false;
    private int[] mState = new int[MSimUtils.getMaxSimSlot()];
    private Set<ICommonPackageListener> mListeners
            = new HashSet<ICommonPackageListener>();
    private Set<IVoltePackageListener> mVolteListeners
            = new HashSet<IVoltePackageListener>();

    private CommonStarter() {
    }

    public static CommonStarter getInstance() {
        if (sCS == null) {
            sCS = new CommonStarter();
        }

        return sCS;
    }

    public int getState(int slotId) {
        if (slotId < 0 || slotId >= mState.length) {
            return STATE_IDLE;
        }

        synchronized (this) {
            return mState[slotId];
        }
    }

    public void addListener(ICommonPackageListener listener) {
        if (listener == null) {
            return;
        }

        synchronized (mListeners) {
            mListeners.add(listener);
        }
    }

    public void removeListener(ICommonPackageListener listener) {
        synchronized (mListeners) {
            mListeners.remove(listener);
        }
    }

    public void addVolteListener(IVoltePackageListener listener) {
        if (listener == null) {
            return;
        }

        synchronized (mVolteListeners) {
            mVolteListeners.add(listener);
        }
    }

    public void removeVolteListener(IVoltePackageListener listener) {
        synchronized (mVolteListeners) {
            mVolteListeners.remove(listener);
        }
    }

    public boolean isCommonAgentReady() {
        return mCommonAgentReady;
    }

    public boolean isJNIReady() {
        return mJNIReady;
    }

    public void createAgents() {
        if (isCommonAgentReady()) {
            return;
        }

        Log.i(TAG, "createAgents");

        Context context = AppContext.getInstance();

        ImsLog.init();

        SystemConfig.setDeviceConfig(
                MSimUtils.getMaxSimSlot(),
                true, // imsEmergencyEnabled
                ImsUtils.isVoLteEnabledByDevice(
                        context, MSimUtils.DEFAULT_PHONE_ID),
                ImsUtils.isVtEnabledByDevice(
                        context, MSimUtils.DEFAULT_PHONE_ID),
                ImsUtils.isWfcEnabledByDevice(
                        context, MSimUtils.DEFAULT_PHONE_ID));

        SystemInterface.getInstance().init();

        AosFactory.getInstance().init();

        JNIUpCallEvtManager.getInstance().init();

        AgentFactory.createDefaultAgents();
        AgentFactory.initDefaultAgents(context);
    }

    public void notifyVoltePackageReady(int slotId) {
        Log.i(TAG, "notifyVoltePackageReady(" + slotId + ")");
        synchronized (mVolteListeners) {
            Iterator<IVoltePackageListener> iterator = mVolteListeners.iterator();

            while (iterator.hasNext()) {
                IVoltePackageListener listener = iterator.next();
                listener.onVoltePackageReady(slotId);
            }
        }
    }

    public void startAgents(int slotId) {
        Log.i(TAG, "startAgents(" + slotId + ")");

        Context context = AppContext.getInstance();

        ImsTestMode.getInstance().init(context, slotId);

        FeatureConfig.init(slotId);

        SystemInterface.getInstance().start(slotId);

        JNIUpCallEvtManager.getInstance().start(slotId);

        AosFactory.getInstance().start(slotId);

        AgentFactory.createAgents(context, slotId);
        AgentFactory.initAgentsForMIms(context, slotId);

        DcFactory.createDc(context, slotId);
        DcFactory.initDc(context, slotId);

        ConfigLoader.updateCarrierConfig(slotId);

        setStateOnStart(slotId);

        notifyPackageReady(slotId);

        AosFactory.getInstance().init(slotId);
    }

    public void stopAgents(int slotId) {
        Log.i(TAG, "stopAgents(" + slotId + ")");

        setStateOnStop(slotId);

        notifyPackageStop(slotId);

        AosFactory.getInstance().cleanup(slotId);

        Context context = AppContext.getInstance();

        DcFactory.cleanUpDc(slotId);

        AgentFactory.cleanUpAgents(slotId);

        JNIUpCallEvtManager.getInstance().stop(slotId);

        AosFactory.getInstance().stop(slotId);

        SystemInterface.getInstance().stop(slotId);

        ImsTestMode.getInstance().cleanUp(slotId);
    }

    public void createJNI() {
        if (isJNIReady()) {
            return;
        }

        Log.i(TAG, "createJNI");

        updateSystemConfigForBootup();

        JNIIms.construct();

        mJNIReady = true;
    }

    public void setCommonAgentCompleted() {
        if (isCommonAgentReady()) {
            return;
        }

        mCommonAgentReady = true;
    }

    public void updateSystemConfigForBootup() {
        Log.i(TAG, "updateSystemConfigForBootup");
        NativeLoader.setSystemConfigForBootup();
    }

    public void updateSystemConfigOnSimLoaded(int slotId) {
        Log.i(TAG, "updateSystemConfigOnSimLoaded(" + slotId + ")");
        NativeLoader.setSystemConfigForAllConfigurationChanged(slotId, false);
    }

    private void notifyPackageReady(int slotId) {
        Log.i(TAG, "notifyPackageReady(" + slotId + ")");
        synchronized (mListeners) {
            Iterator<ICommonPackageListener> iterator = mListeners.iterator();

            while (iterator.hasNext()) {
                ICommonPackageListener listener = iterator.next();
                listener.onCommonPackageReady(slotId);
            }
        }
    }

    private void notifyPackageStop(int slotId) {
        Log.i(TAG, "notifyPackageStop(" + slotId + ")");
        synchronized (mListeners) {
            Iterator<ICommonPackageListener> iterator = mListeners.iterator();

            while (iterator.hasNext()) {
                ICommonPackageListener listener = iterator.next();
                listener.onCommonPackageStop(slotId);
            }
        }
    }

    private void setStateOnStart(int slotId) {
        if (slotId < 0 || slotId >= mState.length) {
            return;
        }

        int oldState = getState(slotId);
        int newState = STATE_READY;

        if (oldState != newState) {
            Log.i(TAG, "onStart :: slotId="
                    + slotId + ", state: " + oldState + " >> " + newState);

            synchronized (this) {
                mState[slotId] = newState;
            }
        }
    }

    private void setStateOnStop(int slotId) {
        if (slotId < 0 || slotId >= mState.length) {
            return;
        }

        int oldState = getState(slotId);
        int newState = (oldState == STATE_IDLE) ? STATE_IDLE : STATE_CREATED;

        if (oldState != newState) {
            Log.i(TAG, "onStop :: slotId="
                    + slotId + ", state: " + oldState + " >> " + newState);

            synchronized (this) {
                mState[slotId] = newState;
            }
        }
    }

    private static boolean isSlotIdValid(int slotId) {
        return (slotId >= MSimUtils.DEFAULT_SLOT_ID) && (slotId < MSimUtils.getMaxSimSlot());
    }
}
