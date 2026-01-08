/*
 * Copyright (C) 2025 The Android Open Source Project
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
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.os.Message;
import android.os.Parcel;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;
import android.util.SparseArray;

import com.android.imsstack.ImsStackTest;
import com.android.imsstack.base.AppContext;
import com.android.imsstack.imsservice.mmtel.ut.UtFactory;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtInterface;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtInterface.CbData;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtInterface.SupplementaryServiceConfiguration;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtInterface.TerminalBasedSupplementaryServiceConfigurationChangeListener;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtInterface.TirData;
import com.android.imsstack.internal.ImsStackRegistry;
import com.android.imsstack.internal.imsservice.ImsServiceRegistry;
import com.android.imsstack.internal.imsservice.MmTelFeatureRegistry;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Captor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.ArrayList;
import java.util.List;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class MtcTerminalBasedSupplementaryServiceNotifierTest extends ImsStackTest {
    @Mock SparseArray<ImsServiceRegistry> mMockImsServiceRegistrys;
    @Mock ImsServiceRegistry mMockImsServiceRegistry;
    @Mock MmTelFeatureRegistry mMockMmTelFeatureRegistry;
    @Mock private IUtInterface mUtInterface;
    @Mock private MtcApp.MtcAppHandler mMockMtcAppHandler;
    @Captor ArgumentCaptor<ImsStackRegistry.ImsServiceListener> mImsServiceListenerCaptor;
    @Captor private ArgumentCaptor<TerminalBasedSupplementaryServiceConfigurationChangeListener>
            mTbSscConfigListenerCaptor;
    @Captor private ArgumentCaptor<MmTelFeatureRegistry.Listener> mMmtelFeatureListenerCaptor;
    @Captor private ArgumentCaptor<Message> mMessageCaptor;

    private int mSlotId0 = 0;
    private MtcTerminalBasedSupplementaryServiceNotifier mTbSscSender;

    @Before
    public void setUp() throws Exception {
        super.setUp(getClass().getSimpleName());
        MockitoAnnotations.initMocks(this);
        AppContext.init(mContext);

        when(mMockImsServiceRegistrys.get(mSlotId0)).thenReturn(mMockImsServiceRegistry);
        when(mMockImsServiceRegistry.getMmTelFeatureRegistry()).thenReturn(
                mMockMmTelFeatureRegistry);
        replaceInstance(ImsServiceRegistry.class, "sImsServiceRegistrys", null,
                mMockImsServiceRegistrys);

        UtFactory.getInstance().setUtInterfaceForSlot(mSlotId0, mUtInterface);

        mTbSscSender = new MtcTerminalBasedSupplementaryServiceNotifier(
                mSlotId0, mTestableLooper.getLooper());
    }

    @After
    public void tearDown() throws Exception {
        AppContext.deinit();
        UtFactory.getInstance().setUtInterfaceForSlot(mSlotId0, null);
        restoreInstances();
        super.tearDown();
    }

    @Test
    public void testInit() {
        mTbSscSender.init();
        verify(mMockMmTelFeatureRegistry, times(1)).addListener(any());
        verify(mUtInterface, times(1)).addTbSscChangeListener(any());
    }

    @Test
    public void testDeinit() {
        mTbSscSender.init();
        verify(mMockMmTelFeatureRegistry, times(1)).addListener(any());
        verify(mUtInterface, times(1)).addTbSscChangeListener(any());

        mTbSscSender.deinit();
        verify(mMockMmTelFeatureRegistry, times(1)).removeListener(any());
        verify(mUtInterface, times(1)).removeTbSscChangeListener(any());
    }

    @Test
    public void testOnTerminalBasedCallWaitingStatusChanged() {
        mTbSscSender.init();
        verify(mMockMmTelFeatureRegistry, times(1)).addListener(
                mMmtelFeatureListenerCaptor.capture());

        mTbSscSender.setHandler(null);
        MmTelFeatureRegistry.Listener mmtelFeatureListener = mMmtelFeatureListenerCaptor.getValue();
        when(mMockMmTelFeatureRegistry.isTerminalBasedCallWaitingEnabled()).thenReturn(true);
        mmtelFeatureListener.onTerminalBasedCallWaitingStatusChanged();
        processAllMessages();

        verify(mMockMtcAppHandler, never()).sendMessage(any());

        mTbSscSender.setHandler(mMockMtcAppHandler);
        mmtelFeatureListener.onTerminalBasedCallWaitingStatusChanged();
        processAllMessages();

        verify(mMockMtcAppHandler).sendMessage(mMessageCaptor.capture());
        Message capturedMessage = mMessageCaptor.getValue();
        assertNotNull(capturedMessage);
        assertEquals(MtcApp.MSG_SEND_NOTIFICATION, capturedMessage.what);
    }

    @Test
    public void testOnSupplementaryServiceConfigurationChanged() {
        mTbSscSender.init();
        verify(mUtInterface, times(1)).addTbSscChangeListener(mTbSscConfigListenerCaptor.capture());

        mTbSscSender.setHandler(null);
        TerminalBasedSupplementaryServiceConfigurationChangeListener TbSscConfigListener =
                mTbSscConfigListenerCaptor.getValue();
        TbSscConfigListener.onSupplementaryServiceConfigurationChanged(
                List.of(new TirData(SupplementaryServiceConfiguration.STATUS_ENABLED)));
        processAllMessages();

        verify(mMockMtcAppHandler, never()).sendMessage(any());

        mTbSscSender.setHandler(mMockMtcAppHandler);
        TbSscConfigListener.onSupplementaryServiceConfigurationChanged(
                List.of(new TirData(SupplementaryServiceConfiguration.STATUS_ENABLED)));
        processAllMessages();

        verify(mMockMtcAppHandler).sendMessage(mMessageCaptor.capture());
        Message capturedMessage = mMessageCaptor.getValue();
        assertNotNull(capturedMessage);
        assertEquals(MtcApp.MSG_SEND_NOTIFICATION, capturedMessage.what);
        Parcel parcel = (Parcel) capturedMessage.obj;
        parcel.setDataPosition(0);
        assertEquals(IUMtcService.PERMANENT_SUPP_CHANGED, parcel.readInt());
        assertEquals(1, parcel.readInt());
        assertEquals(PermanentSuppInfo.SUPP_TYPE_TB_TIR, parcel.readInt());
    }

    @Test
    public void testIsOutgoingCallBarringActivated() {
        mTbSscSender.init();
        verify(mUtInterface, times(1)).addTbSscChangeListener(mTbSscConfigListenerCaptor.capture());
        assertFalse(mTbSscSender.isOutgoingCallBarringActivated(
                IUMtcCall.CALLTYPE_VOIP, "+8279281327"));

        mTbSscSender.setHandler(mMockMtcAppHandler);
        TerminalBasedSupplementaryServiceConfigurationChangeListener TbSscConfigListener =
                mTbSscConfigListenerCaptor.getValue();
        TbSscConfigListener.onSupplementaryServiceConfigurationChanged(
                List.of(new CbData(CbData.CONDITION_BAOC, CbData.SERVICE_CLASS_VOICE,
                SupplementaryServiceConfiguration.STATUS_ENABLED)));
        processAllMessages();

        verify(mMockMtcAppHandler, never()).sendMessage(any());
        assertTrue(mTbSscSender.isOutgoingCallBarringActivated(
                IUMtcCall.CALLTYPE_VOIP, "+8279281327"));
    }

    @Test
    public void testOnSupplementaryServiceConfigurationChanged_withInvalidObjectType()
            throws Exception {
        // This test verifies that the handler gracefully handles messages where the object
        // is not a List, which is an edge case the new code explicitly checks for.

        // Setup: Initialize the notifier and set a mock handler to capture notifications.
        mTbSscSender.setHandler(mMockMtcAppHandler);

        // Action: Send a message with an invalid object type (a String instead of a List)
        // to the internal handler using a test-only helper method. This simulates an
        // unexpected message format.
        mTbSscSender.sendMessageToTbssHandler(
                MtcTerminalBasedSupplementaryServiceNotifier.MSG_ON_TBTIR_TBCB_CHANGED,
                "This is not a list");
        processAllMessages();

        // Verification: Ensure that no notification was sent to the native layer, as the
        // invalid message should be ignored.
        verify(mMockMtcAppHandler, never()).sendMessage(any());
    }

    @Test
    public void testOnSupplementaryServiceConfigurationChanged_withMixedList() throws Exception {
        // This test verifies that the handler can process a list containing a mix of valid
        // and invalid object types, correctly filtering out the invalid ones. This validates
        // the type check inside the loop.

        // Setup: Initialize the notifier and set a mock handler.
        mTbSscSender.setHandler(mMockMtcAppHandler);

        // Action: Create a raw list with mixed object types and send it to the internal handler
        // using a test-only helper method.
        List<Object> mixedList = new ArrayList<>();
        mixedList.add(new TirData(SupplementaryServiceConfiguration.STATUS_ENABLED)); // Valid item
        mixedList.add("Invalid item"); // Invalid item

        mTbSscSender.sendMessageToTbssHandler(
                MtcTerminalBasedSupplementaryServiceNotifier.MSG_ON_TBTIR_TBCB_CHANGED,
                mixedList);
        processAllMessages();

        // Verification:
        // Ensure that a notification was sent, but only once, for the valid item in the list.
        verify(mMockMtcAppHandler, times(1)).sendMessage(mMessageCaptor.capture());

        // Inspect the captured message to confirm it contains the correct data from the valid item.
        Message capturedMessage = mMessageCaptor.getValue();
        assertNotNull(capturedMessage);
        assertEquals(MtcApp.MSG_SEND_NOTIFICATION, capturedMessage.what);

        Parcel parcel = (Parcel) capturedMessage.obj;
        parcel.setDataPosition(0);
        // Check that the parcel contains the expected PERMANENT_SUPP_CHANGED code.
        assertEquals(IUMtcService.PERMANENT_SUPP_CHANGED, parcel.readInt());
        // Check that there is 1 service update in the parcel.
        assertEquals(1, parcel.readInt());
        // Check that the service type is the one from the valid TirData object.
        assertEquals(PermanentSuppInfo.SUPP_TYPE_TB_TIR, parcel.readInt());
    }
}
