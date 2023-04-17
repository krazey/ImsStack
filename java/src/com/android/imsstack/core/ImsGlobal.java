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
import android.content.ContextWrapper;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.util.ImsPrivateProperties;
import com.android.imsstack.util.MessageExecutor;

import java.util.Objects;

public class ImsGlobal extends ContextWrapper {
    private static ImsGlobal sImsGlobal = null;
    private final Handler mDefaultHandler;
    private final HandlerThread mDefaultHandlerThread =
            new HandlerThread(ImsGlobal.class.getSimpleName());
    private final Handler mCallHandler;
    private final HandlerThread mCallHandlerThread = new HandlerThread("ImsCallHandlerThread");
    private final MessageExecutor mExecutor;

    private ImsGlobal(Context context) {
        super(context);
        sImsGlobal = this;

        mDefaultHandlerThread.start();
        mDefaultHandler = new Handler(mDefaultHandlerThread.getLooper());
        mExecutor = new MessageExecutor(mDefaultHandlerThread.getLooper());

        mCallHandlerThread.start();
        mCallHandler = new Handler(mCallHandlerThread.getLooper());
    }

    public static ImsGlobal create(Context context) {
        return (sImsGlobal == null) ? new ImsGlobal(context) : sImsGlobal;
    }

    public static ImsGlobal getInstance() {
        return sImsGlobal;
    }

    public static boolean isVoLteProvisioningRequired(Context context, int slotId) {
        ConfigInterface config = AgentFactory.getInstance().getAgent(
                ConfigInterface.class, slotId);
        CarrierConfig cc = (config != null) ? config.getCarrierConfig() : null;
        return cc != null && cc.isVoLteProvisioningRequired();
    }

    public static boolean isVtProvisioningRequired(Context context, int slotId) {
        if (isOperator(slotId, "VZW")) {
            return true;
        } else if (isOperator(slotId, "ATT")) {
            return isVoLteProvisioningRequired(context, slotId);
        }

        return false;
    }

    public static boolean isWfcProvisioningRequired(Context context, int slotId) {
        if (isOperator(slotId, "ATT")) {
            return isVoLteProvisioningRequired(context, slotId);
        }

        return false;
    }

    // OPERATOR_COUNTRY {
    public static boolean equalsOperator(String op1, String op2) {
        return Objects.equals(op1, op2);
    }

    public static boolean equalsCountry(String co1, String co2) {
        return Objects.equals(co1, co2);
    }

    public static boolean equalsOperatorCountry(String op1, String co1, String op2, String co2) {
        return equalsOperator(op1, op2) && equalsCountry(co1, co2);
    }

    public static String getOperator(int slotId) {
        return ImsPrivateProperties.getSimOperator(slotId);
    }

    public static String getCountry(int slotId) {
        return ImsPrivateProperties.getSimCountry(slotId);
    }

    public static boolean isOperator(int slotId, String operator) {
        return getOperator(slotId).equals(operator);
    }

    public static boolean isCountry(int slotId, String country) {
        return getCountry(slotId).equals(country);
    }

    public static boolean isOperatorCountry(int slotId, String operator, String country) {
        return isOperator(slotId, operator) && isCountry(slotId, country);
    }
    // OPERATOR_COUNTRY }

    public static void postAndRunTask(Runnable task) {
        ImsGlobal app = ImsGlobal.getInstance();

        if (app == null) {
            task.run();
        } else {
            app.mExecutor.execute(task);
        }
    }

    public Handler getCallHandler() {
        return mCallHandler;
    }

    public Looper getCallLooper() {
        Looper looper = mCallHandlerThread.getLooper();
        return (looper != null) ? looper : Looper.getMainLooper();
    }

    public Handler getDefaultHandler() {
        return mDefaultHandler;
    }

    public Looper getDefaultLooper() {
        Looper looper = mDefaultHandlerThread.getLooper();
        return (looper != null) ? looper : Looper.getMainLooper();
    }

    public MessageExecutor getExecutor() {
        return mExecutor;
    }
}
