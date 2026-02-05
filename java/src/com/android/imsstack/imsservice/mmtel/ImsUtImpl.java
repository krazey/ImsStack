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

package com.android.imsstack.imsservice.mmtel;

import static android.telephony.ims.feature.CapabilityChangeRequest.CapabilityPair;

import android.os.DeadObjectException;
import android.telephony.ims.ImsCallForwardInfo;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsSsInfo;
import android.telephony.ims.ImsUtListener;
import android.telephony.ims.stub.ImsUtImplBase;
import android.text.TextUtils;

import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.ssc.SscConstant;
import com.android.imsstack.imsservice.mmtel.ut.UtCommand;
import com.android.imsstack.imsservice.mmtel.ut.UtFactory;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtInterface;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtListener;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.util.Arrays;
import java.util.List;

public final class ImsUtImpl extends ImsUtImplBase {
    private static final boolean DBG = ImsLog.isDebuggable();

    private final IBaseContext mContext;
    @VisibleForTesting
    ImsUtListener mListener = null;
    private IUtInterface mUt = null;
    private IUtListener mUtListenerProxy = null;
    private int mTransactionId = 1;

    public ImsUtImpl(IBaseContext context) {
        mContext = context;
        init();
    }

    public void init() {
        mUt = UtFactory.getInstance().getUtInterface(mContext.getSlotId());
        if (mUt == null) {
            return;
        }

        mUt.start(mContext.getContext());
        mUtListenerProxy = new UtListenerProxy();
        mUt.setListener(mUtListenerProxy);
    }

    public void clear() {
        if (mUt == null) {
            return;
        }

        mUt.close();
        mUtListenerProxy = null;
        mUt.setListener(null);
        mUt = null;
    }

    public void dispose() {
        clear();
        UtFactory.getInstance().releaseUtInterface(mContext.getSlotId());
    }

    public IUtInterface getUtInterface() {
        return mUt;
    }

    private int getTransactionId() {
        synchronized (this) {
            if (mTransactionId == Integer.MAX_VALUE) {
                mTransactionId = 1;
            }

            return mTransactionId++;
        }
    }

    @Override
    public void close() {
        if (DBG) {
            log("close");
        }

        // do nothing. all start/stop for ImsUtImpl will be handled ImsStack internally
    }

    @Override
    public int queryCallBarring(int cbType) {
        if (DBG) {
            log("queryCallBarring :: cbType=" + cbType);
        }

        if (mUt == null) {
            return (ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1));
        }

        int tId = getTransactionId();
        UtCommand utCmd = new UtCommand.Builder(mContext, tId, UtCommand.CMD_CB,
                SscConstant.ACTION_INTERROGATION, mUtListenerProxy).setCondition(cbType).build();
        utCmd.startTransaction();

        return tId;
    }

    @Override
    public int queryCallBarringForServiceClass(int cbType, int serviceClass) {
        if (DBG) {
            log("queryCallBarringForServiceClass :: cbType=" + cbType
                + ", serviceClass=" + serviceClass);
        }

        if (mUt == null) {
            return (ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1));
        }

        int tId = getTransactionId();
        UtCommand utCmd = new UtCommand.Builder(mContext, tId, UtCommand.CMD_CB,
                SscConstant.ACTION_INTERROGATION, mUtListenerProxy).setCondition(cbType)
                .setServiceClass(serviceClass).build();
        utCmd.startTransaction();

        return tId;
    }

    @Override
    public int queryCallForward(int condition, String number) {
        if (DBG) {
            log("queryCallForward :: condition=" + condition + ", number=" + number);
        }

        if (mUt == null) {
            return (ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1));
        }

        int tId = getTransactionId();
        UtCommand utCmd = new UtCommand.Builder(mContext, tId, UtCommand.CMD_CF,
                SscConstant.ACTION_INTERROGATION, mUtListenerProxy).setCondition(condition)
                .setTargetNumber(number).build();
        utCmd.startTransaction();

        return tId;
    }

    @Override
    public int queryCallWaiting() {
        if (DBG) {
            log("queryCallWaiting :: ");
        }

        if (mUt == null) {
            return (ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1));
        }

        int tId = getTransactionId();
        UtCommand utCmd = new UtCommand.Builder(mContext, tId, UtCommand.CMD_CW,
                SscConstant.ACTION_INTERROGATION, mUtListenerProxy).build();
        utCmd.startTransaction();

        return tId;
    }

    @Override
    public int queryCLIR() {
        if (DBG) {
            log("queryCLIR :: ");
        }

        if (mUt == null) {
            return (ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1));
        }

        int tId = getTransactionId();
        UtCommand utCmd = new UtCommand.Builder(mContext, tId, UtCommand.CMD_OIR,
                SscConstant.ACTION_INTERROGATION, mUtListenerProxy).build();
        utCmd.startTransaction();

        return tId;
    }

    @Override
    public int queryCLIP() {
        if (DBG) {
            log("queryCLIP :: ");
        }

        if (mUt == null) {
            return (ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1));
        }

        int tId = getTransactionId();
        UtCommand utCmd = new UtCommand.Builder(mContext, tId, UtCommand.CMD_OIP,
                SscConstant.ACTION_INTERROGATION, mUtListenerProxy).build();
        utCmd.startTransaction();

        return tId;
    }

    @Override
    public int queryCOLR() {
        if (DBG) {
            log("queryCOLR :: ");
        }

        if (mUt == null) {
            return (ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1));
        }

        int tId = getTransactionId();
        UtCommand utCmd = new UtCommand.Builder(mContext, tId, UtCommand.CMD_TIR,
                SscConstant.ACTION_INTERROGATION, mUtListenerProxy).build();
        utCmd.startTransaction();

        return tId;
    }

    @Override
    public int queryCOLP() {
        if (DBG) {
            log("queryCOLP :: ");
        }

        if (mUt == null) {
            return (ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1));
        }

        int tId = getTransactionId();
        UtCommand utCmd = new UtCommand.Builder(mContext, tId, UtCommand.CMD_TIP,
                SscConstant.ACTION_INTERROGATION, mUtListenerProxy).build();
        utCmd.startTransaction();

        return tId;
    }

    @Override
    public int updateCallBarring(int cbType, int action, String[] barringList) {
        if (DBG) {
            log("updateCallBarring :: cbType=" + cbType
                    + ", action=" + action + ", barringList=" + Arrays.toString(barringList));
        }

        if (mUt == null) {
            return (ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1));
        }

        int tId = getTransactionId();
        UtCommand utCmd = new UtCommand.Builder(mContext, tId, UtCommand.CMD_CB, action,
                mUtListenerProxy).setCondition(cbType).setBarringList(barringList).build();
        utCmd.startTransaction();

        return tId;
    }

    @Override
    public int updateCallBarringForServiceClass(int cbType, int action, String[] barringList,
            int serviceClass) {
        if (DBG) {
            log("updateCallBarringForServiceClass :: cbType=" + cbType
                + ", action=" + action + ", serviceClass=" + serviceClass
                + ", barringList=" + Arrays.toString(barringList));
        }

        if (mUt == null) {
            return (ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1));
        }

        int tId = getTransactionId();
        UtCommand utCmd = new UtCommand.Builder(mContext, tId, UtCommand.CMD_CB, action,
                mUtListenerProxy).setCondition(cbType).setBarringList(barringList)
                .setServiceClass(serviceClass).build();
        utCmd.startTransaction();

        return tId;
    }

    @Override
    public int updateCallBarringWithPassword(int cbType, int action, String[] barringList,
            int serviceClass, String password) {
        if (DBG) {
            log("updateCallBarringWithPassword :: cbType=" + cbType + ", action=" + action
                    + ", serviceClass=" + serviceClass
                    + ", barringList=" + Arrays.toString(barringList)
                    + ", password=" + (TextUtils.isEmpty(password) ? null : "****"));
        }

        if (mUt == null) {
            return (ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1));
        }

        int tId = getTransactionId();
        UtCommand utCmd = new UtCommand.Builder(mContext, tId, UtCommand.CMD_CB, action,
                mUtListenerProxy).setCondition(cbType).setBarringList(barringList)
                .setServiceClass(serviceClass).setPassword(password).build();
        utCmd.startTransaction();

        return tId;
    }

    @Override
    public int updateCallForward(int action, int condition, String number, int serviceClass,
            int timeSeconds) {
        if (DBG) {
            log("updateCallForward :: action=" + action + ", condition=" + condition
                    + ", number=" + number + ", serviceClass=" + serviceClass
                    + ", timeSeconds=" + timeSeconds);
        }

        if (mUt == null) {
            return (ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1));
        }

        int tId = getTransactionId();
        UtCommand utCmd = new UtCommand.Builder(mContext, tId, UtCommand.CMD_CF, action,
                mUtListenerProxy).setCondition(condition).setTargetNumber(number)
                .setServiceClass(serviceClass).setNoReplyTimer(timeSeconds).build();
        utCmd.startTransaction();

        return tId;
    }

    @Override
    public int updateCallWaiting(boolean enable, int serviceClass) {
        if (DBG) {
            log("updateCallWaiting :: enable=" + enable + ", serviceClass=" + serviceClass);
        }

        if (mUt == null) {
            return (ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1));
        }

        int tId = getTransactionId();
        int action = enable ? SscConstant.ACTION_ACTIVATION : SscConstant.ACTION_DEACTIVATION;
        UtCommand utCmd = new UtCommand.Builder(mContext, tId, UtCommand.CMD_CW, action,
                mUtListenerProxy).setEnable(enable).setServiceClass(serviceClass).build();
        utCmd.startTransaction();

        return tId;
    }

    @Override
    public int updateCLIR(int clirMode) {
        if (DBG) {
            log("updateCLIR :: clirMode=" + clirMode);
        }

        if (mUt == null) {
            return (ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1));
        }

        int tId = getTransactionId();
        int action = (clirMode == SscConstant.OIR_INVOCATION)
                ? SscConstant.STATUS_ENABLE : SscConstant.ACTION_DEACTIVATION;
        UtCommand utCmd = new UtCommand.Builder(mContext, tId, UtCommand.CMD_OIR, action,
                mUtListenerProxy).setClirMode(clirMode).build();
        utCmd.startTransaction();

        return tId;
    }

    @Override
    public int updateCLIP(boolean enable) {
        if (DBG) {
            log("updateCLIP :: enable=" + enable);
        }

        if (mUt == null) {
            return (ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1));
        }

        int tId = getTransactionId();
        int action = enable ? SscConstant.ACTION_ACTIVATION : SscConstant.ACTION_DEACTIVATION;
        UtCommand utCmd = new UtCommand.Builder(mContext, tId, UtCommand.CMD_OIP, action,
                mUtListenerProxy).setEnable(enable).build();
        utCmd.startTransaction();

        return tId;
    }

    @Override
    public int updateCOLR(int presentation) {
        if (DBG) {
            log("updateCOLR :: presentation=" + presentation);
        }

        if (mUt == null) {
            return (ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1));
        }

        int tId = getTransactionId();
        int action = (presentation == SscConstant.TIR_PROVISIONED)
                ? SscConstant.ACTION_ACTIVATION : SscConstant.ACTION_DEACTIVATION;
        UtCommand utCmd = new UtCommand.Builder(mContext, tId, UtCommand.CMD_TIR, action,
                mUtListenerProxy).setColrPresentation(presentation).build();
        utCmd.startTransaction();

        return tId;
    }

    @Override
    public int updateCOLP(boolean enable) {
        if (DBG) {
            log("updateCOLP :: enable=" + enable);
        }

        if (mUt == null) {
            return (ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1));
        }

        int tId = getTransactionId();
        int action = enable ? SscConstant.ACTION_ACTIVATION : SscConstant.ACTION_DEACTIVATION;
        UtCommand utCmd = new UtCommand.Builder(mContext, tId, UtCommand.CMD_TIP, action,
                mUtListenerProxy).setEnable(enable).build();
        utCmd.startTransaction();

        return tId;
    }

    @Override
    public void setListener(ImsUtListener listener) {
        mListener = listener;
    }

    public boolean isUtAvailable() {
        return (mUt != null) && mUt.isUtAvailable();
    }

    /**
     * Updating feature capabilities when capabilities are changed from
     * {@link com.android.ims.ImsManager#changeMmTelCapability}
     *
     * @param enabledCaps list of CapabilityPair of features which are enabled.
     * @param disabledCaps list of CapabilityPair of features which are disabled.
     */
    public void changeCapabilities(List<CapabilityPair> enabledCaps,
            List<CapabilityPair> disabledCaps) {
        if (mUt == null) {
            return;
        }

        mUt.changeCapabilities(enabledCaps, disabledCaps);
    }

    private void postAndRunTask(Runnable task) {
        mContext.getExecutor().execute(task);
    }

    private void log(Throwable t, String message) {
        ImsLog.e(mContext.getSlotId(), "[ISIL] " + message + t, t);

        if (t instanceof DeadObjectException) {
            mListener = null;
        }
    }

    private void log(String s) {
        ImsLog.d(mContext.getSlotId(), "[ISIL] " + s);
    }

    private class UtListenerProxy implements IUtListener {
        @Override
        public void utConfigurationUpdated(int id) {
            if (mListener == null) {
                return;
            }

            postAndRunTask(new Runnable() {
                @Override
                public void run() {
                    try {
                        mListener.onUtConfigurationUpdated(id);
                    } catch (Throwable t) {
                        log(t, "onConfigurationUpdated");
                    }
                }
            });
        }

        @Override
        public void utConfigurationUpdateFailed(int id, ImsReasonInfo reasonInfo) {
            if (mListener == null) {
                return;
            }

            postAndRunTask(new Runnable() {
                @Override
                public void run() {
                    try {
                        mListener.onUtConfigurationUpdateFailed(id, reasonInfo);
                    } catch (Throwable t) {
                        log(t, "onConfigurationUpdateFailed");
                    }
                }
            });
        }

        @Override
        public void lineIdentificationSupplementaryServiceResponse(int id, ImsSsInfo ssInfo) {
            if (mListener == null) {
                return;
            }

            postAndRunTask(new Runnable() {
                @Override
                public void run() {
                    try {
                        mListener.onLineIdentificationSupplementaryServiceResponse(id, ssInfo);
                    } catch (Throwable t) {
                        log(t, "onConfigurationQueried");
                    }
                }
            });
        }

        @Override
        public void utConfigurationCallBarringQueried(int id, ImsSsInfo[] cbInfo) {
            if (mListener == null) {
                return;
            }

            postAndRunTask(new Runnable() {
                @Override
                public void run() {
                    try {
                        mListener.onUtConfigurationCallBarringQueried(id, cbInfo);
                    } catch (Throwable t) {
                        log(t, "onConfigurationQueried");
                    }
                }
            });
        }

        @Override
        public void utConfigurationCallForwardQueried(int id, ImsCallForwardInfo[] cfInfo) {
            if (mListener == null) {
                return;
            }

            postAndRunTask(new Runnable() {
                @Override
                public void run() {
                    try {
                        mListener.onUtConfigurationCallForwardQueried(id, cfInfo);
                    } catch (Throwable t) {
                        log(t, "onConfigurationQueried");
                    }
                }
            });
        }

        @Override
        public void utConfigurationCallWaitingQueried(int id, ImsSsInfo[] cwInfo) {
            if (mListener == null) {
                return;
            }

            postAndRunTask(new Runnable() {
                @Override
                public void run() {
                    try {
                        mListener.onUtConfigurationCallWaitingQueried(id, cwInfo);
                    } catch (Throwable t) {
                        log(t, "onConfigurationQueried");
                    }
                }
            });
        }

        @Override
        public void utConfigurationQueryFailed(int id, ImsReasonInfo reasonInfo) {
            if (mListener == null) {
                return;
            }

            postAndRunTask(new Runnable() {
                @Override
                public void run() {
                    try {
                        mListener.onUtConfigurationQueryFailed(id, reasonInfo);
                    } catch (Throwable t) {
                        log(t, "onConfigurationQueryFailed");
                    }
                }
            });
        }
    }
}
