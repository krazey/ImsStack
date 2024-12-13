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

import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Parcel;

import com.android.imsstack.enabler.mtc.conf.IUConf;
import com.android.imsstack.enabler.mtc.conf.UsersInfo;
import com.android.imsstack.util.ImsArgs;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

public class MtcConference {
    /**
     * Listener class for call operations via proxy
     */
    public static interface ProxyListener {
        public void onCallProxyMerge(MtcConference confCall,
                MtcConference hostCall, MtcConference peerCall);
        public void onCallProxyExtendToConference(MtcConference confCall,
                MtcConference hostCall, String[] participants);
    }

    /**
     * Listener for MtcConference
     */
    public static class Listener implements ProxyListener {
        @Override
        public void onCallProxyMerge(MtcConference confCall,
                MtcConference hostCall, MtcConference peerCall) {
            // no-op
        }

        @Override
        public void onCallProxyExtendToConference(MtcConference confCall,
                MtcConference hostCall, String[] participants) {
            // no-op
        }

        public void onCallMergeStarted(MtcConference call, MtcConference confCall,
                CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            // no-op
        }

        public void onCallMerged(MtcConference call,
                CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo, UsersInfo usersInfo) {
            // no-op
        }

        public void onCallMergeFailed(MtcConference call,
                CallReasonInfo callReasonInfo) {
            // no-op
        }

        public void onCallConferenceExtended(MtcConference call, long confCallId,
                CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            // no-op
        }

        public void onCallConferenceExtendFailed(MtcConference call,
                CallReasonInfo callReasonInfo) {
            // no-op
        }

        public void onCallConferenceExtendReceived(MtcConference call, long confCallId,
                CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            // no-op
        }

        public void onCallInviteParticipantsRequestDelivered(MtcConference call) {
            // no-op
        }

        public void onCallInviteParticipantsRequestFailed(MtcConference call,
                CallReasonInfo callReasonInfo) {
            // no-op
        }

        public void onCallRemoveParticipantsRequestDelivered(MtcConference call) {
            // no-op
        }

        public void onCallRemoveParticipantsRequestFailed(MtcConference call,
                CallReasonInfo callReasonInfo) {
            // no-op
        }

        public void onCallConferenceStateUpdated(MtcConference call,
                UsersInfo usersInfo) {
            // no-op
        }

        /**
         * Device's internal events.
         */
        // If the operation is successfully done, the CallReasonInfo will be a null.
        public void onCallDeleteParticipantsRequestCompleted(MtcConference call,
                CallReasonInfo callReasonInfo) {
            // no-op
        }
    }



    /**
     * Internal messages
     * Requests from application : 101 ~
     */
    /** Args: Parcel */
    private static final int MSG_SEND_REQUEST = 101;

    private final Object mLock = new Object();
    private final Call mParent;
    private final ConferenceTracker mCT;
    private final MessageHandler mHandler;
    private MtcConference.Listener mListener = null;
    private boolean mDisposed = false;
    private MtcJniProxy mMtcJniProxy;

    public MtcConference(Looper looper, Call parent, ConferenceTracker ct) {
        mHandler = new MessageHandler(looper);

        mCT = ct;
        mParent = parent;
        mMtcJniProxy = MtcJniProxy.getInstance();
    }

    @VisibleForTesting
    public void setMtcJniProxy(MtcJniProxy mtcJniProxy) {
        mMtcJniProxy = mtcJniProxy;
    }

    public void dispose() {
        if (!isConferenceValid()) {
            return;
        }

        log("dispose");

        synchronized (mLock) {
            mListener = null;
            mDisposed = true;
        }
    }

    public Call getParent() {
        return mParent;
    }

    public boolean isSameCall(MtcConference c) {
        return (mParent != null) ? mParent.equals((c != null) ? c.getParent() : null) : false;
    }

    public void extendToConference(UsersInfo usersInfo) {
        if (!isConferenceValid()) {
            if (mListener != null) {
                mListener.onCallConferenceExtendFailed(this,
                        new CallReasonInfo(CallReasonInfo.CODE_LOCAL_INTERNAL_ERROR, 0, ""));
            }
            return;
        }

        log("extendToConference :: " + MtcCallUtils.toString(usersInfo));

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IUConf.EXPAND);

        usersInfo.writeToParcel(parcel, 1);

        sendRequest(parcel);

        mCT.updateConferenceState(this,
                ConferenceTracker.EVENT_EXTEND_TO_CONFERENCE,
                ImsArgs.obtain(usersInfo, null, null));
    }

    public void merge(UsersInfo usersInfo) {
        if (!isConferenceValid()) {
            return;
        }

        log("merge :: ");

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IUConf.MERGE);

        usersInfo.writeToParcel(parcel, 1);

        sendRequest(parcel);

        mCT.updateConferenceState(this,
                ConferenceTracker.EVENT_MERGE,
                ImsArgs.obtain(usersInfo, null, null));
    }

    public void inviteParticipants(UsersInfo usersInfo) {
        if (!isConferenceValid()) {
            return;
        }

        log("inviteParticipants :: " + MtcCallUtils.toString(usersInfo));

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IUConf.JOIN);

        usersInfo.writeToParcel(parcel, 1);

        sendRequest(parcel);

        mCT.updateConferenceState(this,
                ConferenceTracker.EVENT_INVITE_PARTICIPANTS,
                ImsArgs.obtain(usersInfo, null, null));
    }

    public void removeParticipants(UsersInfo usersInfo) {
        if (!isConferenceValid()) {
            return;
        }

        log("removeParticipants :: " + MtcCallUtils.toString(usersInfo));

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IUConf.DROP);

        usersInfo.writeToParcel(parcel, 1);

        sendRequest(parcel);

        mCT.updateConferenceState(this,
                ConferenceTracker.EVENT_REMOVE_PARTICIPANTS,
                ImsArgs.obtain(usersInfo, null, null));
    }

    public void deleteParticipants(UsersInfo usersInfo) {
        if (!isConferenceValid()) {
            return;
        }

        log("deleteParticipants :: " + MtcCallUtils.toString(usersInfo));

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IUConf.DELETE);

        usersInfo.writeToParcel(parcel, 1);

        sendRequest(parcel);

        mCT.updateConferenceState(this,
                ConferenceTracker.EVENT_DELETE_PARTICIPANTS,
                ImsArgs.obtain(usersInfo, null, null));
    }

    public void setListener(MtcConference.Listener listener) {
        synchronized (mLock) {
            mListener = listener;
        }
    }

    public void handleMessage(int msg, Parcel parcel) {
        Listener listener = null;

        synchronized (mLock) {
            listener = mListener;
        }

        // Checks if the listener is alive
        if (listener == null) {
            return;
        }

        switch (msg) {
        case IUConf.EXPANDED:
        {
            CallInfo callInfo = new CallInfo(parcel);
            MediaInfo mediaInfo = new MediaInfo(parcel);
            SuppInfo suppInfo = new SuppInfo(parcel);
            long confCallId = 0; //parcel.readLong();

            logi("EXPANDED :: Call Id=" + Long.toHexString(confCallId)
                    + ", " + MtcCallUtils.toString(callInfo)
                    + ", " + MtcCallUtils.toString(mediaInfo)
                    + ", " + MtcCallUtils.toString(suppInfo));

            ImsArgs args = ImsArgs.obtain(callInfo, mediaInfo, suppInfo);
            args.mLongArg = confCallId;

            mCT.updateConferenceState(this,
                    ConferenceTracker.EVENT_EXTENDED,
                    args);

            listener.onCallConferenceExtended(this,
                    confCallId, callInfo, mediaInfo, suppInfo);
            break;
        }

        case IUConf.EXPANDFAILED:
        {
                CallReasonInfo callReasonInfo = new CallReasonInfo(parcel);

                logi("EXPANDFAILED :: " + MtcCallUtils.toString(callReasonInfo));

                mCT.updateConferenceState(this,
                        ConferenceTracker.EVENT_EXTEND_FAILED,
                        ImsArgs.obtain(callReasonInfo, null, null));

                listener.onCallConferenceExtendFailed(this, callReasonInfo);
                break;
        }

        case IUConf.EXPANDED_BY:
        {
            CallInfo callInfo = new CallInfo(parcel);
            MediaInfo mediaInfo = new MediaInfo(parcel);
            SuppInfo suppInfo = new SuppInfo(parcel);
            long confCallId = parcel.readLong();

            logi("EXPANDED_BY :: confCallId=" + Long.toHexString(confCallId)
                    + ", " + MtcCallUtils.toString(callInfo)
                    + ", " + MtcCallUtils.toString(mediaInfo)
                    + ", " + MtcCallUtils.toString(suppInfo));

            ImsArgs args = ImsArgs.obtain(callInfo, mediaInfo, suppInfo);
            args.mLongArg = confCallId;

            mCT.updateConferenceState(this,
                    ConferenceTracker.EVENT_EXTEND_RECEIVED,
                    args);

            listener.onCallConferenceExtendReceived(this,
                    confCallId, callInfo, mediaInfo, suppInfo);
            break;
        }

        case IUConf.MERGED:
        {
            CallInfo callInfo = new CallInfo(parcel);
            MediaInfo mediaInfo = new MediaInfo(parcel);
            SuppInfo suppInfo = new SuppInfo(parcel);
            UsersInfo usersInfo = new UsersInfo(parcel);

            logi("MERGED :: " + MtcCallUtils.toString(callInfo)
                    + ", " + MtcCallUtils.toString(mediaInfo)
                    + ", " + MtcCallUtils.toString(suppInfo)
                    + ", " + MtcCallUtils.toString(usersInfo));

            mCT.updateConferenceState(this,
                    ConferenceTracker.EVENT_MERGED,
                    ImsArgs.obtain(callInfo, mediaInfo, suppInfo));

            listener.onCallMerged(this, callInfo, mediaInfo, suppInfo, usersInfo);
            break;
        }

        case IUConf.MERGEFAILED:
        {
                CallReasonInfo callReasonInfo = new CallReasonInfo(parcel);

                logi("MERGEFAILED :: " + MtcCallUtils.toString(callReasonInfo));

                mCT.updateConferenceState(this,
                        ConferenceTracker.EVENT_MERGE_FAILED,
                        ImsArgs.obtain(callReasonInfo, null, null));

                listener.onCallMergeFailed(this, callReasonInfo);
                break;
        }

        case IUConf.JOINED:
        {
                int result = parcel.readInt();
                logi("JOINED :: result=" + result);

                if (result != 0) {
                    mCT.updateConferenceState(this,
                            ConferenceTracker.EVENT_INVITE_PARTICIPANTS_REQUEST_DELIVERED,
                            null);

                    listener.onCallInviteParticipantsRequestDelivered(this);
                } else {
                    CallReasonInfo callReasonInfo = new CallReasonInfo(parcel);
                    mCT.updateConferenceState(this,
                            ConferenceTracker.EVENT_INVITE_PARTICIPANTS_REQUEST_FAILED,
                            ImsArgs.obtain(callReasonInfo, null, null));

                    listener.onCallInviteParticipantsRequestFailed(this, callReasonInfo);
                }
                break;
        }

        case IUConf.DROPPED:
        {
                int result = parcel.readInt();
                logi("DROPPED :: result=" + result);

                if (result != 0) {
                    mCT.updateConferenceState(this,
                            ConferenceTracker.EVENT_REMOVE_PARTICIPANTS_REQUEST_DELIVERED,
                            null);

                    listener.onCallRemoveParticipantsRequestDelivered(this);
                } else {
                    CallReasonInfo callReasonInfo = new CallReasonInfo(parcel);
                    mCT.updateConferenceState(this,
                            ConferenceTracker.EVENT_REMOVE_PARTICIPANTS_REQUEST_FAILED,
                            ImsArgs.obtain(callReasonInfo, null, null));

                    listener.onCallRemoveParticipantsRequestFailed(this, callReasonInfo);
                }
                break;
        }

        case IUConf.DELETED:
        {
                int result = parcel.readInt();
                CallReasonInfo callReasonInfo = new CallReasonInfo(parcel);

                logi("DELETED :: result=" + result
                        + " " + ((result == 0) ? MtcCallUtils.toString(callReasonInfo) : ""));

                mCT.updateConferenceState(this,
                        ConferenceTracker.EVENT_DELETE_PARTICIPANTS_REQUEST_COMPLETED,
                        (result == 0) ? ImsArgs.obtain(callReasonInfo, null, null) : null);

                listener.onCallDeleteParticipantsRequestCompleted(this,
                        (result == 0) ? callReasonInfo : null);
                break;
        }

        case IUConf.NOTIFY_USERS_INFO:
        {
            UsersInfo usersInfo = new UsersInfo(parcel);

            logi("NOTIFY_USERS_INFO :: " + MtcCallUtils.toString(usersInfo));

            listener.onCallConferenceStateUpdated(this, usersInfo);
            break;
        }

        default:
            break;
        }
    }

    public static boolean isMessageForConference(int msg) {
        return (msg == IUConf.EXPANDED)
                || (msg == IUConf.EXPANDFAILED)
                || (msg == IUConf.EXPANDED_BY)
                || (msg == IUConf.MERGED)
                || (msg == IUConf.MERGEFAILED)
                || (msg == IUConf.JOINED)
                || (msg == IUConf.DROPPED)
                || (msg == IUConf.DELETED)
                || (msg == IUConf.NOTIFY_USERS_INFO);
    }

    private boolean isConferenceValid() {
        synchronized (mLock) {
            return !mDisposed;
        }
    }

    private void sendRequest(Parcel parcel) {
        Message.obtain(mHandler, MSG_SEND_REQUEST, parcel).sendToTarget();
    }

    private static void log(String s) {
        ImsLog.d("[GII-MTC] " + s);
    }

    private static void logi(String s) {
        ImsLog.i("[GII-MTC] " + s);
    }

    /**
     * Handles the requests from the application, and the messages from the native layer.
     * All the JNI operations will be handled in a single thread("ImsCallHandlerThread").
     */
    private class MessageHandler extends Handler {
        public MessageHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_SEND_REQUEST: {
                    log("MtcConference :: sendRequest");

                    Parcel parcel = (Parcel)msg.obj;

                    mMtcJniProxy.sendDataToNative(mParent.getNativeCallId(), parcel);
                    break;
                }
                default:
                    // no-op
                    break;
            }
        }
    }
}
