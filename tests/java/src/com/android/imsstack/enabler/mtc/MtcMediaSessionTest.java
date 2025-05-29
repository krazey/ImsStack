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
import static org.junit.Assert.assertNotNull;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.argThat;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyLong;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.doNothing;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.content.pm.ServiceInfo;
import android.media.ToneGenerator;
import android.os.Parcel;
import android.telephony.AccessNetworkConstants;
import android.telephony.AccessNetworkConstants.AccessNetworkType;
import android.telephony.CallQuality;
import android.telephony.ims.MediaQualityStatus;
import android.telephony.ims.MediaThreshold;
import android.telephony.ims.RtpHeaderExtension;
import android.telephony.imsmedia.IImsMedia;
import android.telephony.imsmedia.RtpReceptionStats;
import android.view.Surface;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.media.IMediaListener;
import com.android.imsstack.enabler.media.MediaSession;
import com.android.imsstack.internal.imsservice.MmTelMediaQualityReporter;
import com.android.imsstack.internal.imsservice.MmTelMediaRegistry;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

@RunWith(JUnit4.class)
public class MtcMediaSessionTest {
    private static final int SLOT0 = 0;
    private static final String EXPECTED_IMS_MEDIA_SERVICE_CONTROLLER_ACTION =
            "com.android.telephony.imsmedia.IMS_MEDIA_SERVICE_CONTROLLER";
    private static final int MEDIA_TYPE_AUDIO = IUMtcMedia.SESSION_TYPE_AUDIO;
    private static final int MEDIA_TYPE_VIDEO = IUMtcMedia.SESSION_TYPE_VIDEO;
    private static final int MEDIA_DIRECTION_UL = 1;
    private static final int RAT_EUTRAN = AccessNetworkType.EUTRAN;
    private MtcMediaSession mMtcMediaSession;

    @Mock private IBaseContext mMockBaseContext;
    @Mock private Call mMockCall;
    @Mock private IMediaListener mMockMediaListener;
    @Mock private MtcMediaSession.AudioListener mMockAudioListener;
    @Mock private MtcMediaSession.VideoListener mMockVideoListener;
    @Mock private MtcMediaSession.TextListener mMockTextListener;
    @Mock private Surface mMockSurface;
    @Mock private MmTelMediaQualityReporter mMockMediaQualityReporter;
    @Mock private MmTelMediaRegistry mMockMediaRegistry;
    @Mock private MediaThreshold mMockMediaThreshold;
    @Mock private MediaSession mMockMediaSession;
    @Mock private IImsMedia.Stub mMockImsMedia;
    @Mock private MtcJniProxy mMockMtcJniProxy;
    @Mock private Context mContext;

    @Before
    public void setUp() throws Exception {
        AppContext.deinit();
        // TODO : Need to improve towards not using Appcontext

        MockitoAnnotations.initMocks(this);

        AppContext.init(mContext);

        PackageManager mockPackageManager = Mockito.mock(PackageManager.class);
        when(mContext.getPackageManager()).thenReturn(mockPackageManager);
        when(mMockBaseContext.getContext()).thenReturn(mContext);
        when(mMockBaseContext.getSlotId()).thenReturn(SLOT0);

        final ComponentName imsMediaServiceComponent =
                new ComponentName("com.android.telephony.imsmedia",
                                "com.android.telephony.imsmedia.ImsMediaController");

        ResolveInfo resolveInfo = new ResolveInfo();
        resolveInfo.serviceInfo = new ServiceInfo();
        resolveInfo.serviceInfo.packageName = imsMediaServiceComponent.getPackageName();
        resolveInfo.serviceInfo.name = imsMediaServiceComponent.getClassName();
        resolveInfo.serviceInfo.applicationInfo = new ApplicationInfo();
        resolveInfo.serviceInfo.applicationInfo.packageName =
                imsMediaServiceComponent.getPackageName();

        List<ResolveInfo> resolveInfoList = new ArrayList<>();
        resolveInfoList.add(resolveInfo);

        when(mockPackageManager.queryIntentServices(
                argThat(intent -> EXPECTED_IMS_MEDIA_SERVICE_CONTROLLER_ACTION.equals(
                        intent.getAction())),
                eq(PackageManager.MATCH_DEFAULT_ONLY)
        )).thenReturn(resolveInfoList);

        // --- Mocking the Context.bindService behavior ---
        // ImsMediaManager will call bindService with an Intent that has the action and package set.
        // The flags used are Context.BIND_AUTO_CREATE | Context.BIND_IMPORTANT |
        // Context.BIND_NOT_VISIBLE.
        when(mContext.bindService(
                argThat((Intent intent) ->
                        EXPECTED_IMS_MEDIA_SERVICE_CONTROLLER_ACTION.equals(intent.getAction())
                        && imsMediaServiceComponent.getPackageName().equals(intent.getPackage())
                ),
                any(ServiceConnection.class),
                eq(Context.BIND_AUTO_CREATE | Context.BIND_IMPORTANT | Context.BIND_NOT_VISIBLE)
        )).thenAnswer(invocation -> {
            ServiceConnection connectionArgument = invocation.getArgument(1);
            connectionArgument.onServiceConnected(imsMediaServiceComponent, mMockImsMedia);
            return true;
        });

        MtcJniProxy.setInstanceForTesting(mMockMtcJniProxy);
        doNothing().when(mMockMtcJniProxy).sendDataToNative(anyLong(), any(Parcel.class));

        mMtcMediaSession = new MtcMediaSession(mMockBaseContext, mMockCall);
        mMtcMediaSession.setMediaSession(mMockMediaSession);
        mMtcMediaSession.setMediaListener(mMockMediaListener);
        mMtcMediaSession.setAudioListener(mMockAudioListener);
        mMtcMediaSession.setVideoListener(mMockVideoListener);
        mMtcMediaSession.setTextListener(mMockTextListener);
        mMtcMediaSession.setMediaQualityReporter(mMockMediaQualityReporter);
        when(mMockMediaQualityReporter.getMediaRegistry()).thenReturn(mMockMediaRegistry);
        when(mMockMediaRegistry.getMediaThreshold(anyInt())).thenReturn(mMockMediaThreshold);
    }

    @After
    public void tearDown() throws Exception {
        if (mMtcMediaSession != null) {
            mMtcMediaSession.dispose();
            mMtcMediaSession = null;
        }
        MtcJniProxy.setInstanceForTesting(null);
        AppContext.deinit();
    }

    @Test
    public void testSendDtmf() throws Exception {
        when(mMockCall.getNativeCallId()).thenReturn(1L);
        mMtcMediaSession.sendDtmf('1');

        verify(mMockMtcJniProxy, times(1)).sendDataToNative(eq(1L), any(Parcel.class));
    }

    @Test
    public void testSetPreviewSurface() throws Exception {
        when(mMockCall.getNativeCallId()).thenReturn(1L);
        mMtcMediaSession.setPreviewSurface(mMockSurface);

        verify(mMockMtcJniProxy, times(1)).sendDataToNative(eq(1L), any(Parcel.class));
    }

    @Test
    public void testSetDisplaySurface() throws Exception {
        when(mMockCall.getNativeCallId()).thenReturn(1L);
        mMtcMediaSession.setDisplaySurface(mMockSurface);

        verify(mMockMtcJniProxy, times(1)).sendDataToNative(eq(1L), any(Parcel.class));
    }

    @Test
    public void testPeerDimensionChanged() throws Exception {
        mMtcMediaSession.peerDimensionChanged(100, 200);

        verify(mMockVideoListener, times(1)).onMediaSessionPeerDimensionsChanged(
                eq(mMtcMediaSession), eq(100), eq(200));
    }

    @Test
    public void testOnNotifyVideoDataUsage() throws Exception {
        mMtcMediaSession.onNotifyVideoDataUsage(1000);

        verify(mMockVideoListener, times(1)).onMediaSessionDataUsageChanged(
                eq(mMtcMediaSession), eq(1000L));
    }

    @Test
    public void testRttMessageReceived() throws Exception {
        mMtcMediaSession.rttMessageReceived("test");

        verify(mMockTextListener, times(1)).onRttMessageReceived(
                eq(mMtcMediaSession), eq("test"));
    }

    @Test
    public void testAudioSessionOpened() throws Exception {
        mMtcMediaSession.audioSessionOpened();

        verify(mMockAudioListener, times(1)).onAudioSessionOpened();
        verify(mMockMediaRegistry).addListener(any(MmTelMediaRegistry.Listener.class));
    }

    @Test
    public void testAudioSessionClosed() throws Exception {
        mMtcMediaSession.audioSessionOpened();
        mMtcMediaSession.audioSessionClosed();

        verify(mMockAudioListener, times(1)).onAudioSessionClosed();
        verify(mMockMediaRegistry).removeListener(any(MmTelMediaRegistry.Listener.class));
    }

    @Test
    public void testCallQualityChanged() throws Exception {
        CallQuality callQuality = new CallQuality(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0);
        mMtcMediaSession.callQualityChanged(callQuality);

        verify(mMockAudioListener, times(1)).onCallQualityChanged(eq(callQuality));
    }

    @Test
    public void testRtpHeaderExtensionsReceived() throws Exception {
        Set<RtpHeaderExtension> extensions = new HashSet<>();
        mMtcMediaSession.rtpHeaderExtensionsReceived(extensions);

        verify(mMockAudioListener, times(1)).onRtpHeaderExtensionsReceived(eq(extensions));
    }

    @Test
    public void testOnNotifyIncomingDtmfReceived() throws Exception {
        mMtcMediaSession.onNotifyIncomingDtmfReceived(ToneGenerator.TONE_DTMF_1, 300);

        verify(mMockAudioListener, times(1)).onNotifyIncomingDtmfReceived(
                eq(ToneGenerator.TONE_DTMF_1));
    }

    @Test
    public void testVideoSessionOpened_processesPendingCameraSelection() throws Exception {
        when(mMockCall.getNativeCallId()).thenReturn(1L);
        mMtcMediaSession.selectCamera(1);
        mMtcMediaSession.videoSessionOpened();

        verify(mMockMtcJniProxy, times(1)).sendDataToNative(eq(1L), any(Parcel.class));
    }

    @Test
    public void testOnNotifyRtpReceptionStats() throws Exception {
        final int nRtpTs = 12345;
        final int nRtcpTs = 67890;
        final long nNtp = System.currentTimeMillis();
        final int nDelay = 50;
        final int nRoundtrip = 100;

        RtpReceptionStats stats = new RtpReceptionStats.Builder()
                .setRtpTimestamp(nRtpTs)
                .setRtcpSrTimestamp(nRtcpTs)
                .setRtcpSrNtpTimestamp(nNtp)
                .setJitterBufferMs(nDelay)
                .setRoundTripTimeMs(nRoundtrip)
                .build();

        mMtcMediaSession.onNotifyRtpReceptionStats(MEDIA_TYPE_AUDIO, stats);

        verify(mMockMediaSession, times(1)).notifyRtpReceptionStats(
                eq(MEDIA_TYPE_AUDIO), eq(stats));
    }

    @Test
    public void testGetMediaThreshold() throws Exception {
        MediaThreshold threshold = mMtcMediaSession.getMediaThreshold(MEDIA_TYPE_AUDIO);

        assertNotNull(threshold);
        verify(mMockMediaRegistry, times(1)).getMediaThreshold(anyInt());
    }

    @Test
    public void testMediaQualityStatusChanged() throws Exception {
        final int nRtpPacketLossRate = 10;
        final int nRtpJitterMillis = 20;
        final int nRtpInactivityTimeMillis = 20000;

        android.telephony.imsmedia.MediaQualityStatus mediaQualityStatus =
                new android.telephony.imsmedia.MediaQualityStatus.Builder()
                .setRtpInactivityTimeMillis(nRtpInactivityTimeMillis)
                .setRtcpInactivityTimeMillis(nRtpInactivityTimeMillis + 1)
                .setRtpPacketLossRate(nRtpPacketLossRate)
                .setRtpJitterMillis(nRtpJitterMillis)
                .build();

        mMtcMediaSession.mediaQualityStatusChanged(
                MEDIA_TYPE_AUDIO, RAT_EUTRAN, mediaQualityStatus);

        verify(mMockMediaQualityReporter, times(1)).notifyMediaQualityStatusChanged(
                eq(MediaQualityStatus.MEDIA_SESSION_TYPE_AUDIO),
                eq(AccessNetworkConstants.TRANSPORT_TYPE_WWAN),
                eq(nRtpPacketLossRate),
                eq(nRtpJitterMillis),
                eq((long) nRtpInactivityTimeMillis)
        );
    }

    @Test
    public void testTriggerAnbrQuery() throws Exception {
        mMtcMediaSession.triggerAnbrQuery(MEDIA_TYPE_AUDIO, MEDIA_DIRECTION_UL, 5400);

        verify(mMockAudioListener, times(1)).onTriggerAnbrQueryReceived(
                eq(MEDIA_TYPE_AUDIO), eq(MEDIA_DIRECTION_UL), eq(5400));
    }

    @Test
    public void testSelectCamera() throws Exception {
        when(mMockCall.getNativeCallId()).thenReturn(1L);
        mMtcMediaSession.videoSessionOpened();
        mMtcMediaSession.selectCamera(1);

        verify(mMockMtcJniProxy, times(1)).sendDataToNative(eq(1L), any(Parcel.class));
    }

    @Test
    public void testSetCameraZoom() throws Exception {
        when(mMockCall.getNativeCallId()).thenReturn(1L);
        mMtcMediaSession.setCameraZoom(1);

        verify(mMockMtcJniProxy, times(1)).sendDataToNative(eq(1L), any(Parcel.class));
    }

    @Test
    public void testSetDeviceOrientation() throws Exception {
        when(mMockCall.getNativeCallId()).thenReturn(1L);
        mMtcMediaSession.setDeviceOrientation(1);

        verify(mMockMtcJniProxy, times(1)).sendDataToNative(eq(1L), any(Parcel.class));
    }

    @Test
    public void testRequestCallDataUsage() throws Exception {
        mMtcMediaSession.requestCallDataUsage();
        verify(mMockMediaSession).requestCallDataUsage();
    }

    @Test
    public void testNotifyMediaInfoChanged() throws Exception {
        mMtcMediaSession.notifyMediaInfoChanged(MEDIA_TYPE_VIDEO, 2, "test");
        verify(mMockVideoListener).onMediaSessionMediaInfoChanged(
                eq(mMtcMediaSession), eq(MEDIA_TYPE_VIDEO), eq(2), eq("test"));
    }

    @Test
    public void testOnMessage() throws Exception {
        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IUMtcMedia.IMS_MSG_BASE_MEDIA + 1);
        parcel.setDataPosition(0);

        mMtcMediaSession.onMessage(parcel);
        verify(mMockMediaListener, times(1)).onMediaMessage(any(Parcel.class));
    }

    @Test
    public void testSendRttMessage() throws Exception {
        mMtcMediaSession.sendRttMessage("test");

        verify(mMockMediaListener, times(1)).onMediaMessage(any(Parcel.class));
    }

    @Test
    public void testSendRtpHeaderExtensions() throws Exception {
        Set<RtpHeaderExtension> extensions = new HashSet<>();
        mMtcMediaSession.sendRtpHeaderExtensions(extensions);

        verify(mMockMediaListener, times(1)).onMediaMessage(any(Parcel.class));
    }

    @Test
    public void testNotifyAnbr() throws Exception {
        mMtcMediaSession.notifyAnbr(MEDIA_TYPE_AUDIO, MEDIA_DIRECTION_UL, 5400);

        verify(mMockMediaListener, times(1)).onMediaMessage(any(Parcel.class));
    }

    @Test
    public void testSendRequest() throws Exception {
        Parcel parcel = Parcel.obtain();
        parcel.writeInt(0);
        parcel.setDataPosition(0);

        when(mMockCall.getNativeCallId()).thenReturn(0L);
        mMtcMediaSession.sendRequest(parcel);

        verify(mMockMediaListener, times(0)).onMediaMessage(any(Parcel.class));
    }

    @Test
    public void testNotifyMediaThresholdChanged() throws Exception {
        when(mMockCall.getNativeCallId()).thenReturn(1L);
        int[] packetLossRate = {1, 20};
        int[] jitterMillies = {20};
        long[] inactivityTimeMillies = {5000, 20000};

        MediaThreshold mediaThreshold = new MediaThreshold.Builder()
                .setThresholdsRtpPacketLossRate(packetLossRate)
                .setThresholdsRtpJitterMillis(jitterMillies)
                .setThresholdsRtpInactivityTimeMillis(inactivityTimeMillies)
                .build();

        mMtcMediaSession.notifyMediaThresholdChanged(
                MediaQualityStatus.MEDIA_SESSION_TYPE_AUDIO, mediaThreshold);
        verify(mMockMediaListener).onMediaMessage(any(Parcel.class));
    }

    @Test
    public void testDispose() {
        mMtcMediaSession.audioSessionOpened();
        mMtcMediaSession.audioSessionClosed();

        mMtcMediaSession.dispose();
        verify(mMockMediaRegistry).removeListener(any(MmTelMediaRegistry.Listener.class));
    }
}
