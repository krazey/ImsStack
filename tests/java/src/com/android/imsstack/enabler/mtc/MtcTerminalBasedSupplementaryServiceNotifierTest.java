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
}
