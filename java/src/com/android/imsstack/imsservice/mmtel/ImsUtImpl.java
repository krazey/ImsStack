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

import android.os.Bundle;
import android.os.DeadObjectException;
import android.os.RemoteException;
import android.telephony.ims.ImsCallForwardInfo;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsSsInfo;
import android.telephony.ims.ImsUtListener;
import android.telephony.ims.stub.ImsUtImplBase;
import android.text.TextUtils;

import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.imsservice.mmtel.base.IFDNTracker;
import com.android.imsstack.imsservice.mmtel.ut.UtFactory;
import com.android.imsstack.imsservice.mmtel.ut.base.UtInterface;
import com.android.imsstack.imsservice.mmtel.ut.base.UtListener;
import com.android.imsstack.util.ImsLog;

import java.util.Arrays;

public final class ImsUtImpl extends ImsUtImplBase {
    private static final boolean DBG = ImsLog.isDebuggable();

    private final IBaseContext mContext;
    private ImsUtListener mListener = null;
    private UtInterface mUt = null;

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
        mUt.setListener(new UtListenerProxy());
    }

    public void clear() {
        if (mUt == null) {
            return;
        }

        mUt.setListener(null);
        UtFactory.getInstance().releaseUtInterface(mContext.getSlotId());
        mUt = null;
    }

    public UtInterface getUtInterface() {
        return mUt;
    }

    private boolean isServiceBlockedByFDN(int action, int serviceClass, int condition) {
        IFDNTracker fdnTracker = getFDNTracker();
        if (null == fdnTracker) {
            return false;
        }

        return fdnTracker.isCallBlockedByFDN(new ImsMMICode(action, serviceClass, condition)
                .getMMIString());
    }

    private boolean isServiceBlockedByFDN(int action, int serviceClass, int condition,
            String number, int timeSeconds) {
        IFDNTracker fdnTracker = getFDNTracker();
        if (null == fdnTracker) {
            return false;
        }

        return fdnTracker.isCallBlockedByFDN(new ImsMMICode(action, serviceClass, condition,
                number, timeSeconds).getMMIString());
    }

    IFDNTracker getFDNTracker() {
        if (mContext instanceof ImsCallContext) {
            return ((ImsCallContext)mContext).getFDNTracker();
        }

        return new ImsFDNTracker(mContext);
    }

    @Override
    public void close() {
        if (mUt == null) {
            return;
        }

        UtFactory.getInstance().releaseUtInterface(mContext.getSlotId());
        mUt = null;
    }

    @Override
    public int queryCallBarring(int cbType) {
        if (DBG) {
            log("queryCallBarring :: cbType=" + cbType);
        }

        if (mUt == null) {
            return (ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1));
        }

        if (isServiceBlockedByFDN(ImsMMICode.ACTION_INTERROGATE, ImsMMICode.CATEGORY_CB, cbType)) {
            log("Call Blocked by FDN");
            return (ImsReasonInfo.CODE_FDN_BLOCKED * (-1));  // blocked by FDN
        }

        return mUt.queryCallBarring(cbType);
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

        if (isServiceBlockedByFDN(ImsMMICode.ACTION_INTERROGATE, ImsMMICode.CATEGORY_CB, cbType)) {
            log("Call Blocked by FDN");
            return (ImsReasonInfo.CODE_FDN_BLOCKED * (-1));  // blocked by FDN
        }

        return mUt.queryCallBarringForServiceClass(cbType, serviceClass);
    }

    @Override
    public int queryCallForward(int condition, String number) {
        if (DBG) {
            log("queryCallForward :: condition=" + condition + ", number=" + number);
        }

        if (mUt == null) {
            return (ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1));
        }

        if (isServiceBlockedByFDN(ImsMMICode.ACTION_INTERROGATE, ImsMMICode.CATEGORY_CF,
                condition)) {
            log("Call Blocked by FDN");
            return (ImsReasonInfo.CODE_FDN_BLOCKED * (-1));  // blocked by FDN
        }

        return mUt.queryCallForward(condition, number);
    }

    @Override
    public int queryCallWaiting() {
        if (DBG) {
            log("queryCallWaiting :: ");
        }

        if (mUt == null) {
            return (ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1));
        }

        if (isServiceBlockedByFDN(ImsMMICode.ACTION_INTERROGATE, ImsMMICode.CATEGORY_CW, 0)) {
            log("Call Blocked by FDN");
            return (ImsReasonInfo.CODE_FDN_BLOCKED * (-1));  // blocked by FDN
        }

        return mUt.queryCallWaiting();
    }

    @Override
    public int queryCLIR() {
        if (DBG) {
            log("queryCLIR :: ");
        }

        if (mUt == null) {
            return (ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1));
        }

        if (isServiceBlockedByFDN(ImsMMICode.ACTION_INTERROGATE, ImsMMICode.CATEGORY_OIR, 0)) {
            log("Call Blocked by FDN");
            return (ImsReasonInfo.CODE_FDN_BLOCKED * (-1));  // blocked by FDN
        }

        return mUt.queryCLIR();
    }

    @Override
    public int queryCLIP() {
        if (DBG) {
            log("queryCLIP :: ");
        }

        if (mUt == null) {
            return (ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1));
        }

        if (isServiceBlockedByFDN(ImsMMICode.ACTION_INTERROGATE, ImsMMICode.CATEGORY_OIP, 0)) {
            log("Call Blocked by FDN");
            return (ImsReasonInfo.CODE_FDN_BLOCKED * (-1));  // blocked by FDN
        }

        return mUt.queryCLIP();
    }

    @Override
    public int queryCOLR() {
        if (DBG) {
            log("queryCOLR :: ");
        }

        if (mUt == null) {
            return (ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1));
        }

        if (isServiceBlockedByFDN(ImsMMICode.ACTION_INTERROGATE, ImsMMICode.CATEGORY_TIR, 0)) {
            log("Call Blocked by FDN");
            return (ImsReasonInfo.CODE_FDN_BLOCKED * (-1));  // blocked by FDN
        }

        return mUt.queryCOLR();
    }

    @Override
    public int queryCOLP() {
        if (DBG) {
            log("queryCOLP :: ");
        }

        if (mUt == null) {
            return (ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1));
        }

        if (isServiceBlockedByFDN(ImsMMICode.ACTION_INTERROGATE, ImsMMICode.CATEGORY_TIP, 0)) {
            log("Call Blocked by FDN");
            return (ImsReasonInfo.CODE_FDN_BLOCKED * (-1));  // blocked by FDN
        }

        return mUt.queryCOLP();
    }

    @Override
    public int transact(Bundle ssInfo) {
        if (DBG) {
            log("transact :: ssInfo=" + ssInfo);
        }

        if (mUt == null) {
            return (ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1));
        }

        return mUt.transact(ssInfo);
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

        if (isServiceBlockedByFDN(action, ImsMMICode.CATEGORY_CB, cbType)) {
            log("Call Blocked by FDN");
            return (ImsReasonInfo.CODE_FDN_BLOCKED * (-1));  // blocked by FDN
        }

        return mUt.updateCallBarring(cbType, action, barringList);
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

        if (isServiceBlockedByFDN(action, ImsMMICode.CATEGORY_CB, cbType)) {
            log("Call Blocked by FDN");
            return (ImsReasonInfo.CODE_FDN_BLOCKED * (-1));  // blocked by FDN
        }

        return mUt.updateCallBarringForServiceClass(cbType, action, barringList, serviceClass);
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

        if (isServiceBlockedByFDN(action, ImsMMICode.CATEGORY_CB, cbType)) {
            log("Call Blocked by FDN");
            return (ImsReasonInfo.CODE_FDN_BLOCKED * (-1));  // blocked by FDN
        }

        return mUt.updateCallBarringWithPassword(cbType, action, barringList, serviceClass,
                password);
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

        if (isServiceBlockedByFDN(action, ImsMMICode.CATEGORY_CF, condition, number, timeSeconds)) {
            log("Call Blocked by FDN");
            return (ImsReasonInfo.CODE_FDN_BLOCKED * (-1));  // blocked by FDN
        }

        return mUt.updateCallForward(action, condition, number, serviceClass, timeSeconds);
    }

    @Override
    public int updateCallWaiting(boolean enable, int serviceClass) {
        if (DBG) {
            log("updateCallWaiting :: enable=" + enable + ", serviceClass=" + serviceClass);
        }

        if (mUt == null) {
            return (ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1));
        }

        int action = enable ? ImsMMICode.ACTION_ACTIVATE : ImsMMICode.ACTION_DEACTIVATE;
        if (isServiceBlockedByFDN(action, ImsMMICode.CATEGORY_CW, 0)) {
            log("Call Blocked by FDN");
            return (ImsReasonInfo.CODE_FDN_BLOCKED * (-1));  // blocked by FDN
        }

        return mUt.updateCallWaiting(enable, serviceClass);
    }

    @Override
    public int updateCLIR(int clirMode) {
        if (DBG) {
            log("updateCLIR :: clirMode=" + clirMode);
        }

        if (mUt == null) {
            return (ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1));
        }

        int action  = ImsMMICode.ACTION_NONE;
        if (clirMode == 1) {
            action = ImsMMICode.ACTION_ACTIVATE;
        } else if (clirMode == 2) {
            action = ImsMMICode.ACTION_DEACTIVATE;
        }

        if (isServiceBlockedByFDN(action, ImsMMICode.CATEGORY_OIR, 0)) {
            log("Call Blocked by FDN");
            return (ImsReasonInfo.CODE_FDN_BLOCKED * (-1));  // blocked by FDN
        }

        return mUt.updateCLIR(clirMode);
    }

    @Override
    public int updateCLIP(boolean enable) {
        if (DBG) {
            log("updateCLIP :: enable=" + enable);
        }

        if (mUt == null) {
            return (ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1));
        }

        int action = enable ? ImsMMICode.ACTION_ACTIVATE : ImsMMICode.ACTION_DEACTIVATE;
        if (isServiceBlockedByFDN(action, ImsMMICode.CATEGORY_OIP, 0)) {
            log("Call Blocked by FDN");
            return (ImsReasonInfo.CODE_FDN_BLOCKED * (-1));  // blocked by FDN
        }

        return mUt.updateCLIP(enable);
    }

    @Override
    public int updateCOLR(int presentation) {
        if (DBG) {
            log("updateCOLR :: presentation=" + presentation);
        }

        if (mUt == null) {
            return (ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1));
        }

        int action = (presentation == 0) ?
                ImsMMICode.ACTION_DEACTIVATE : ImsMMICode.ACTION_ACTIVATE;
        if (isServiceBlockedByFDN(action, ImsMMICode.CATEGORY_TIR, 0)) {
            log("Call Blocked by FDN");
            return (ImsReasonInfo.CODE_FDN_BLOCKED * (-1));  // blocked by FDN
        }

        return mUt.updateCOLR(presentation);
    }

    @Override
    public int updateCOLP(boolean enable) {
        if (DBG) {
            log("updateCOLP :: enable=" + enable);
        }

        if (mUt == null) {
            return (ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1));
        }

        int action = enable ? ImsMMICode.ACTION_ACTIVATE : ImsMMICode.ACTION_DEACTIVATE;
        if (isServiceBlockedByFDN(action, ImsMMICode.CATEGORY_TIP, 0)) {
            log("Call Blocked by FDN");
            return (ImsReasonInfo.CODE_FDN_BLOCKED * (-1));  // blocked by FDN
        }

        return mUt.updateCOLP(enable);
    }

    @Override
    public void setListener(ImsUtListener listener) {
        mListener = listener;
    }

    public boolean isUtAvailable() {
        return (mUt != null) ? mUt.isUtAvailable() : false;
    }

    private void postAndRunTask(Runnable task) {
        mContext.getExecutor().execute(task);
    }

    private void log(Throwable t, String message) {
        if (t instanceof DeadObjectException) {
            mListener = null;
        } else if (mListener != null) {
            ImsLog.e("[GII-IMPL] " + message + t, t);
        }
    }

    private static void log(String s) {
        ImsLog.d("[GII-IMPL] " + s);
    }

    private class UtListenerProxy extends UtListener {
        @Override
        public void utConfigurationUpdated(final int id) {
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
        public void utConfigurationUpdateFailed(final int id, final ImsReasonInfo reasonInfo) {
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
        public void lineIdentificationSupplementaryServiceResponse(final int id, ImsSsInfo ssInfo) {
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
        public void utConfigurationCallBarringQueried(final int id, final ImsSsInfo[] cbInfo) {
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
        public void utConfigurationCallForwardQueried(final int id,
                final ImsCallForwardInfo[] cfInfo) {
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
        public void utConfigurationCallWaitingQueried(final int id, final ImsSsInfo[] cwInfo) {
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
        public void utConfigurationQueryFailed(final int id, final ImsReasonInfo reasonInfo) {
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
