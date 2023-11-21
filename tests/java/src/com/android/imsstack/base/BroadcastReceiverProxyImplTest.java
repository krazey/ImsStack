/*
 * Copyright (C) 2023 The Android Open Source Project
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
package com.android.imsstack.base;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.IntentFilter;
import android.os.Handler;

import androidx.test.filters.SmallTest;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class BroadcastReceiverProxyImplTest {
    @Mock private Context mContext;
    @Mock private Handler mScheduler;

    private BroadcastReceiverProxyImpl mBroadcastReceiverProxy;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mBroadcastReceiverProxy = new BroadcastReceiverProxyImpl(mContext, mScheduler);
    }

    @After
    public void tearDown() throws Exception {
        mScheduler = null;
        mBroadcastReceiverProxy = null;
        mContext = null;
    }

    @Test
    @SmallTest
    public void testRegisterAndUnregisterReceiver() {
        BroadcastReceiver receiver = mock(BroadcastReceiver.class);
        IntentFilter filter = mock(IntentFilter.class);

        mBroadcastReceiverProxy.registerReceiver(receiver, filter);
        verify(mContext).registerReceiver(
                eq(receiver), eq(filter), any(), eq(mScheduler), anyInt());

        Handler handler = mock(Handler.class);
        mBroadcastReceiverProxy.registerReceiver(receiver, filter, handler);
        verify(mContext).registerReceiver(eq(receiver), eq(filter), any(), eq(handler), anyInt());

        mBroadcastReceiverProxy.unregisterReceiver(receiver);
        verify(mContext).unregisterReceiver(eq(receiver));
    }
}
