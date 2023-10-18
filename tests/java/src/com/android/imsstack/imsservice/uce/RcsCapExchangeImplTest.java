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
package com.android.imsstack.imsservice.uce;

import static org.mockito.Mockito.timeout;
import static org.mockito.Mockito.verify;

import android.content.Context;
import android.net.Uri;
import android.telephony.ims.stub.CapabilityExchangeEventListener;
import android.telephony.ims.stub.RcsCapabilityExchangeImplBase;
import android.telephony.ims.stub.RcsCapabilityExchangeImplBase.PublishResponseCallback;
import android.telephony.ims.stub.RcsCapabilityExchangeImplBase.SubscribeResponseCallback;

import com.android.imsstack.enabler.uce.impl.RcsCapOptionsResponseCallBack;
import com.android.imsstack.enabler.uce.impl.RcsCapPublishResponseCallBack;
import com.android.imsstack.enabler.uce.impl.RcsCapSubscribeResponseCallBack;
import com.android.imsstack.enabler.uce.interf.IUceApi;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.Set;
@RunWith(JUnit4.class)
public class RcsCapExchangeImplTest {
    private static final String TEST_PHONE_NUMBER = "+123456789";
    @Mock
    Context mContext;
    @Mock
    PublishResponseCallback mPublishResponseCallback;
    @Mock
    RcsCapPublishResponseCallBack mRcsCapPublishResponseCallBack;
    @Mock
    RcsCapSubscribeResponseCallBack mRcsCapSubscribeResponseCallBack;
    @Mock
    SubscribeResponseCallback mSubscribeResponseCallback;
    @Mock
    RcsCapabilityExchangeImplBase.OptionsResponseCallback mOptionCallback;
    @Mock
    RcsCapOptionsResponseCallBack mRcsCapOptionsResponseCallBack;
    private int mSlotid = 1;
    private RcsCapExchangeImpl mRcsCapExchangeImpl;
    @Mock
    private CapabilityExchangeEventListener mCapabilityExchangeEventListener;
    @Mock
    private IUceApi mUceApi;
    private static final String VIDEO = "urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel\";video";

    @Before
    public void setUp() throws Exception {
        //Initialize Mock Objects
        MockitoAnnotations.initMocks(this);
        mRcsCapExchangeImpl = new RcsCapExchangeImpl(mCapabilityExchangeEventListener, mSlotid,
                mContext, mUceApi, mRcsCapSubscribeResponseCallBack,
                mRcsCapOptionsResponseCallBack, mRcsCapPublishResponseCallBack);
    }

    @Test
    public void subscribeForCapabilitiesTest() {
        Collection<Uri> uris = new ArrayList<>();
        uris.add(Uri.parse(TEST_PHONE_NUMBER));
        mRcsCapExchangeImpl.subscribeForCapabilities(uris, mSubscribeResponseCallback);
        verify(mUceApi, Mockito.timeout(100).times(1)).subscribeCapabilities(uris,
                mRcsCapSubscribeResponseCallBack);
    }

    // Test case to test publishCapability request
    @Test
    public void publishCapabilitiesTest() {
        String pidf = getpidf();
        mRcsCapExchangeImpl.publishCapabilities(pidf, mPublishResponseCallback);
        verify(mUceApi, Mockito.timeout(100).times(1)).publishCapabilities(pidf,
                mRcsCapPublishResponseCallBack);
    }

    // Test case to test sendOptionsCapabilityRequest request
    @Test
    public void sendOptionsCapabilityRequestTest() {
        Uri contactUri = Uri.parse("1234566789");
        Set<String> myCapabilities = new HashSet<String>();
        myCapabilities.add(VIDEO);
        mRcsCapExchangeImpl.sendOptionsCapabilityRequest(contactUri, myCapabilities,
                mOptionCallback);
        verify(mUceApi, timeout(100).times(1))
                .sendOptionsCapabilityRequest(contactUri, myCapabilities,
                        mRcsCapOptionsResponseCallBack);
    }

    public String getpidf() {
        String pidfBuilder = "<?xml version='1.0' encoding='utf-8' standalone='yes' ?>"
                + "<presence entity=\"" + "sip:0123@ims.mnc001.mcc001.3gppnetwork.org" + "\""
                + " xmlns=\"urn:ietf:params:xml:ns:pidf\""
                + " xmlns:op=\"urn:oma:xml:prs:pidf:oma-pres\""
                + " xmlns:caps=\"urn:ietf:params:xml:ns:pidf:caps\">"
                + "<tuple id=\"tid0\">"
                // status
                + "<status><basic>" + "open" + "</basic></status>"
                // service description
                + "<op:service-description>"
                + "<op:service-id>service_id_01</op:service-id>"
                + "<op:version>1.0</op:version>"
                + "<op:description>" + "Capabilities Discovery Service" + "</op:description>"
                + "</op:service-description>"
                // service capabilities
                + "<caps:servcaps>"
                // audio capabilities
                + "<caps:audio>" + "true" + "</caps:audio>"
                // video capabilities
                + "<caps:video>" + "true" + "</caps:video>"
                // duplex mode
                + "<caps:duplex>"
                // support duplex mode
                + "<caps:supported>"
                + "<caps:" + "duplex" + "/>"
                + "</caps:supported>"
                // unsupported duplex mode
                + "<caps:notsupported>"
                + "<caps:" + "full" + "/>"
                + "</caps:notsupported>"
                + "</caps:duplex>"
                + "</caps:servcaps>"
                + "<contact>" + "sip:0123@ims.mnc001.mcc001.3gppnetwork.org" + "</contact>"
                + "</tuple>"
                + "</presence>";
        return pidfBuilder;
    }

    @After
    public void tearDown() {
        mSlotid = 0;
    }

    @After
    public void cleanUp() {
        mRcsCapExchangeImpl = null;
        mRcsCapOptionsResponseCallBack = null;
        mRcsCapSubscribeResponseCallBack = null;
        mRcsCapPublishResponseCallBack = null;
        mCapabilityExchangeEventListener = null;
        mUceApi = null;
    }
}
