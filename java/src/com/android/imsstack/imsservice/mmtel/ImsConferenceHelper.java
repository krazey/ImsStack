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

import android.telephony.ims.stub.ImsCallSessionImplBase;

import com.android.imsstack.enabler.mtc.MtcCall;
import com.android.imsstack.imsservice.mmtel.base.ICallContext;
import com.android.imsstack.imsservice.mmtel.internal.ConferenceExtensionProxy;
import com.android.imsstack.imsservice.mmtel.internal.ConferenceProxy;
import com.android.imsstack.imsservice.mmtel.internal.MergeProxy;
import com.android.imsstack.util.ImsLog;

import java.util.ArrayList;

public final class ImsConferenceHelper implements ConferenceProxy.DisposalCallback {
    private static ImsConferenceHelper sConferenceHelper = new ImsConferenceHelper();

    private final ArrayList<ConferenceProxy> mConferenceProxys
            = new ArrayList<ConferenceProxy>();
    private ImsCallSessionImplBase mTransientConferenceSession = null;
    // To recover background session when foreground session is terminated
    // before call merge is completed.
    private ImsCallSessionImplBase mBackgroundSession = null;

    private ImsConferenceHelper() {
    }

    public static ImsConferenceHelper getInstance() {
        return sConferenceHelper;
    }

    @Override
    public void onConferenceProxyDisposed(ConferenceProxy confProxy) {
        int size = mConferenceProxys.size();
        mConferenceProxys.remove(confProxy);
        log("onConferenceProxyDisposed :: size - "
                + size + " >> " + mConferenceProxys.size());

        if (mConferenceProxys.isEmpty()) {
            setBackgroundSession(null);
        }

        confProxy.dispose();
        confProxy = null;
    }

    public ImsCallSessionImplBase getBackgroundSession() {
        return mBackgroundSession;
    }

    public ImsCallSessionImplBase getTransientConferenceSession() {
        return mTransientConferenceSession;
    }

    public void setBackgroundSession(ImsCallSessionImplBase session) {
        mBackgroundSession = session;
    }

    public void setTransientConferenceSession(ImsCallSessionImplBase session) {
        mTransientConferenceSession = session;
    }

    public boolean extendToConference(ICallContext callContext, String[] participants) {
        ImsCallApp callApp = (ImsCallApp)callContext.getApp();
        ImsCallManager cm = callApp.getCallManager();
        ImsCallSessionImpl fgCallSession = cm.getActiveSession();

        if (fgCallSession == null) {
            loge("ImsConferenceHelper :: fgCall=" + fgCallSession);
            return false;
        }

        MtcCall fgCall = fgCallSession.getMtcCall();

        if (fgCall == null) {
            loge("ImsConferenceHelper :: fgCall=" + fgCall);
            return false;
        }

        ConferenceProxy confProxy = new ConferenceExtensionProxy(
                callContext, fgCall, participants);

        confProxy.setDisposalCallback(this);

        fgCallSession.setConferenceProxy(confProxy);

        if (!confProxy.start(cm.getMtcApp(), false)) {
            fgCallSession.setConferenceProxy(null);
            loge("ImsConferenceHelper :: extendToConference failed");
            return false;
        }

        mConferenceProxys.add(confProxy);

        return true;
    }

    public boolean merge(ICallContext callContext) {
        ImsCallApp callApp = (ImsCallApp)callContext.getApp();
        ImsCallManager cm = callApp.getCallManager();
        ImsCallSessionImpl fgCallSession = cm.getActiveSession();
        ImsCallSessionImpl bgCallSession = cm.getHoldSession();

        if (fgCallSession == null || bgCallSession == null) {
            loge("ImsConferenceHelper :: fgCall="
                    + fgCallSession + ", bgCall=" + bgCallSession);
            return false;
        }

        MtcCall fgCall = fgCallSession.getMtcCall();
        MtcCall bgCall = bgCallSession.getMtcCall();

        if (fgCall == null || bgCall == null) {
            loge("ImsConferenceHelper :: fgCall="
                    + fgCall + ", bgCall=" + bgCall);
            return false;
        }

        ConferenceProxy confProxy = new MergeProxy(callContext, fgCall, bgCall);

        confProxy.setDisposalCallback(this);

        fgCallSession.setConferenceProxy(confProxy);
        bgCallSession.setConferenceProxy(confProxy);

        if (!confProxy.start(cm.getMtcApp(), true)) {
            fgCallSession.setConferenceProxy(null);
            bgCallSession.setConferenceProxy(null);
            loge("ImsConferenceHelper :: merge failed");
            return false;
        }

        setBackgroundSession(bgCallSession);

        mConferenceProxys.add(confProxy);

        return true;
    }

    private static void log(String s) {
        ImsLog.d("[ISIL] " + s);
    }

    private static void loge(String s) {
        ImsLog.e("[ISIL] " + s);
    }
}
