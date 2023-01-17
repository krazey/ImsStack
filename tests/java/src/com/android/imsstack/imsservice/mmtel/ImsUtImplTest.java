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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.Mockito.doAnswer;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.os.DeadObjectException;
import android.telephony.ims.ImsCallForwardInfo;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsSsInfo;
import android.telephony.ims.ImsUtListener;
import android.telephony.ims.feature.CapabilityChangeRequest.CapabilityPair;
import android.telephony.ims.feature.MmTelFeature.MmTelCapabilities;
import android.telephony.ims.stub.ImsRegistrationImplBase;

import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.ssc.SscConstant;
import com.android.imsstack.enabler.ssc.SscServiceClassUtil;
import com.android.imsstack.imsservice.mmtel.ut.UtFactory;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtInterface;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtListener;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.ArgumentCaptor;
import org.mockito.Captor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.ArrayList;
import java.util.concurrent.Executor;

@RunWith(JUnit4.class)
public class ImsUtImplTest  {
    private static final int SLOT_0 = 0;
    private static final int INVALID_SLOT = -1;

    private ImsUtImpl mImsUtImpl;

    @Mock private Executor mMockExecutor;
    @Mock private ImsUtListener mMockImsUtListener;
    @Mock private IBaseContext mMockBaseContext;
    @Mock private IUtInterface mMockUtInterface;

    @Captor private ArgumentCaptor<Runnable> mRunnableCaptor;
    @Captor private ArgumentCaptor<IUtListener> mUtListenerCaptor;

    @Before
    public void setup() {
        MockitoAnnotations.initMocks(this);

        when(mMockBaseContext.getSlotId()).thenReturn(SLOT_0);
        when(mMockBaseContext.getExecutor()).thenReturn(mMockExecutor);
        doAnswer(invocation -> {
            mRunnableCaptor.getValue().run();
            return null;
        }).when(mMockExecutor).execute(mRunnableCaptor.capture());

        UtFactory.getInstance().setUtInterfaceForSlot(SLOT_0, mMockUtInterface);

        replaceInstance(ImsUtImpl.class, "DBG", null, true);
    }

    @After
    public void tearDown() {
        if (mImsUtImpl != null) {
            mImsUtImpl.close();
        }
    }

    @Test
    public void initAndClear() {
        mImsUtImpl = new ImsUtImpl(mMockBaseContext);

        verify(mMockUtInterface).start(any());
        verify(mMockUtInterface).setListener(any(IUtListener.class));
        assertEquals(mMockUtInterface, mImsUtImpl.getUtInterface());

        mImsUtImpl.clear();
        verify(mMockUtInterface).close();
        verify(mMockUtInterface).setListener(null);
        assertNull(mImsUtImpl.getUtInterface());
    }

    @Test
    public void initAndClear_whenUtInterfaceIsNull() {
        when(mMockBaseContext.getSlotId()).thenReturn(INVALID_SLOT);

        mImsUtImpl = new ImsUtImpl(mMockBaseContext);

        verify(mMockUtInterface, never()).start(any());
        verify(mMockUtInterface, never()).setListener(any(IUtListener.class));
        assertNull(mImsUtImpl.getUtInterface());

        mImsUtImpl.clear();
        verify(mMockUtInterface, never()).close();
        verify(mMockUtInterface, never()).setListener(null);
    }

    @Test
    public void getTransactionId_whenIdIsIntegerMax() {
        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        replaceInstance(ImsUtImpl.class, "mTransactionId", mImsUtImpl, java.lang.Integer.MAX_VALUE);

        int tId = mImsUtImpl.updateCOLP(true);

        verify(mMockUtInterface).updateCOLP(tId, true);
        assertEquals(1, tId);
    }

    @Test
    public void queryCallBarring() {
        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        int tId = mImsUtImpl.queryCallBarring(SscConstant.CONDITION_BAIC);

        verify(mMockUtInterface).queryCallBarringForServiceClass(tId, SscConstant.CONDITION_BAIC,
                SscServiceClassUtil.SERVICE_CLASS_NONE);
        assertEquals(1, tId);
    }

    @Test
    public void queryCallBarring_whenUtInterfaceIsNull() {
        when(mMockBaseContext.getSlotId()).thenReturn(INVALID_SLOT);

        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        int tId = mImsUtImpl.queryCallBarring(SscConstant.CONDITION_BAIC);

        verify(mMockUtInterface, never()).queryCallBarringForServiceClass(tId,
                SscConstant.CONDITION_BAIC, SscServiceClassUtil.SERVICE_CLASS_NONE);
        assertEquals(ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1), tId);
    }

    @Test
    public void queryCallBarringForServiceClass() {
        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        int tId = mImsUtImpl.queryCallBarringForServiceClass(SscConstant.CONDITION_BAIC,
                SscServiceClassUtil.SERVICE_CLASS_VOICE);

        verify(mMockUtInterface).queryCallBarringForServiceClass(tId, SscConstant.CONDITION_BAIC,
                SscServiceClassUtil.SERVICE_CLASS_VOICE);
        assertEquals(1, tId);
    }

    @Test
    public void queryCallBarringForServiceClass_whenUtInterfaceIsNull() {
        when(mMockBaseContext.getSlotId()).thenReturn(INVALID_SLOT);

        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        int tId = mImsUtImpl.queryCallBarringForServiceClass(SscConstant.CONDITION_BAIC,
                SscServiceClassUtil.SERVICE_CLASS_VOICE);

        verify(mMockUtInterface, never()).queryCallBarringForServiceClass(tId,
                SscConstant.CONDITION_BAIC, SscServiceClassUtil.SERVICE_CLASS_VOICE);
        assertEquals(ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1), tId);
    }

    @Test
    public void queryCallForward() {
        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        int tId = mImsUtImpl.queryCallForward(SscConstant.CONDITION_CFU, null);

        verify(mMockUtInterface).queryCallForward(tId, SscConstant.CONDITION_CFU, null);
        assertEquals(1, tId);
    }

    @Test
    public void queryCallForward_whenUtInterfaceIsNull() {
        when(mMockBaseContext.getSlotId()).thenReturn(INVALID_SLOT);

        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        int tId = mImsUtImpl.queryCallForward(SscConstant.CONDITION_CFU, null);

        verify(mMockUtInterface, never()).queryCallForward(tId, SscConstant.CONDITION_CFU, null);
        assertEquals(ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1), tId);
    }

    @Test
    public void queryCallWaiting() {
        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        int tId = mImsUtImpl.queryCallWaiting();

        verify(mMockUtInterface).queryCallWaiting(tId);
        assertEquals(1, tId);
    }

    @Test
    public void queryCallWaiting_whenUtInterfaceIsNull() {
        when(mMockBaseContext.getSlotId()).thenReturn(INVALID_SLOT);

        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        int tId = mImsUtImpl.queryCallWaiting();

        verify(mMockUtInterface, never()).queryCallWaiting(tId);
        assertEquals(ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1), tId);
    }

    @Test
    public void queryCLIR() {
        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        int tId = mImsUtImpl.queryCLIR();

        verify(mMockUtInterface).queryCLIR(tId);
        assertEquals(1, tId);
    }

    @Test
    public void queryCLIR_whenUtInterfaceIsNull() {
        when(mMockBaseContext.getSlotId()).thenReturn(INVALID_SLOT);

        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        int tId = mImsUtImpl.queryCLIR();

        verify(mMockUtInterface, never()).queryCLIR(tId);
        assertEquals(ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1), tId);
    }

    @Test
    public void queryCLIP() {
        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        int tId = mImsUtImpl.queryCLIP();

        verify(mMockUtInterface).queryCLIP(tId);
        assertEquals(1, tId);
    }

    @Test
    public void queryCLIP_whenUtInterfaceIsNull() {
        when(mMockBaseContext.getSlotId()).thenReturn(INVALID_SLOT);

        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        int tId = mImsUtImpl.queryCLIP();

        verify(mMockUtInterface, never()).queryCLIP(tId);
        assertEquals(ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1), tId);
    }

    @Test
    public void queryCOLR() {
        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        int tId = mImsUtImpl.queryCOLR();

        verify(mMockUtInterface).queryCOLR(tId);
        assertEquals(1, tId);
    }

    @Test
    public void queryCOLR_whenUtInterfaceIsNull() {
        when(mMockBaseContext.getSlotId()).thenReturn(INVALID_SLOT);

        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        int tId = mImsUtImpl.queryCOLR();

        verify(mMockUtInterface, never()).queryCOLR(tId);
        assertEquals(ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1), tId);
    }

    @Test
    public void queryCOLP() {
        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        int tId = mImsUtImpl.queryCOLP();

        verify(mMockUtInterface).queryCOLP(tId);
        assertEquals(1, tId);
    }

    @Test
    public void queryCOLP_whenUtInterfaceIsNull() {
        when(mMockBaseContext.getSlotId()).thenReturn(INVALID_SLOT);

        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        int tId = mImsUtImpl.queryCOLP();

        verify(mMockUtInterface, never()).queryCOLP(tId);
        assertEquals(ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1), tId);
    }

    @Test
    public void updateCallBarring() {
        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        int tId = mImsUtImpl.updateCallBarring(SscConstant.CONDITION_BAIC,
                SscConstant.ACTION_ACTIVATION, null);

        verify(mMockUtInterface).updateCallBarringWithPassword(tId, SscConstant.CONDITION_BAIC,
                SscConstant.ACTION_ACTIVATION, null, SscServiceClassUtil.SERVICE_CLASS_NONE, null);
        assertEquals(1, tId);
    }

    @Test
    public void updateCallBarring_whenUtInterfaceIsNull() {
        when(mMockBaseContext.getSlotId()).thenReturn(INVALID_SLOT);

        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        int tId = mImsUtImpl.updateCallBarring(SscConstant.CONDITION_BAIC,
                SscConstant.ACTION_ACTIVATION, null);

        verify(mMockUtInterface, never()).updateCallBarringWithPassword(tId,
                SscConstant.CONDITION_BAIC, SscConstant.ACTION_ACTIVATION, null,
                SscServiceClassUtil.SERVICE_CLASS_NONE, null);
        assertEquals(ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1), tId);
    }

    @Test
    public void updateCallBarringForServiceClass() {
        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        int tId = mImsUtImpl.updateCallBarringForServiceClass(SscConstant.CONDITION_BAIC,
                SscConstant.ACTION_ACTIVATION, null, SscServiceClassUtil.SERVICE_CLASS_VOICE);

        verify(mMockUtInterface).updateCallBarringWithPassword(tId, SscConstant.CONDITION_BAIC,
                SscConstant.ACTION_ACTIVATION, null, SscServiceClassUtil.SERVICE_CLASS_VOICE, null);
        assertEquals(1, tId);
    }

    @Test
    public void updateCallBarringForServiceClass_whenUtInterfaceIsNull() {
        when(mMockBaseContext.getSlotId()).thenReturn(INVALID_SLOT);

        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        int tId = mImsUtImpl.updateCallBarringForServiceClass(SscConstant.CONDITION_BAIC,
                SscConstant.ACTION_ACTIVATION, null, SscServiceClassUtil.SERVICE_CLASS_VOICE);

        verify(mMockUtInterface, never()).updateCallBarringWithPassword(tId,
                SscConstant.CONDITION_BAIC, SscConstant.ACTION_ACTIVATION, null,
                SscServiceClassUtil.SERVICE_CLASS_VOICE, null);
        assertEquals(ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1), tId);
    }

    @Test
    public void updateCallBarringWithPassword() {
        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        int tId = mImsUtImpl.updateCallBarringWithPassword(SscConstant.CONDITION_BAIC,
                SscConstant.ACTION_ACTIVATION, null, SscServiceClassUtil.SERVICE_CLASS_VIDEO, "00");

        verify(mMockUtInterface).updateCallBarringWithPassword(tId, SscConstant.CONDITION_BAIC,
                SscConstant.ACTION_ACTIVATION, null, SscServiceClassUtil.SERVICE_CLASS_VIDEO, "00");
        assertEquals(1, tId);
    }

    @Test
    public void updateCallBarringWithPassword_whenUtInterfaceIsNull() {
        when(mMockBaseContext.getSlotId()).thenReturn(INVALID_SLOT);

        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        int tId = mImsUtImpl.updateCallBarringWithPassword(SscConstant.CONDITION_BAIC,
                SscConstant.ACTION_ACTIVATION, null, SscServiceClassUtil.SERVICE_CLASS_VIDEO, "00");

        verify(mMockUtInterface, never()).updateCallBarringWithPassword(tId,
                SscConstant.CONDITION_BAIC, SscConstant.ACTION_ACTIVATION, null,
                SscServiceClassUtil.SERVICE_CLASS_VIDEO, "00");
        assertEquals(ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1), tId);
    }

    @Test
    public void updateCallForward() {
        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        int tId = mImsUtImpl.updateCallForward(SscConstant.CONDITION_CFB,
                SscConstant.ACTION_REGISTRATION, null, SscServiceClassUtil.SERVICE_CLASS_NONE, 0);

        verify(mMockUtInterface).updateCallForward(tId, SscConstant.CONDITION_CFB,
                SscConstant.ACTION_REGISTRATION, null, SscServiceClassUtil.SERVICE_CLASS_NONE, 0);
        assertEquals(1, tId);
    }

    @Test
    public void updateCallForward_whenUtInterfaceIsNull() {
        when(mMockBaseContext.getSlotId()).thenReturn(INVALID_SLOT);

        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        int tId = mImsUtImpl.updateCallForward(SscConstant.CONDITION_CFB,
                SscConstant.ACTION_REGISTRATION, null, SscServiceClassUtil.SERVICE_CLASS_NONE, 0);

        verify(mMockUtInterface, never()).updateCallForward(tId, SscConstant.CONDITION_CFB,
                SscConstant.ACTION_REGISTRATION, null, SscServiceClassUtil.SERVICE_CLASS_NONE, 0);
        assertEquals(ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1), tId);
    }

    @Test
    public void updateCallWaiting() {
        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        int tId = mImsUtImpl.updateCallWaiting(false, SscServiceClassUtil.SERVICE_CLASS_NONE);

        verify(mMockUtInterface)
                .updateCallWaiting(tId, false, SscServiceClassUtil.SERVICE_CLASS_NONE);
        assertEquals(1, tId);
    }

    @Test
    public void updateCallWaiting_whenUtInterfaceIsNull() {
        when(mMockBaseContext.getSlotId()).thenReturn(INVALID_SLOT);

        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        int tId = mImsUtImpl.updateCallWaiting(false, SscServiceClassUtil.SERVICE_CLASS_NONE);

        verify(mMockUtInterface, never())
                .updateCallWaiting(tId, false, SscServiceClassUtil.SERVICE_CLASS_NONE);
        assertEquals(ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1), tId);
    }

    @Test
    public void updateCLIR() {
        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        int tId = mImsUtImpl.updateCLIR(SscConstant.OIR_SUPPRESSION);

        verify(mMockUtInterface).updateCLIR(tId, SscConstant.OIR_SUPPRESSION);
        assertEquals(1, tId);
    }

    @Test
    public void updateCLIR_whenUtInterfaceIsNull() {
        when(mMockBaseContext.getSlotId()).thenReturn(INVALID_SLOT);

        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        int tId = mImsUtImpl.updateCLIR(SscConstant.OIR_SUPPRESSION);

        verify(mMockUtInterface, never()).updateCLIR(tId, SscConstant.OIR_SUPPRESSION);
        assertEquals(ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1), tId);
    }

    @Test
    public void updateCLIP() {
        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        int tId = mImsUtImpl.updateCLIP(true);

        verify(mMockUtInterface).updateCLIP(tId, true);
        assertEquals(1, tId);
    }

    @Test
    public void updateCLIP_whenUtInterfaceIsNull() {
        when(mMockBaseContext.getSlotId()).thenReturn(INVALID_SLOT);

        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        int tId = mImsUtImpl.updateCLIP(true);

        verify(mMockUtInterface, never()).updateCLIP(tId, true);
        assertEquals(ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1), tId);
    }

    @Test
    public void updateCOLR() {
        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        int tId = mImsUtImpl.updateCOLR(SscConstant.TIR_PROVISIONED);

        verify(mMockUtInterface).updateCOLR(tId, SscConstant.TIR_PROVISIONED);
        assertEquals(1, tId);
    }

    @Test
    public void updateCOLR_whenUtInterfaceIsNull() {
        when(mMockBaseContext.getSlotId()).thenReturn(INVALID_SLOT);

        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        int tId = mImsUtImpl.updateCOLR(SscConstant.TIR_PROVISIONED);

        verify(mMockUtInterface, never()).updateCOLR(tId, SscConstant.TIR_PROVISIONED);
        assertEquals(ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1), tId);
    }

    @Test
    public void updateCOLP() {
        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        int tId = mImsUtImpl.updateCOLP(false);

        verify(mMockUtInterface).updateCOLP(tId, false);
        assertEquals(1, tId);
    }

    @Test
    public void updateCOLP_whenUtInterfaceIsNull() {
        when(mMockBaseContext.getSlotId()).thenReturn(INVALID_SLOT);

        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        int tId = mImsUtImpl.updateCOLP(false);

        verify(mMockUtInterface, never()).updateCOLP(tId, false);
        assertEquals(ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE * (-1), tId);
    }

    @Test
    public void isUtAvailable() {
        when(mMockUtInterface.isUtAvailable()).thenReturn(false);

        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        boolean available = mImsUtImpl.isUtAvailable();

        verify(mMockUtInterface).isUtAvailable();
        assertEquals(false, available);
    }

    @Test
    public void isUtAvailable_whenUtInterfaceIsNull() {
        when(mMockBaseContext.getSlotId()).thenReturn(INVALID_SLOT);

        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        boolean available = mImsUtImpl.isUtAvailable();

        verify(mMockUtInterface, never()).isUtAvailable();
        assertEquals(false, available);
    }

    @Test
    public void changeCapabilities_enablingUt() {
        ArrayList<CapabilityPair> enabledCaps = new ArrayList<CapabilityPair>();
        ArrayList<CapabilityPair> disabledCaps = new ArrayList<CapabilityPair>();
        enabledCaps.add(new CapabilityPair(MmTelCapabilities.CAPABILITY_TYPE_UT,
                ImsRegistrationImplBase.REGISTRATION_TECH_LTE));
        disabledCaps.add(new CapabilityPair(MmTelCapabilities.CAPABILITY_TYPE_VIDEO,
                ImsRegistrationImplBase.REGISTRATION_TECH_LTE));

        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        mImsUtImpl.changeCapabilities(enabledCaps, disabledCaps);

        verify(mMockUtInterface).changeCapability(true);
    }

    @Test
    public void changeCapabilities_disablingUt() {
        ArrayList<CapabilityPair> enabledCaps = new ArrayList<CapabilityPair>();
        ArrayList<CapabilityPair> disabledCaps = new ArrayList<CapabilityPair>();
        enabledCaps.add(new CapabilityPair(MmTelCapabilities.CAPABILITY_TYPE_VOICE,
                ImsRegistrationImplBase.REGISTRATION_TECH_LTE));
        disabledCaps.add(new CapabilityPair(MmTelCapabilities.CAPABILITY_TYPE_UT,
                ImsRegistrationImplBase.REGISTRATION_TECH_LTE));

        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        mImsUtImpl.changeCapabilities(enabledCaps, disabledCaps);

        verify(mMockUtInterface).changeCapability(false);
    }

    @Test
    public void changeCapabilities_utCapabilityNotIncluded() {
        ArrayList<CapabilityPair> enabledCaps = new ArrayList<CapabilityPair>();
        ArrayList<CapabilityPair> disabledCaps = new ArrayList<CapabilityPair>();
        enabledCaps.add(new CapabilityPair(MmTelCapabilities.CAPABILITY_TYPE_VOICE,
                ImsRegistrationImplBase.REGISTRATION_TECH_LTE));
        disabledCaps.add(new CapabilityPair(MmTelCapabilities.CAPABILITY_TYPE_VIDEO,
                ImsRegistrationImplBase.REGISTRATION_TECH_LTE));

        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        mImsUtImpl.changeCapabilities(enabledCaps, disabledCaps);

        verify(mMockUtInterface, never()).changeCapability(anyBoolean());
    }

    @Test
    public void utConfigurationUpdated() {
        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        verify(mMockUtInterface).setListener(mUtListenerCaptor.capture());
        IUtListener utListenerProxy = mUtListenerCaptor.getValue();
        mImsUtImpl.setListener(mMockImsUtListener);

        utListenerProxy.utConfigurationUpdated(1);

        verify(mMockImsUtListener).onUtConfigurationUpdated(1);
    }

    @Test
    public void utConfigurationUpdated_whenImsUtListenerIsNull() {
        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        verify(mMockUtInterface).setListener(mUtListenerCaptor.capture());
        IUtListener utListenerProxy = mUtListenerCaptor.getValue();

        utListenerProxy.utConfigurationUpdated(1);

        verify(mMockImsUtListener, never()).onUtConfigurationUpdated(1);
    }

    @Test
    public void utConfigurationUpdated_deadObjectException() {
        doAnswer(invocation -> {
            throw new DeadObjectException();
        }).when(mMockImsUtListener).onUtConfigurationUpdated(anyInt());

        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        verify(mMockUtInterface).setListener(mUtListenerCaptor.capture());
        IUtListener utListenerProxy = mUtListenerCaptor.getValue();
        mImsUtImpl.setListener(mMockImsUtListener);

        utListenerProxy.utConfigurationUpdated(1);

        assertNull(mImsUtImpl.mListener);
    }

    @Test
    public void utConfigurationUpdateFailed() {
        ImsReasonInfo reasonInfo = new ImsReasonInfo(ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE,
                ImsReasonInfo.CODE_UNSPECIFIED, null);
        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        verify(mMockUtInterface).setListener(mUtListenerCaptor.capture());
        IUtListener utListenerProxy = mUtListenerCaptor.getValue();
        mImsUtImpl.setListener(mMockImsUtListener);

        utListenerProxy.utConfigurationUpdateFailed(1, reasonInfo);

        verify(mMockImsUtListener).onUtConfigurationUpdateFailed(1, reasonInfo);
    }

    @Test
    public void utConfigurationUpdateFailed_whenImsUtListenerIsNull() {
        ImsReasonInfo reasonInfo = new ImsReasonInfo(ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE,
                ImsReasonInfo.CODE_UNSPECIFIED, null);
        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        verify(mMockUtInterface).setListener(mUtListenerCaptor.capture());
        IUtListener utListenerProxy = mUtListenerCaptor.getValue();

        utListenerProxy.utConfigurationUpdateFailed(1, reasonInfo);

        verify(mMockImsUtListener, never()).onUtConfigurationUpdateFailed(1, reasonInfo);
    }

    @Test
    public void utConfigurationUpdateFailed_deadObjectException() {
        ImsReasonInfo reasonInfo = new ImsReasonInfo(ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE,
                ImsReasonInfo.CODE_UNSPECIFIED, null);
        doAnswer(invocation -> {
            throw new DeadObjectException();
        }).when(mMockImsUtListener).onUtConfigurationUpdateFailed(anyInt(), any());

        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        verify(mMockUtInterface).setListener(mUtListenerCaptor.capture());
        IUtListener utListenerProxy = mUtListenerCaptor.getValue();
        mImsUtImpl.setListener(mMockImsUtListener);

        utListenerProxy.utConfigurationUpdateFailed(1, reasonInfo);

        assertNull(mImsUtImpl.mListener);
    }

    @Test
    public void lineIdentificationSupplementaryServiceResponse() {
        ImsSsInfo ssInfo = new ImsSsInfo.Builder(ImsSsInfo.ENABLED).build();
        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        verify(mMockUtInterface).setListener(mUtListenerCaptor.capture());
        IUtListener utListenerProxy = mUtListenerCaptor.getValue();
        mImsUtImpl.setListener(mMockImsUtListener);

        utListenerProxy.lineIdentificationSupplementaryServiceResponse(1, ssInfo);

        verify(mMockImsUtListener).onLineIdentificationSupplementaryServiceResponse(1, ssInfo);
    }

    @Test
    public void lineIdentificationSupplementaryServiceResponse_whenImsUtListenerIsNull() {
        ImsSsInfo ssInfo = new ImsSsInfo.Builder(ImsSsInfo.ENABLED).build();
        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        verify(mMockUtInterface).setListener(mUtListenerCaptor.capture());
        IUtListener utListenerProxy = mUtListenerCaptor.getValue();

        utListenerProxy.lineIdentificationSupplementaryServiceResponse(1, ssInfo);

        verify(mMockImsUtListener, never())
                .onLineIdentificationSupplementaryServiceResponse(1, ssInfo);
    }

    @Test
    public void lineIdentificationSupplementaryServiceResponse_deadObjectException() {
        ImsSsInfo ssInfo = new ImsSsInfo.Builder(ImsSsInfo.ENABLED).build();
        doAnswer(invocation -> {
            throw new DeadObjectException();
        }).when(mMockImsUtListener)
                .onLineIdentificationSupplementaryServiceResponse(anyInt(), any());

        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        verify(mMockUtInterface).setListener(mUtListenerCaptor.capture());
        IUtListener utListenerProxy = mUtListenerCaptor.getValue();
        mImsUtImpl.setListener(mMockImsUtListener);

        utListenerProxy.lineIdentificationSupplementaryServiceResponse(1, ssInfo);

        assertNull(mImsUtImpl.mListener);
    }

    @Test
    public void utConfigurationCallBarringQueried() {
        ImsSsInfo[] ssInfo = new ImsSsInfo[2];
        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        verify(mMockUtInterface).setListener(mUtListenerCaptor.capture());
        IUtListener utListenerProxy = mUtListenerCaptor.getValue();
        mImsUtImpl.setListener(mMockImsUtListener);

        utListenerProxy.utConfigurationCallBarringQueried(1, ssInfo);

        verify(mMockImsUtListener).onUtConfigurationCallBarringQueried(1, ssInfo);
    }

    @Test
    public void utConfigurationCallBarringQueried_whenImsUtListenerIsNull() {
        ImsSsInfo[] ssInfo = new ImsSsInfo[2];
        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        verify(mMockUtInterface).setListener(mUtListenerCaptor.capture());
        IUtListener utListenerProxy = mUtListenerCaptor.getValue();

        utListenerProxy.utConfigurationCallBarringQueried(1, ssInfo);

        verify(mMockImsUtListener, never()).onUtConfigurationCallBarringQueried(1, ssInfo);
    }

    @Test
    public void utConfigurationCallBarringQueried_deadObjectException() {
        ImsSsInfo[] ssInfo = new ImsSsInfo[2];
        doAnswer(invocation -> {
            throw new DeadObjectException();
        }).when(mMockImsUtListener).onUtConfigurationCallBarringQueried(anyInt(), any());

        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        verify(mMockUtInterface).setListener(mUtListenerCaptor.capture());
        IUtListener utListenerProxy = mUtListenerCaptor.getValue();
        mImsUtImpl.setListener(mMockImsUtListener);

        utListenerProxy.utConfigurationCallBarringQueried(1, ssInfo);

        assertNull(mImsUtImpl.mListener);
    }

    @Test
    public void utConfigurationCallForwardQueried() {
        ImsCallForwardInfo[] cfInfo = new ImsCallForwardInfo[2];
        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        verify(mMockUtInterface).setListener(mUtListenerCaptor.capture());
        IUtListener utListenerProxy = mUtListenerCaptor.getValue();
        mImsUtImpl.setListener(mMockImsUtListener);

        utListenerProxy.utConfigurationCallForwardQueried(1, cfInfo);

        verify(mMockImsUtListener).onUtConfigurationCallForwardQueried(1, cfInfo);
    }

    @Test
    public void utConfigurationCallForwardQueried_whenImsUtListenerIsNull() {
        ImsCallForwardInfo[] cfInfo = new ImsCallForwardInfo[2];
        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        verify(mMockUtInterface).setListener(mUtListenerCaptor.capture());
        IUtListener utListenerProxy = mUtListenerCaptor.getValue();

        utListenerProxy.utConfigurationCallForwardQueried(1, cfInfo);

        verify(mMockImsUtListener, never()).onUtConfigurationCallForwardQueried(1, cfInfo);
    }

    @Test
    public void utConfigurationCallForwardQueried_deadObjectException() {
        ImsCallForwardInfo[] cfInfo = new ImsCallForwardInfo[2];
        doAnswer(invocation -> {
            throw new DeadObjectException();
        }).when(mMockImsUtListener).onUtConfigurationCallForwardQueried(anyInt(), any());

        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        verify(mMockUtInterface).setListener(mUtListenerCaptor.capture());
        IUtListener utListenerProxy = mUtListenerCaptor.getValue();
        mImsUtImpl.setListener(mMockImsUtListener);

        utListenerProxy.utConfigurationCallForwardQueried(1, cfInfo);

        assertNull(mImsUtImpl.mListener);
    }

    @Test
    public void utConfigurationCallWaitingQueried() {
        ImsSsInfo[] ssInfo = new ImsSsInfo[2];
        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        verify(mMockUtInterface).setListener(mUtListenerCaptor.capture());
        IUtListener utListenerProxy = mUtListenerCaptor.getValue();
        mImsUtImpl.setListener(mMockImsUtListener);

        utListenerProxy.utConfigurationCallWaitingQueried(1, ssInfo);

        verify(mMockImsUtListener).onUtConfigurationCallWaitingQueried(1, ssInfo);
    }

    @Test
    public void utConfigurationCallWaitingQueried_whenImsUtListenerIsNull() {
        ImsSsInfo[] ssInfo = new ImsSsInfo[2];
        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        verify(mMockUtInterface).setListener(mUtListenerCaptor.capture());
        IUtListener utListenerProxy = mUtListenerCaptor.getValue();

        utListenerProxy.utConfigurationCallWaitingQueried(1, ssInfo);

        verify(mMockImsUtListener, never()).onUtConfigurationCallWaitingQueried(1, ssInfo);
    }

    @Test
    public void utConfigurationCallWaitingQueried_deadObjectException() {
        ImsSsInfo[] ssInfo = new ImsSsInfo[2];
        doAnswer(invocation -> {
            throw new DeadObjectException();
        }).when(mMockImsUtListener).onUtConfigurationCallWaitingQueried(anyInt(), any());

        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        verify(mMockUtInterface).setListener(mUtListenerCaptor.capture());
        IUtListener utListenerProxy = mUtListenerCaptor.getValue();
        mImsUtImpl.setListener(mMockImsUtListener);

        utListenerProxy.utConfigurationCallWaitingQueried(1, ssInfo);

        assertNull(mImsUtImpl.mListener);
    }

    @Test
    public void utConfigurationQueryFailed() {
        ImsReasonInfo reasonInfo = new ImsReasonInfo(ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE,
                ImsReasonInfo.CODE_UNSPECIFIED, null);
        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        verify(mMockUtInterface).setListener(mUtListenerCaptor.capture());
        IUtListener utListenerProxy = mUtListenerCaptor.getValue();
        mImsUtImpl.setListener(mMockImsUtListener);

        utListenerProxy.utConfigurationQueryFailed(1, reasonInfo);

        verify(mMockImsUtListener).onUtConfigurationQueryFailed(1, reasonInfo);
    }

    @Test
    public void utConfigurationQueryFailed_whenImsUtListenerIsNull() {
        ImsReasonInfo reasonInfo = new ImsReasonInfo(ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE,
                ImsReasonInfo.CODE_UNSPECIFIED, null);
        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        verify(mMockUtInterface).setListener(mUtListenerCaptor.capture());
        IUtListener utListenerProxy = mUtListenerCaptor.getValue();

        utListenerProxy.utConfigurationQueryFailed(1, reasonInfo);

        verify(mMockImsUtListener, never()).onUtConfigurationQueryFailed(1, reasonInfo);
    }

    @Test
    public void utConfigurationQueryFailed_deadObjectException() {
        ImsReasonInfo reasonInfo = new ImsReasonInfo(ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE,
                ImsReasonInfo.CODE_UNSPECIFIED, null);
        doAnswer(invocation -> {
            throw new DeadObjectException();
        }).when(mMockImsUtListener).onUtConfigurationQueryFailed(anyInt(), any());

        mImsUtImpl = new ImsUtImpl(mMockBaseContext);
        verify(mMockUtInterface).setListener(mUtListenerCaptor.capture());
        IUtListener utListenerProxy = mUtListenerCaptor.getValue();
        mImsUtImpl.setListener(mMockImsUtListener);

        utListenerProxy.utConfigurationQueryFailed(1, reasonInfo);

        assertNull(mImsUtImpl.mListener);
    }

    private synchronized void replaceInstance(final Class c, final String instanceName,
            final Object obj, final Object newValue) {
        try {
            java.lang.reflect.Field field = c.getDeclaredField(instanceName);
            field.setAccessible(true);
            field.set(obj, newValue);
        } catch (Exception e) {
            org.junit.Assert.fail(e.toString());
        }
    }
}
