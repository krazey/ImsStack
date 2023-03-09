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
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.anyBoolean;
import static org.mockito.Mockito.anyInt;
import static org.mockito.Mockito.anyLong;
import static org.mockito.Mockito.doAnswer;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;

import android.os.Looper;
import android.os.Parcel;
import android.telephony.emergency.EmergencyNumber;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import com.android.imsstack.ImsStackTest;
import com.android.imsstack.enabler.IBaseContext;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class MtcEmergencyServiceManagerTest extends ImsStackTest {
    private int mCommand;
    private final int mInvalid = -1;
    private long mNativeObject;

    @Mock private IBaseContext mMockContext;
    @Mock private MtcJniProxy mMockMtcJniProxy;
    @Mock private MtcCall mMockMtcCall;
    @Mock private IServiceStateTracker mServiceStateTracker;

    private MtcEmergencyServiceManager mTestMtcEmergencyServiceManager;

    @Before
    public void setUp() throws Exception {
        super.setUp(getClass().getSimpleName());
        MockitoAnnotations.initMocks(this);

        mCommand = mInvalid;
        mNativeObject = mInvalid;
        mTestMtcEmergencyServiceManager = new MtcEmergencyServiceManager(
                mMockContext, mMockMtcJniProxy);

        doReturn(Looper.myLooper()).when(mMockContext).getCallLooper();
        doAnswer(invocation -> {
                    mNativeObject = (long) invocation.getArgument(0);

                    Parcel parcel = (Parcel) invocation.getArgument(1);
                    if (parcel == null) {
                        return null;
                    }

                    parcel.setDataPosition(0);
                    mCommand = parcel.readInt();

                    parcel.recycle();
                    parcel = null;
                    return null;
                }
                ).when(mMockMtcJniProxy).sendDataToNative(anyLong(), any(Parcel.class));

        mTestMtcEmergencyServiceManager.init();
        mTestMtcEmergencyServiceManager.setCall(mMockMtcCall);
    }

    @After
    public void tearDown() throws Exception {
        mTestMtcEmergencyServiceManager = null;
        super.tearDown();
    }

    @Test
    public void testOpenEmergencyService() {
        mTestMtcEmergencyServiceManager.setNativeObject(1);
        mTestMtcEmergencyServiceManager.openEmergencyService(
                EmergencyNumber.EMERGENCY_CALL_ROUTING_EMERGENCY, mServiceStateTracker);
        processAllMessages();

        assertEquals(1, mNativeObject);
        assertEquals(IUMtcService.OPEN_EMERGENCY_SERVICE, mCommand);
    }

    @Test
    public void testOnEmergencyServiceStateChanged() {
        mTestMtcEmergencyServiceManager.onEmergencyServiceStateChanged(IUMtcService.ES_IDLE, 0, 0);
        mTestMtcEmergencyServiceManager.onEmergencyServiceStateChanged(
                IUMtcService.ES_OPENED, 0, 0);

        verifyNoMoreInteractions(mMockMtcCall);

        mTestMtcEmergencyServiceManager.setCall(mMockMtcCall);
        mTestMtcEmergencyServiceManager.onEmergencyServiceStateChanged(
                IUMtcService.ES_UNAVAILABLE, 0, 0);
        mTestMtcEmergencyServiceManager.onEmergencyServiceStateChanged(
                IUMtcService.ES_OPENED, 0, 0);

        verifyNoMoreInteractions(mMockMtcCall);

        mTestMtcEmergencyServiceManager.setCall(mMockMtcCall);
        mTestMtcEmergencyServiceManager.onEmergencyServiceStateChanged(
                IUMtcService.ES_OPENED, 0, 0);

        verify(mMockMtcCall, times(1)).createNativeCallObject();
        verify(mMockMtcCall, times(1)).open(anyInt(), anyBoolean(), anyBoolean(), anyBoolean());
    }
}
