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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.anyBoolean;
import static org.mockito.Mockito.anyInt;
import static org.mockito.Mockito.anyLong;
import static org.mockito.Mockito.atLeastOnce;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.eq;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;

import android.os.Looper;
import android.os.Parcel;
import android.telephony.CallQuality;
import android.telephony.CarrierConfigManager;
import android.telephony.ims.RtpHeaderExtension;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import com.android.imsstack.ImsStackTest;
import com.android.imsstack.base.AppContext;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.media.MediaTestUtils;
import com.android.imsstack.enabler.mtc.conf.IUConf;
import com.android.imsstack.enabler.mtc.conf.UsersInfo;
import com.android.imsstack.jni.JniImsListener;
import com.android.imsstack.util.ImsArgs;
import com.android.imsstack.util.ImsLog;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Captor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.Random;
import java.util.Set;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class MtcCallTest extends ImsStackTest {
    private final int mInvalidCommand = -1;
    private final String mCallee = "123";
    private final String mActualCallee = "456";

    @Mock private IBaseContext mBaseContext;
    @Mock private CallTracker mCT;
    @Mock private MtcConference mMtcConference;
    @Mock private MtcMediaSession mMtcMediaSession;
    @Mock private MtcCall.Listener mListener;
    @Mock private MtcCall.IEmergencyCallFailureListener mEmergencyCallFailureListener;
    @Mock private CallInfo mCallInfo;
    @Mock private MediaInfo mMediaInfo;
    @Mock private MtcJniProxy mMtcJniProxy;
    @Mock private MtcCall mConferenceMtcCall;
    @Mock private ConfigInterface mConfigInterface;
    @Mock private CarrierConfig mCarrierConfig;
    @Captor ArgumentCaptor<JniImsListener> mJNIImsListenerCaptor;

    private int mCommand;
    private boolean mClearInterface;
    private String mCcid;

    private TestMtcJniProxy mTestMtcJniProxy;
    private TestMtcCall mTestMtcCall;
    private TestMtcCall mTestMtcCallWithMockJniProxy;

    private class TestMtcJniProxy extends MtcJniProxy {
        @Override
        public long getJniInterfaceAndSetListener(
                int nSlot, int category, JniImsListener listener) {
            return (long) 1;
        }

        @Override
        public void releaseJniInterfaceAndrRemoveListener(long nativeObj, JniImsListener listener) {
            mClearInterface = true;
        }

        @Override
        public void sendDataToNative(long nativeObj, Parcel parcel) {
            if (parcel == null) {
                return;
            }

            parcel.setDataPosition(0);
            mCommand = parcel.readInt();

            parcel.recycle();
            parcel = null;
        }
    }

    private class TestMtcCall extends MtcCall {
        TestMtcCall(IBaseContext context, CallTracker ct, int callAttributes, int index,
                String logTag) {
            super(context, ct, callAttributes, index, logTag);
        }

        TestMtcCall(IBaseContext context, CallTracker ct, int index, String logTag, Looper looper,
                MtcConference mtcConference, MtcMediaSession mtcMediaSession,
                MtcJniProxy mtcJniProxy, CallInfo callInfo, MediaInfo mediaInfo) {
            super(context, ct, index, logTag, looper, mtcConference, mtcMediaSession, mtcJniProxy,
                    callInfo, mediaInfo);
        }

        @Override
        public String getCallId() {
            return mCcid;
        }

        @Override
        protected void initMedia() {
            mMediaSession = mMtcMediaSession;
            mAudioListener = new AudioSessionListener();
            mTextListener = new TextSessionListener();
        }

        @Override
        protected MtcCall createAndSetMtcCallForConference(
                long jniConfCallId, CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            return mConferenceMtcCall;
        }
    }

    @Before
    public void setUp() throws Exception {
        super.setUp(getClass().getSimpleName());

        mCommand = mInvalidCommand;
        mClearInterface = false;
        mCcid = "mCcid";
        MockitoAnnotations.initMocks(this);
        AppContext.init(mContext);

        doReturn((long) 1).when(mMtcJniProxy).getJniInterfaceAndSetListener(
                    anyInt(), anyInt(), any());
        doReturn(0).when(mBaseContext).getSlotId();
        AgentFactory.getInstance().setAgent(ConfigInterface.class, mConfigInterface, 0);
        doReturn(mCarrierConfig).when(mConfigInterface).getCarrierConfig();
        doReturn(true).when(mCarrierConfig).getBoolean(
                CarrierConfigManager.KEY_RTT_SUPPORTED_BOOL, false);
        doReturn(Looper.myLooper()).when(mBaseContext).getCallLooper();
        doReturn(0).when(mBaseContext).getSlotId();

        mTestMtcJniProxy = new TestMtcJniProxy();
        mTestMtcCall = new TestMtcCall(mBaseContext, mCT, 0, "", Looper.myLooper(),
                mMtcConference, mMtcMediaSession, mTestMtcJniProxy, mCallInfo, mMediaInfo);
        mTestMtcCallWithMockJniProxy = new TestMtcCall(mBaseContext, mCT, 0, "", Looper.myLooper(),
                mMtcConference, mMtcMediaSession, mMtcJniProxy, mCallInfo, mMediaInfo);

        ImsLog.setDebugOn(true);
    }

    @After
    public void tearDown() throws Exception {
        mTestMtcJniProxy = null;
        mTestMtcCall = null;
        mTestMtcCallWithMockJniProxy = null;
        super.tearDown();
        AppContext.deinit();
    }

    public void sendMessageToJniListener(int command) {
        mTestMtcCallWithMockJniProxy.createNativeCallObject();
        mTestMtcCallWithMockJniProxy.setListener(mListener);

        verify(mMtcJniProxy, atLeastOnce()).getJniInterfaceAndSetListener(anyInt(), anyInt(),
                mJNIImsListenerCaptor.capture());

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(command);
        fillInfo(parcel);
        parcel.setDataPosition(0);
        mJNIImsListenerCaptor.getValue().onMessage(parcel);
        processAllMessages();
        parcel.recycle();
    }

    private void fillInfo(Parcel parcel) {
        Random randomGenerator = new Random();
        parcel.writeInt(randomGenerator.ints(IUMtcCall.SERVICETYPE_NORMAL,
                IUMtcCall.SERVICETYPE_NORMAL + 1).findFirst().getAsInt());
        parcel.writeInt(randomGenerator.ints(IUMtcCall.CALLTYPE_VOIP,
                IUMtcCall.CALLTYPE_VIDEO_RTT + 1).findFirst().getAsInt());
        parcel.writeInt(randomGenerator.ints(0, 2).findFirst().getAsInt());
        parcel.writeInt(randomGenerator.ints(0, 2).findFirst().getAsInt());
        parcel.writeInt(randomGenerator.ints(0, 2).findFirst().getAsInt());
        parcel.writeInt(randomGenerator.ints(0, 2).findFirst().getAsInt());
        parcel.writeInt(randomGenerator.ints(0, 2).findFirst().getAsInt());
        parcel.writeInt(randomGenerator.ints(0, 2).findFirst().getAsInt());
        parcel.writeInt(randomGenerator.ints(0, 2).findFirst().getAsInt());
        parcel.writeInt(randomGenerator.ints(0, 2).findFirst().getAsInt());

        parcel.writeInt(randomGenerator.ints(MediaInfo.AUDIO_QUALITY_AMR_NB,
                MediaInfo.AUDIO_QUALITY_MAX).findFirst().getAsInt());
        parcel.writeInt(randomGenerator.ints(MediaInfo.VIDEO_QUALITY_QCIF,
                MediaInfo.VIDEO_QUALITY_NOTUSED).findFirst().getAsInt());
        parcel.writeInt(randomGenerator.ints(MediaInfo.DIRECTION_RECEIVE,
                MediaInfo.DIRECTION_SEND_RECEIVE + 1).findFirst().getAsInt());
        parcel.writeInt(randomGenerator.ints(MediaInfo.DIRECTION_RECEIVE,
                MediaInfo.DIRECTION_SEND_RECEIVE + 1).findFirst().getAsInt());
        parcel.writeInt(randomGenerator.ints(MediaInfo.DIRECTION_RECEIVE,
                MediaInfo.DIRECTION_SEND_RECEIVE + 1).findFirst().getAsInt());
        parcel.writeInt(randomGenerator.ints(MediaInfo.GTTMODE_FULL,
                MediaInfo.GTTMODE_VCO + 1).findFirst().getAsInt());

        parcel.writeInt(2);
        parcel.writeInt(SuppInfo.SUPP_TYPE_CALLERID);
        parcel.writeString("");
        parcel.writeInt(randomGenerator.ints(SuppInfo.CALLERID_NETWORK,
                SuppInfo.CALLERID_IDENTITY + 1).findFirst().getAsInt());
        parcel.writeInt(0);
        parcel.writeInt(SuppInfo.SUPP_TYPE_CNAP);
        parcel.writeString("test");
        parcel.writeInt(0);
        parcel.writeInt(0);
    }

    public void sendMessageToJniListener(int command, int send) {
        mTestMtcCallWithMockJniProxy.createNativeCallObject();
        mTestMtcCallWithMockJniProxy.setListener(mListener);

        verify(mMtcJniProxy, atLeastOnce()).getJniInterfaceAndSetListener(anyInt(), anyInt(),
                mJNIImsListenerCaptor.capture());

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(command);
        parcel.writeInt(send);
        parcel.setDataPosition(0);
        mJNIImsListenerCaptor.getValue().onMessage(parcel);
        processAllMessages();
        parcel.recycle();
    }

    private void sendMessageToJniListener(int command, long longValue, int intValue) {
        mTestMtcCallWithMockJniProxy.createNativeCallObject();
        mTestMtcCallWithMockJniProxy.setListener(mListener);

        verify(mMtcJniProxy, atLeastOnce()).getJniInterfaceAndSetListener(anyInt(), anyInt(),
                mJNIImsListenerCaptor.capture());

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(command);
        fillInfo(parcel);
        parcel.writeLong(longValue);
        parcel.writeInt(intValue);
        parcel.setDataPosition(0);
        mJNIImsListenerCaptor.getValue().onMessage(parcel);
        processAllMessages();
        parcel.recycle();
    }

    private void sendMessageToJniListener(int command, CallReasonInfo callReasonInfo) {
        mTestMtcCallWithMockJniProxy.createNativeCallObject();
        mTestMtcCallWithMockJniProxy.setListener(mListener);
        mTestMtcCallWithMockJniProxy.setEmergencyCallFailureListener(mEmergencyCallFailureListener);

        verify(mMtcJniProxy, atLeastOnce()).getJniInterfaceAndSetListener(anyInt(), anyInt(),
                mJNIImsListenerCaptor.capture());

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(command);
        callReasonInfo.writeToParcel(parcel, 0);
        parcel.setDataPosition(0);
        mJNIImsListenerCaptor.getValue().onMessage(parcel);
        processAllMessages();
        parcel.recycle();
    }

    @Test
    public void testConstructor() {
        TestMtcCall MtcCallEmergency = new TestMtcCall(
                mBaseContext, mCT, MtcCall.FLAG_EMERGENCY, 0, "");

        assertTrue(MtcCallEmergency.isOnPreIncoming());
        assertTrue(MtcCallEmergency.isEmergencyCall());

        int fullAttrubutes = MtcCall.FLAG_EMERGENCY | MtcCall.FLAG_CONFERENCE
                | MtcCall.FLAG_WIFI_EMERGENCY | MtcCall.FLAG_VIDEO_CALL | MtcCall.FLAG_MO
                | MtcCall.FLAG_RTT;

        TestMtcCall MtcCallFullAttributes = new TestMtcCall(
                mBaseContext, mCT, fullAttrubutes, 0, "MO");

        assertTrue(MtcCallFullAttributes.hasCallExtra(MtcCall.EXTRA_E_CALL));
        assertTrue(MtcCallFullAttributes.isMO());
        assertTrue(MtcCallFullAttributes.isEmergencyCall());
        assertTrue(MtcCallEmergency.equals(MtcCallEmergency));
        assertFalse(MtcCallEmergency.equals(MtcCallFullAttributes));
    }

    @Test
    public void testClose() {
        mTestMtcCall.close();

        verifyNoMoreInteractions(mMtcConference);

        mTestMtcCall.setCallExtraBoolean(Call.EXTRA_E_CALL, true);

        mTestMtcCall.close();
        processAllMessages();

        verify(mMtcConference, times(1)).dispose();
        verify(mMtcMediaSession, times(3)).onMessage(any(Parcel.class));
        verify(mMtcMediaSession, times(1)).setAudioListener(eq(null));
        verify(mMtcMediaSession, times(1)).setTextListener(eq(null));
        verify(mMtcMediaSession, times(1)).dispose();
        assertTrue(mClearInterface);
        verify(mCT, times(1)).updateCallState(
                eq(mTestMtcCall), eq(CallTracker.CALL_EVENT_DESTROY), any());

        mTestMtcCall.createNativeCallObject();

        mTestMtcCall.close();
        processAllMessages();

        verify(mMtcConference, times(2)).dispose();
        verify(mMtcMediaSession, times(6)).onMessage(any(Parcel.class));
        verify(mMtcMediaSession, times(2)).setAudioListener(eq(null));
        verify(mMtcMediaSession, times(2)).setTextListener(eq(null));
        verify(mMtcMediaSession, times(2)).dispose();
        verify(mCT, times(2)).updateCallState(
                eq(mTestMtcCall), eq(CallTracker.CALL_EVENT_DESTROY), any());
    }

    @Test
    public void testTerminate() {
        assertFalse(mTestMtcCall.isCallValid());
        mTestMtcCall.terminate(1, true);
        processAllMessages();

        assertEquals(mInvalidCommand, mCommand);

        mTestMtcCall.createNativeCallObject();
        assertTrue(mTestMtcCall.isCallValid());
        mTestMtcCall.setCallState(CallTracker.CALL_STATE_OFFHOOK);
        mTestMtcCall.setOnHeld(true);
        mTestMtcCall.setOnHold(true);
        mTestMtcCall.terminate(1, true);
        processAllMessages();

        assertFalse(mTestMtcCall.isOnHeld());
        assertFalse(mTestMtcCall.isOnHold());
        assertEquals(CallTracker.CALL_STATE_IDLE, mTestMtcCall.getCallState());
        assertEquals(IUMtcCall.TERMINATE, mCommand);

        verify(mCT, times(1)).updateCallState(
                eq(mTestMtcCall), eq(CallTracker.CALL_EVENT_TERMINATING), any());

        mTestMtcCall.setListener(mListener);
        mTestMtcCall.terminate(1, true);
        processAllMessages();
        verify(mListener, times(1)).onCallTerminated(eq(mTestMtcCall), any());

        mTestMtcCall.terminate(1, false);
        verifyNoMoreInteractions(mListener);
    }

    @Test
    public void testTerminate2() {
        assertFalse(mTestMtcCall.isCallValid());
        mTestMtcCall.terminate(1);
        processAllMessages();

        assertEquals(mInvalidCommand, mCommand);

        mTestMtcCall.createNativeCallObject();
        assertTrue(mTestMtcCall.isCallValid());
        mTestMtcCall.setCallState(CallTracker.CALL_STATE_OFFHOOK);
        mTestMtcCall.setOnHeld(true);
        mTestMtcCall.setOnHold(true);
        mTestMtcCall.terminate(1);
        processAllMessages();

        assertFalse(mTestMtcCall.isOnHeld());
        assertFalse(mTestMtcCall.isOnHold());
        assertEquals(CallTracker.CALL_STATE_IDLE, mTestMtcCall.getCallState());
        assertEquals(IUMtcCall.TERMINATE, mCommand);

        verify(mCT, times(1)).updateCallState(
                eq(mTestMtcCall), eq(CallTracker.CALL_EVENT_TERMINATING), any());
    }

    @Test
    public void testUpdateConferenceStateExtended() {
        ImsArgs confArg = ImsArgs.obtain(new CallInfo(), new MediaInfo(), new SuppInfo());
        confArg.mLongArg = 0;

        mTestMtcCall.updateConferenceState(
                mMtcConference, ConferenceTracker.EVENT_EXTENDED, confArg);

        assertEquals(CallTracker.CALL_STATE_OFFHOOK, mTestMtcCall.getCallState());
        assertTrue(mTestMtcCall.isAdhocGroup());
        assertNotNull(ConferenceInfoHelper.getConferenceInfo(mCcid));

        confArg = ImsArgs.obtain(new CallInfo(), new MediaInfo(), new SuppInfo());
        confArg.mLongArg = 1;

        mTestMtcCall.updateConferenceState(
                mMtcConference, ConferenceTracker.EVENT_EXTENDED, confArg);

        verify(mCT, times(1)).updateCallState(
                eq(mConferenceMtcCall), eq(CallTracker.CALL_EVENT_CREATE), any());
    }

    @Test
    public void testUpdateConferenceStateExtendReceived() {
        ImsArgs confArg = ImsArgs.obtain(new CallInfo(), new MediaInfo(), new SuppInfo());
        confArg.mLongArg = 0;

        mTestMtcCall.updateConferenceState(
                mMtcConference, ConferenceTracker.EVENT_EXTEND_RECEIVED, confArg);

        assertNotNull(ConferenceInfoHelper.getConferenceInfo(mCcid));

        confArg = ImsArgs.obtain(new CallInfo(), new MediaInfo(), new SuppInfo());
        confArg.mLongArg = 1;

        mTestMtcCall.updateConferenceState(
                mMtcConference, ConferenceTracker.EVENT_EXTEND_RECEIVED, confArg);

        verify(mCT, times(1)).updateCallState(
                eq(mConferenceMtcCall), eq(CallTracker.CALL_EVENT_CREATE), any());
    }

    @Test
    public void testUpdateConferenceStateMerged() {
        ImsArgs confArg = ImsArgs.obtain(new CallInfo(), new MediaInfo(), new SuppInfo());
        confArg.mLongArg = 0;
        mTestMtcCall.updateConferenceState(
                mMtcConference, ConferenceTracker.EVENT_MERGED, confArg);

        assertEquals(CallTracker.CALL_STATE_OFFHOOK, mTestMtcCall.getCallState());

        verify(mCT, times(1)).updateCallState(
                eq(mTestMtcCall), eq(CallTracker.CALL_EVENT_ESTABLISHED), any());
    }

    @Test
    public void testDetach() {
        assertFalse(mTestMtcCall.isCallValid());
        mTestMtcCall.detach();

        verifyNoMoreInteractions(mCT);

        mTestMtcCall.createNativeCallObject();
        assertTrue(mTestMtcCall.isCallValid());
        mTestMtcCall.setCallState(CallTracker.CALL_STATE_OFFHOOK);
        mTestMtcCall.detach();

        assertEquals(CallTracker.CALL_STATE_IDLE, mTestMtcCall.getCallState());
        assertTrue(mTestMtcCall.hasDetails(Call.Details.DETACHED));
        verify(mCT, times(1)).updateCallState(
                eq(mTestMtcCall), eq(CallTracker.CALL_EVENT_DESTROY), any());
    }

    @Test
    public void testGetConferenceInterface() {
        assertEquals(mMtcConference, mTestMtcCall.getConferenceInterface());
    }

    @Test
    public void testGetMediaInfo() {
        assertEquals(mMediaInfo, mTestMtcCall.getMediaInfo());
    }

    @Test
    public void testGetCallInfo() {
        assertEquals(mCallInfo, mTestMtcCall.getCallInfo());
    }

    @Test
    public void testSetListener() {
        mTestMtcCall.setCallTerminated();
        mTestMtcCall.setListener(mListener);
        processAllMessages();

        verify(mListener, times(1)).onCallStartFailed(eq(mTestMtcCall), any());

        mTestMtcCall.setCallState(CallTracker.CALL_STATE_OFFHOOK);
        mTestMtcCall.setListener(mListener);
        processAllMessages();

        verify(mListener, times(1)).onCallTerminated(eq(mTestMtcCall), any());
    }

    @Test
    public void testOpen() {
        mTestMtcCall.open(IUMtcCall.SERVICETYPE_EMERGENCY,
                IUMtcCall.EMERGENCYTYPE_EMERGENCY_ROUTING, true, true, false);
        processAllMessages();
        assertEquals(mInvalidCommand, mCommand);

        mTestMtcCall.createNativeCallObject();

        mTestMtcCall.open(IUMtcCall.SERVICETYPE_EMERGENCY,
                IUMtcCall.EMERGENCYTYPE_EMERGENCY_ROUTING, true, true, false);
        processAllMessages();

        assertEquals(IUMtcCall.OPEN, mCommand);
    }

    @Test
    public void testStart() {
        mTestMtcCall.createNativeCallObject();

        mTestMtcCall.start(
                IUMtcCall.CALLTYPE_VT, mCallee, mActualCallee, new MediaInfo(), new SuppInfo());
        processAllMessages();

        assertEquals(IUMtcCall.CALLTYPE_VT, mTestMtcCall.getCallType());
        assertEquals(mCallee, mTestMtcCall.getRemoteNumber());
        assertEquals(IUMtcCall.START, mCommand);
        verify(mCT, times(1)).updateCallState(
                eq(mTestMtcCall), eq(CallTracker.CALL_EVENT_ESTABLISHING), eq(null));
        verifyNoMoreInteractions(mListener);
        assertEquals(mActualCallee, mTestMtcCall.getCallExtra(mTestMtcCall.EXTRA_TI_ORIGIN, ""));

        mTestMtcCall.setListener(mListener);

        mTestMtcCall.start(IUMtcCall.CALLTYPE_VT, mCallee, "", new MediaInfo(), new SuppInfo());
        processAllMessages();

        assertEquals(mCallee, mTestMtcCall.getCallExtra(mTestMtcCall.EXTRA_TI_ORIGIN, ""));
    }

    @Test
    public void testStartDoesNotNotifyInitiating() {
        mTestMtcCall.createNativeCallObject();
        mTestMtcCall.setListener(mListener);
        mTestMtcCall.start(IUMtcCall.CALLTYPE_VT, mCallee, "", new MediaInfo(), new SuppInfo());
        processAllMessages();

        verify(mListener, times(0)).onCallInitiating(eq(mTestMtcCall), any(), any(), anyInt());
    }

    @Test
    public void testStartConference() {
        mTestMtcCall.createNativeCallObject();
        mTestMtcCall.setListener(mListener);
        UsersInfo usersInfo = new UsersInfo();
        usersInfo.addUser(new UsersInfo.User());
        mTestMtcCall.startConference(
                IUMtcCall.CALLTYPE_VOIP, usersInfo, new MediaInfo(), new SuppInfo());
        processAllMessages();

        assertEquals(true, mTestMtcCall.getCallExtraBoolean(mTestMtcCall.EXTRA_CONFERENCE, false));
        assertNotNull(ConferenceInfoHelper.getConferenceInfo(mTestMtcCall.getCallId()));
        assertEquals(IUMtcCall.STARTCONF, mCommand);
        verify(mCT, times(1)).updateCallState(
                eq(mTestMtcCall), eq(CallTracker.CALL_EVENT_ESTABLISHING), eq(null));
    }

    @Test
    public void testStartConferenceDoesNotNotifyInitiating() {
        mTestMtcCall.createNativeCallObject();
        mTestMtcCall.setListener(mListener);
        UsersInfo usersInfo = new UsersInfo();
        usersInfo.addUser(new UsersInfo.User());
        mTestMtcCall.startConference(
                IUMtcCall.CALLTYPE_VOIP, usersInfo, new MediaInfo(), new SuppInfo());
        processAllMessages();

        verify(mListener, times(0)).onCallInitiating(eq(mTestMtcCall), any(), any(), anyInt());
    }

    @Test
    public void testAttach() {
        mTestMtcCall.createNativeCallObject();
        mTestMtcCall.attach(1);
        processAllMessages();

        assertEquals(IUMtcCall.ATTACH, mCommand);
    }

    @Test
    public void testAlertUser() {
        mTestMtcCall.createNativeCallObject();
        mTestMtcCall.alertUser();
        processAllMessages();

        assertEquals(CallTracker.CALL_STATE_RINGING, mTestMtcCall.getCallState());
        assertEquals(IUMtcCall.USER_ALERT, mCommand);
        verify(mCT, times(1)).updateCallState(
                eq(mTestMtcCall), eq(CallTracker.CALL_EVENT_RINGING), eq(null));
    }

    @Test
    public void testAccept() {
        mTestMtcCall.createNativeCallObject();

        mTestMtcCall.accept(IUMtcCall.CALLTYPE_VOIP, new MediaInfo());
        processAllMessages();

        assertEquals(IUMtcCall.ACCEPT, mCommand);
        assertEquals(CallTracker.CALL_STATE_OFFHOOK, mTestMtcCall.getCallState());
        verify(mCT, times(1)).updateCallState(
                eq(mTestMtcCall), eq(CallTracker.CALL_EVENT_ACCEPT), eq(null));

        mTestMtcCall.setUpdateState(mTestMtcCall.UPDATE_STATE_RESUME_RECEIVED);

        mTestMtcCall.accept(IUMtcCall.CALLTYPE_VOIP, new MediaInfo());
        processAllMessages();

        assertEquals(IUMtcCall.ACCEPT_RESUME, mCommand);
        assertEquals(mTestMtcCall.UPDATE_STATE_RESUME_ACCEPTED, mTestMtcCall.getUpdateState());

        mTestMtcCall.setUpdateState(mTestMtcCall.UPDATE_STATE_RECEIVED);

        mTestMtcCall.accept(IUMtcCall.CALLTYPE_VOIP, new MediaInfo());
        processAllMessages();

        assertEquals(IUMtcCall.ACCEPT_UPDATE, mCommand);
        assertEquals(mTestMtcCall.UPDATE_STATE_ACCEPTED, mTestMtcCall.getUpdateState());
    }

    @Test
    public void testAcceptRaceCondition() {
        mTestMtcCall.createNativeCallObject();
        mTestMtcCall.setCallState(CallTracker.CALL_STATE_RINGING);
        mTestMtcCall.setListener(mListener);
        mTestMtcCall.setCallTerminated();

        mTestMtcCall.accept(IUMtcCall.CALLTYPE_VOIP, new MediaInfo());
        processAllMessages();

        assertEquals(CallTracker.CALL_STATE_IDLE, mTestMtcCall.getCallState());
        verify(mCT, times(1)).updateCallState(
                eq(mTestMtcCall), eq(CallTracker.CALL_EVENT_TERMINATED), eq(null));
        verify(mListener, times(1)).onCallStartFailed(eq(mTestMtcCall), any());
    }

    @Test
    public void testReject() {
        mTestMtcCall.createNativeCallObject();

        mTestMtcCall.reject(1);
        processAllMessages();

        assertEquals(IUMtcCall.REJECT, mCommand);

        mTestMtcCall.setCallState(CallTracker.CALL_STATE_OFFHOOK);
        mTestMtcCall.setUpdateState(mTestMtcCall.UPDATE_STATE_RESUME_RECEIVED);

        mTestMtcCall.reject(1);
        processAllMessages();

        assertEquals(IUMtcCall.REJECT_RESUME, mCommand);
        assertEquals(mTestMtcCall.UPDATE_STATE_REJECTED, mTestMtcCall.getUpdateState());

        mTestMtcCall.setUpdateState(mTestMtcCall.UPDATE_STATE_RECEIVED);

        mTestMtcCall.reject(1);
        processAllMessages();

        assertEquals(IUMtcCall.REJECT_UPDATE, mCommand);
    }

    @Test
    public void testHold() {
        mTestMtcCall.createNativeCallObject();
        mTestMtcCall.setListener(mListener);

        mTestMtcCall.hold(new MediaInfo());
        processAllMessages();

        assertEquals(IUMtcCall.HOLD, mCommand);

        mTestMtcCall.setCallTerminated();

        mTestMtcCall.hold(new MediaInfo());
        processAllMessages();

        verify(mListener, times(1)).onCallHoldFailed(eq(mTestMtcCall), any());
    }

    @Test
    public void testResume() {
        mTestMtcCall.createNativeCallObject();
        mTestMtcCall.setListener(mListener);

        mTestMtcCall.resume(new MediaInfo());
        processAllMessages();

        assertEquals(IUMtcCall.RESUME, mCommand);

        mTestMtcCall.setCallTerminated();

        mTestMtcCall.resume(new MediaInfo());
        processAllMessages();

        verify(mListener, times(1)).onCallResumeFailed(eq(mTestMtcCall), any());
    }

    @Test
    public void testUpdate() {
        mTestMtcCall.createNativeCallObject();
        mTestMtcCall.setListener(mListener);

        mTestMtcCall.update(IUMtcCall.CALLTYPE_VT, new MediaInfo());
        processAllMessages();

        assertEquals(IUMtcCall.UPDATE, mCommand);
        assertEquals(mTestMtcCall.UPDATE_STATE_SENT, mTestMtcCall.getUpdateState());

        mTestMtcCall.setUpdateState(mTestMtcCall.UPDATE_STATE_RECEIVED);

        mTestMtcCall.update(IUMtcCall.CALLTYPE_VT, new MediaInfo());
        processAllMessages();

        verify(mListener, times(1)).onCallUpdateFailed(eq(mTestMtcCall), any());

        mTestMtcCall.setCallTerminated();

        mTestMtcCall.update(IUMtcCall.CALLTYPE_VT, new MediaInfo());
        processAllMessages();

        verify(mListener, times(2)).onCallUpdateFailed(eq(mTestMtcCall), any());
    }

    @Test
    public void testTransfer() {
        mTestMtcCall.createNativeCallObject();

        mTestMtcCall.transfer("");
        processAllMessages();

        assertEquals(IUMtcCall.REQUEST_ECT, mCommand);

        mTestMtcCall.transfer(mCallee);
        processAllMessages();

        assertEquals(IUMtcCall.REQUEST_ECT_BLIND, mCommand);
    }

    @Test
    public void testRequestCallPush() {
        mTestMtcCall.createNativeCallObject();

        mTestMtcCall.requestCallPush(mCallee);
        processAllMessages();

        assertEquals(IUMtcCall.REQUEST_CALL_PUSH, mCommand);
    }

    @Test
    public void testCancelCallPush() {
        mTestMtcCall.createNativeCallObject();

        mTestMtcCall.cancelCallPush();
        processAllMessages();

        assertEquals(IUMtcCall.CANCEL_CALL_PUSH, mCommand);
    }

    @Test
    public void testSendDtmf() {
        mTestMtcCall.createNativeCallObject();

        mTestMtcCall.sendDtmf('1');
        processAllMessages();

        verify(mMtcMediaSession, times(1)).sendDtmf('1');
    }

    @Test
    public void testSendRttMessage() {
        mTestMtcCall.sendRttMessage(mCallee);

        verify(mMtcMediaSession, times(1)).sendRttMessage(any());
    }

    @Test
    public void testSendRtpHeaderExtensions() {
        Set<RtpHeaderExtension> extensions = MediaTestUtils.createRtpExtensionsSet();
        mTestMtcCall.sendRtpHeaderExtensions(extensions);

        verify(mMtcMediaSession, times(1)).sendRtpHeaderExtensions(eq(extensions));
    }

    @Test
    public void testSendUssd() {
        mTestMtcCall.createNativeCallObject();

        mTestMtcCall.sendUssd("");
        processAllMessages();

        assertEquals(IUMtcCall.SEND_USSD, mCommand);
    }

    @Test
    public void testNotifyAnbr() {
        mTestMtcCall.notifyAnbr(1, 1, 244);

        verify(mMtcMediaSession, times(1)).notifyAnbr(eq(1), eq(1), eq(244));
    }

    @Test
    public void testUpdateConferenceUserIdWithEmptyValue() {
        int anonymousId = 100;
        ConferenceInfoHelper.setAnonymousId(anonymousId);
        mTestMtcCall.updateConferenceUserId("");

        assertEquals(mTestMtcCall.getCallExtra(mTestMtcCall.EXTRA_CONFERENCE_USER_ID, ""),
                Call.ANONYMOUS + anonymousId + "@anonymous.invalid");
    }

    @Test
    public void testGetMediaSession() {
        assertEquals(mMtcMediaSession, mTestMtcCall.getMediaSession());
    }

    @Test
    public void testInvokeIncomingCallReceivedForAutoRejecting() {
        mTestMtcCall.setListener(mListener);
        Parcel parcel = Parcel.obtain();
        mTestMtcCall.invokeIncomingCallReceivedForAutoRejecting(new IncomingMtcCall(parcel));
        parcel.recycle();

        verify(mCT, times(1)).updateCallState(eq(mTestMtcCall),
                eq(CallTracker.CALL_EVENT_INCOMING_RECEIVED), eq(null));
        verify(mListener, times(1)).onCallIncomingReceived(eq(mTestMtcCall), any(), anyInt());
    }

    @Test
    public void testJniListenerStarted() {
        sendMessageToJniListener(IUMtcCall.STARTED);

        assertEquals(CallTracker.CALL_STATE_OFFHOOK, mTestMtcCallWithMockJniProxy.getCallState());
        verify(mCT, times(1)).updateCallState(
                eq(mTestMtcCallWithMockJniProxy), eq(CallTracker.CALL_EVENT_ESTABLISHED), eq(null));
        verify(mListener, times(1)).onCallStarted(
                eq(mTestMtcCallWithMockJniProxy), any(), any(), any());
    }

    @Test
    public void testJniListenerStartFailed() {
        mTestMtcCallWithMockJniProxy.setCallState(CallTracker.CALL_STATE_RINGBACK);
        sendMessageToJniListener(IUMtcCall.START_FAILED);

        assertEquals(CallTracker.CALL_STATE_IDLE, mTestMtcCallWithMockJniProxy.getCallState());
        verify(mCT, times(1)).updateCallState(
                eq(mTestMtcCallWithMockJniProxy), eq(CallTracker.CALL_EVENT_TERMINATED), eq(null));
        verify(mListener, times(1)).onCallStartFailed(eq(mTestMtcCallWithMockJniProxy), any());

        mTestMtcCallWithMockJniProxy.setDetails(Call.Details.ON_PRE_INCOMING, true);
        sendMessageToJniListener(IUMtcCall.START_FAILED);

        verify(mCT, times(1)).updateCallState(
                eq(mTestMtcCallWithMockJniProxy), eq(CallTracker.CALL_EVENT_TERMINATED), eq(null));
    }

    @Test
    public void testJniListenerStartFailedWithNoListener() {
        mTestMtcCallWithMockJniProxy.createNativeCallObject();

        verify(mMtcJniProxy, times(1)).getJniInterfaceAndSetListener(anyInt(), anyInt(),
                mJNIImsListenerCaptor.capture());

        mTestMtcCallWithMockJniProxy.setCallState(CallTracker.CALL_STATE_RINGBACK);
        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IUMtcCall.START_FAILED);
        parcel.setDataPosition(0);
        mJNIImsListenerCaptor.getValue().onMessage(parcel);
        processAllMessages();
        parcel.recycle();

        assertNotNull(mTestMtcCallWithMockJniProxy.getTerminationReason());
        assertEquals(CallTracker.CALL_STATE_IDLE, mTestMtcCallWithMockJniProxy.getCallState());
        verify(mCT, times(1)).updateCallState(
                eq(mTestMtcCallWithMockJniProxy), eq(CallTracker.CALL_EVENT_TERMINATED), eq(null));
        verify(mMtcJniProxy, times(1)).releaseJniInterfaceAndrRemoveListener(anyLong(), any());
    }

    @Test
    public void testJniListenerStartFailedByAlreadyOpenedServiceClosed() {
        mTestMtcCallWithMockJniProxy.open(IUMtcCall.SERVICETYPE_EMERGENCY,
                IUMtcCall.EMERGENCYTYPE_EMERGENCY_ROUTING, true, true, true);
        mTestMtcCallWithMockJniProxy.setCallState(CallTracker.CALL_STATE_RINGBACK);
        CallReasonInfo callReasonInfo = new CallReasonInfo(
                    CallReasonInfo.CODE_LOCAL_CALL_CS_RETRY_REQUIRED,
                    CallReasonInfo.EXTRA_CODE_CALL_RETRY_EMERGENCY,
                    CallReasonInfo.EXTRA_MESSAGE_AOS_DISCONNECTED);
        sendMessageToJniListener(IUMtcCall.START_FAILED, callReasonInfo);

        verify(mEmergencyCallFailureListener, times(1))
                .onEmergencyCallFailedByAlreadyOpenedServiceClosed();
    }

    @Test
    public void testJniListenerStartFailedWhenNewServiceClosed() {
        mTestMtcCallWithMockJniProxy.open(IUMtcCall.SERVICETYPE_EMERGENCY,
                IUMtcCall.EMERGENCYTYPE_EMERGENCY_ROUTING, true, true, false);
        mTestMtcCallWithMockJniProxy.setCallState(CallTracker.CALL_STATE_RINGBACK);
        CallReasonInfo callReasonInfo = new CallReasonInfo(
                    CallReasonInfo.CODE_LOCAL_CALL_CS_RETRY_REQUIRED,
                    CallReasonInfo.EXTRA_CODE_CALL_RETRY_EMERGENCY,
                    CallReasonInfo.EXTRA_MESSAGE_AOS_DISCONNECTED);
        sendMessageToJniListener(IUMtcCall.START_FAILED, callReasonInfo);

        verify(mEmergencyCallFailureListener, times(0))
                .onEmergencyCallFailedByAlreadyOpenedServiceClosed();
    }

    @Test
    public void testJniListenerStartFailedByNotServiceClosed() {
        mTestMtcCallWithMockJniProxy.open(IUMtcCall.SERVICETYPE_EMERGENCY,
                IUMtcCall.EMERGENCYTYPE_EMERGENCY_ROUTING, true, true, true);
        mTestMtcCallWithMockJniProxy.setCallState(CallTracker.CALL_STATE_RINGBACK);
        CallReasonInfo callReasonInfo = new CallReasonInfo(
                    CallReasonInfo.CODE_LOCAL_CALL_CS_RETRY_REQUIRED,
                    CallReasonInfo.EXTRA_CODE_CALL_RETRY_EMERGENCY, "");
        sendMessageToJniListener(IUMtcCall.START_FAILED, callReasonInfo);

        verify(mEmergencyCallFailureListener, times(0))
                .onEmergencyCallFailedByAlreadyOpenedServiceClosed();
    }

    @Test
    public void testJniListenerInitiating() {
        sendMessageToJniListener(IUMtcCall.INITIATING);

        assertEquals(CallTracker.CALL_STATE_IDLE, mTestMtcCallWithMockJniProxy.getCallState());
        verify(mListener, times(1)).onCallInitiating(
                eq(mTestMtcCallWithMockJniProxy), any(), any(), anyInt());
    }

    @Test
    public void testJniListenerProgressing() {
        sendMessageToJniListener(IUMtcCall.PROGRESSING);

        assertEquals(CallTracker.CALL_STATE_RINGBACK, mTestMtcCallWithMockJniProxy.getCallState());
        verify(mListener, times(1)).onCallProgressing(
                eq(mTestMtcCallWithMockJniProxy), any(), any(), any());
    }

    @Test
    public void testJniListenerHeld() {
        mTestMtcCallWithMockJniProxy.setCallState(CallTracker.CALL_STATE_OFFHOOK);
        sendMessageToJniListener(IUMtcCall.HELD);

        assertTrue(mTestMtcCallWithMockJniProxy.isOnHold());
        verify(mCT, times(1)).updateCallState(
                eq(mTestMtcCallWithMockJniProxy), eq(CallTracker.CALL_EVENT_UPDATED), eq(null));
        verify(mListener, times(1)).onCallHeld(
                eq(mTestMtcCallWithMockJniProxy), any(), any(), any());
    }

    @Test
    public void testJniListenerHoldFailed() {
        sendMessageToJniListener(IUMtcCall.HOLD_FAILED);

        verify(mListener, times(1)).onCallHoldFailed(eq(mTestMtcCallWithMockJniProxy), any());
    }

    @Test
    public void testJniListenerHeldBy() {
        mTestMtcCallWithMockJniProxy.setCallState(CallTracker.CALL_STATE_OFFHOOK);
        sendMessageToJniListener(IUMtcCall.HELD_BY);

        assertTrue(mTestMtcCallWithMockJniProxy.isOnHeld());

        verify(mCT, times(1)).updateCallState(
                eq(mTestMtcCallWithMockJniProxy), eq(CallTracker.CALL_EVENT_UPDATED), eq(null));
        verify(mListener, times(1)).onCallHoldReceived(eq(
                mTestMtcCallWithMockJniProxy), any(), any(), any());
    }

    @Test
    public void testJniListenerResumed() {
        sendMessageToJniListener(IUMtcCall.RESUMED);

        verify(mCT, times(1)).updateCallState(
                eq(mTestMtcCallWithMockJniProxy), eq(CallTracker.CALL_EVENT_UPDATED), eq(null));
        verify(mListener, times(1)).onCallResumed(
                eq(mTestMtcCallWithMockJniProxy), any(), any(), any());
    }

    @Test
    public void testJniListenerResumeFailed() {
        sendMessageToJniListener(IUMtcCall.RESUME_FAILED);

        verify(mListener, times(1)).onCallResumeFailed(eq(mTestMtcCallWithMockJniProxy), any());
    }

    @Test
    public void testJniListenerResumedBy() {
        sendMessageToJniListener(IUMtcCall.RESUMED_BY);

        verify(mCT, times(1)).updateCallState(
                eq(mTestMtcCallWithMockJniProxy), eq(CallTracker.CALL_EVENT_UPDATED), eq(null));
        verify(mListener, times(1)).onCallResumeReceived(
                eq(mTestMtcCallWithMockJniProxy), any(), any(), any());
    }

    @Test
    public void testJniListenerTerminated() {
        sendMessageToJniListener(IUMtcCall.TERMINATED);

        verify(mCT, times(1)).updateCallState(
                eq(mTestMtcCallWithMockJniProxy), eq(CallTracker.CALL_EVENT_TERMINATED), eq(null));
        verify(mListener, times(1)).onCallTerminated(eq(mTestMtcCallWithMockJniProxy), any());
    }

    @Test
    public void testJniListenerIncomingUpdate() {
        sendMessageToJniListener(IUMtcCall.INCOMING_UPDATE);

        assertEquals(mTestMtcCall.UPDATE_STATE_RECEIVED,
                mTestMtcCallWithMockJniProxy.getUpdateState());

        verify(mListener, times(1)).onCallUpdateReceived(
                eq(mTestMtcCallWithMockJniProxy), any(), any(), any());

        mTestMtcCallWithMockJniProxy.setUpdateState(mTestMtcCallWithMockJniProxy.UPDATE_STATE_SENT);

        sendMessageToJniListener(IUMtcCall.INCOMING_UPDATE);

        verify(mMtcJniProxy, times(1)).sendDataToNative(anyLong(), any());
    }

    @Test
    public void testJniListenerUpdated() {
        sendMessageToJniListener(IUMtcCall.UPDATED);

        verify(mCT, times(1)).updateCallState(
                eq(mTestMtcCallWithMockJniProxy), eq(CallTracker.CALL_EVENT_UPDATED), eq(null));
        verify(mListener, times(1)).onCallUpdated(
                eq(mTestMtcCallWithMockJniProxy), any(), any(), any());

        mTestMtcCallWithMockJniProxy.setUpdateState(MtcCall.UPDATE_STATE_ACCEPTED);
        sendMessageToJniListener(IUMtcCall.UPDATED);

        verify(mCT, times(2)).updateCallState(
                eq(mTestMtcCallWithMockJniProxy), eq(CallTracker.CALL_EVENT_UPDATED), eq(null));
        verify(mListener, times(2)).onCallUpdated(
                eq(mTestMtcCallWithMockJniProxy), any(), any(), any());
    }

    @Test
    public void testJniListenerUpdateFailed() {
        sendMessageToJniListener(IUMtcCall.UPDATE_FAILED);

        verify(mListener, times(1)).onCallUpdateFailed(eq(mTestMtcCallWithMockJniProxy), any());
    }

    @Test
    public void testJniListenerUpdatedBy() {
        sendMessageToJniListener(IUMtcCall.UPDATED_BY);

        verify(mCT, times(1)).updateCallState(
                eq(mTestMtcCallWithMockJniProxy), eq(CallTracker.CALL_EVENT_UPDATED), eq(null));
        verify(mListener, times(1)).onCallAutoUpdated(
                eq(mTestMtcCallWithMockJniProxy), any(), any(), any());
    }

    @Test
    public void testJniListenerNotifyInfo() {
        sendMessageToJniListener(IUMtcCall.NOTIFY_INFO, 110);

        verify(mMtcMediaSession, times(1)).notifyMediaInfoChanged(eq(110), anyInt(), any());

        sendMessageToJniListener(IUMtcCall.NOTIFY_INFO);

        verify(mListener, times(1)).onCallInfoUpdated(
                eq(mTestMtcCallWithMockJniProxy), anyInt(), any(), anyInt(), anyBoolean());
    }

    @Test
    public void testJniListenerDefaultConference() {
        sendMessageToJniListener(IUConf.NOTIFY_USERS_INFO);

        verify(mMtcConference, times(1)).handleMessage(anyInt(), any());
    }

    @Test
    public void testJniListenerIncomingResume() {
        sendMessageToJniListener(IUMtcCall.INCOMING_RESUME);

        verify(mListener, times(1)).onCallUpdateResumeReceived(
                eq(mTestMtcCallWithMockJniProxy), any(), any(), any());
    }

    @Test
    public void testJniListenerIncomingCallReceived() {
        sendMessageToJniListener(IUMtcCall.INCOMING_CALL_RECEIVED);

        verify(mCT, times(1)).updateCallState(eq(mTestMtcCallWithMockJniProxy),
                eq(CallTracker.CALL_EVENT_INCOMING_RECEIVED), eq(null));
        verify(mListener, times(1)).onCallIncomingReceived(
                eq(mTestMtcCallWithMockJniProxy), any(), anyInt());
    }

    @Test
    public void testJniListenerEctCompleted() {
        sendMessageToJniListener(IUMtcCall.ECT_COMPLETED, 1);

        verify(mListener, times(1)).onCallTransferred(eq(mTestMtcCallWithMockJniProxy));

        sendMessageToJniListener(IUMtcCall.ECT_COMPLETED, 0);

        verify(mListener, times(1)).onCallTransferFailed(eq(mTestMtcCallWithMockJniProxy), any());
    }

    @Test
    public void testJniListenerReplacedBy() {
        sendMessageToJniListener(IUMtcCall.REPLACED_BY, 0, IUMtcCall.REPLACED_BY_TYPE_ECT);

        verify(mListener, times(1)).onCallTransferReceived(eq(mTestMtcCallWithMockJniProxy), any(),
                any(), any(), any());
    }

    @Test
    public void testJniListenerCallPushCompleted() {
        sendMessageToJniListener(IUMtcCall.CALL_PUSH_COMPLETED, 1);

        verify(mListener, times(1)).onCallPushRequestCompleted(eq(mTestMtcCallWithMockJniProxy));

        sendMessageToJniListener(IUMtcCall.CALL_PUSH_COMPLETED, 0);

        verify(mListener, times(1)).onCallPushRequestFailed(
                eq(mTestMtcCallWithMockJniProxy), any());
    }

    @Test
    public void testGetCallId() {
        assertNotNull(mTestMtcCall.getCallId());
    }

    @Test
    public void testAudioSessionListener() {
        mTestMtcCall.setListener(mListener);

        mTestMtcCall.getAudioListener().onAudioSessionOpened();
        processAllMessages();

        verify(mListener, times(1)).onAudioSessionOpened(eq(mTestMtcCall));

        mTestMtcCall.getAudioListener().onAudioSessionClosed();
        processAllMessages();

        verify(mListener, times(1)).onAudioSessionClosed(eq(mTestMtcCall));

        mTestMtcCall.getAudioListener().onCallQualityChanged(new CallQuality());
        processAllMessages();

        verify(mListener, times(1)).onCallQualityChanged(eq(mTestMtcCall), any());

        Set<RtpHeaderExtension> extensions = MediaTestUtils.createRtpExtensionsSet();
        mTestMtcCall.getAudioListener().onRtpHeaderExtensionsReceived(extensions);
        processAllMessages();

        verify(mListener, times(1)).onCallRtpHeaderExtensionsReceived(
                eq(mTestMtcCall), eq(extensions));
    }

    @Test
    public void testTextSessionListener() {
        mTestMtcCall.setListener(mListener);

        mTestMtcCall.getTextListener().onRttMessageReceived(mMtcMediaSession, "");

        verify(mListener, times(1)).onCallRttMessageReceived(eq(mTestMtcCall), any());

        mTestMtcCall.getTextListener().onRttAudioIndication(mMtcMediaSession, true);

        verify(mListener, times(1)).onCallRttAudioIndication(eq(mTestMtcCall), eq(true));
    }

    @Test
    public void testonNotifyIncomingDtmfReceived() {
        int dtmfDigit = 5;
        mTestMtcCall.setListener(mListener);
        mTestMtcCall.getAudioListener().onNotifyIncomingDtmfReceived(dtmfDigit);
        processAllMessages();
        verify(mListener, times(1)).onNotifyIncomingDtmfReceived(
                eq(mTestMtcCall), eq(dtmfDigit));
    }
}
