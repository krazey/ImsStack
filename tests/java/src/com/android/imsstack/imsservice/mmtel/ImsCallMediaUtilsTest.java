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
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.when;

import android.telecom.TelecomManager;
import android.telecom.VideoProfile;
import android.telephony.CarrierConfigManager;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsStreamMediaProfile;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.mtc.MediaInfo;
import com.android.imsstack.imsservice.mmtel.base.ICallContext;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class ImsCallMediaUtilsTest {
    private static final int SLOT_ID = 0;

    //Mocked classes
    @Mock CarrierConfig mMockCarrierConfig;
    @Mock ConfigInterface mMockConfigInterface;

    @Before
    public void setUp() {
        MockitoAnnotations.initMocks(this);
        when(mMockConfigInterface.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        AgentFactory.getInstance().setAgent(ConfigInterface.class, mMockConfigInterface, SLOT_ID);
    }

    @After
    public void tearDown() throws Exception {
        AgentFactory.getInstance().setAgent(ConfigInterface.class, null, SLOT_ID);
    }

    @Test
    public void testClearMediaProfile() {
        ImsStreamMediaProfile imsStreamMediaProfile = new ImsStreamMediaProfile();
        imsStreamMediaProfile.setRttMode(1);

        ImsCallMediaUtils.clearMediaProfile(imsStreamMediaProfile);
        assertEquals(imsStreamMediaProfile.isRttCall(), false);
    }

    @Test
    public void testCloneVideoProfile() {
        VideoProfile videoProfile = new VideoProfile(VideoProfile.STATE_TX_ENABLED,
                VideoProfile.QUALITY_HIGH);
        VideoProfile clonedVideoProfile = ImsCallMediaUtils.cloneVideoProfile(videoProfile);
        assertEquals(videoProfile.getVideoState(), clonedVideoProfile.getVideoState());
        assertEquals(videoProfile.getQuality(), clonedVideoProfile.getQuality());
    }

    @Test
    public void testCreateMediaInfoForCallAcceptSetsDirectionForVoiceCall() {
        ImsCallProfile profile = new ImsCallProfile();

        profile.getMediaProfile().mAudioDirection = ImsStreamMediaProfile.DIRECTION_INVALID;
        assertEquals(
                ImsCallMediaUtils.createMediaInfoForCallAccept(
                        profile, ImsCallProfile.CALL_TYPE_VOICE, 0, 0).ADir,
                MediaInfo.DIRECTION_INVALID);

        profile.getMediaProfile().mAudioDirection = ImsStreamMediaProfile.DIRECTION_INACTIVE;
        assertEquals(
                ImsCallMediaUtils.createMediaInfoForCallAccept(
                        profile, ImsCallProfile.CALL_TYPE_VOICE, 0, 0).ADir,
                MediaInfo.DIRECTION_INACTIVE);

        profile.getMediaProfile().mAudioDirection = ImsStreamMediaProfile.DIRECTION_RECEIVE;
        assertEquals(
                ImsCallMediaUtils.createMediaInfoForCallAccept(
                        profile, ImsCallProfile.CALL_TYPE_VOICE, 0, 0).ADir,
                MediaInfo.DIRECTION_RECEIVE);

        profile.getMediaProfile().mAudioDirection = ImsStreamMediaProfile.DIRECTION_SEND;
        assertEquals(
                ImsCallMediaUtils.createMediaInfoForCallAccept(
                        profile, ImsCallProfile.CALL_TYPE_VOICE, 0, 0).ADir,
                MediaInfo.DIRECTION_SEND);

        profile.getMediaProfile().mAudioDirection = ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE;
        assertEquals(
                ImsCallMediaUtils.createMediaInfoForCallAccept(
                        profile, ImsCallProfile.CALL_TYPE_VOICE, 0, 0).ADir,
                MediaInfo.DIRECTION_SEND_RECEIVE);
    }

    @Test
    public void testCreateMediaInfoForCallAcceptSetsDirectionForVideoCall() {
        ImsCallProfile profile = new ImsCallProfile();

        profile.getMediaProfile().mVideoDirection = ImsStreamMediaProfile.DIRECTION_INVALID;
        assertEquals(
                ImsCallMediaUtils.createMediaInfoForCallAccept(
                        profile, ImsCallProfile.CALL_TYPE_VIDEO_N_VOICE, 0, 0).VDir,
                MediaInfo.DIRECTION_INVALID);

        profile.getMediaProfile().mVideoDirection = ImsStreamMediaProfile.DIRECTION_INACTIVE;
        assertEquals(
                ImsCallMediaUtils.createMediaInfoForCallAccept(
                        profile, ImsCallProfile.CALL_TYPE_VIDEO_N_VOICE, 0, 0).VDir,
                MediaInfo.DIRECTION_INACTIVE);

        profile.getMediaProfile().mVideoDirection = ImsStreamMediaProfile.DIRECTION_RECEIVE;
        assertEquals(
                ImsCallMediaUtils.createMediaInfoForCallAccept(
                        profile, ImsCallProfile.CALL_TYPE_VIDEO_N_VOICE, 0, 0).VDir,
                MediaInfo.DIRECTION_RECEIVE);

        profile.getMediaProfile().mVideoDirection = ImsStreamMediaProfile.DIRECTION_SEND;
        assertEquals(
                ImsCallMediaUtils.createMediaInfoForCallAccept(
                        profile, ImsCallProfile.CALL_TYPE_VIDEO_N_VOICE, 0, 0).VDir,
                MediaInfo.DIRECTION_SEND);

        profile.getMediaProfile().mVideoDirection = ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE;
        assertEquals(
                ImsCallMediaUtils.createMediaInfoForCallAccept(
                        profile, ImsCallProfile.CALL_TYPE_VIDEO_N_VOICE, 0, 0).VDir,
                MediaInfo.DIRECTION_SEND_RECEIVE);
    }

    @Test
    public void testCreateMediaInfoFromMediaProfile() {
        ImsStreamMediaProfile imsStreamMediaProfile = new ImsStreamMediaProfile(
                ImsStreamMediaProfile.AUDIO_QUALITY_AMR, ImsStreamMediaProfile.DIRECTION_SEND,
                ImsStreamMediaProfile.VIDEO_QUALITY_QCIF,
                ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE, ImsStreamMediaProfile.RTT_MODE_FULL);

        MediaInfo mediaInfo = ImsCallMediaUtils.createMediaInfoFromMediaProfile(
                imsStreamMediaProfile);
        assertEquals(mediaInfo.AQuality, ImsStreamMediaProfile.AUDIO_QUALITY_AMR);
    }

    @Test
    public void testCreateMediaInfoFromVideoProfile() {
        VideoProfile videoProfile = new VideoProfile(VideoProfile.STATE_TX_ENABLED,
                VideoProfile.QUALITY_HIGH);
        MediaInfo mediaInfo = ImsCallMediaUtils.createMediaInfoFromVideoProfile(videoProfile);
        assertEquals(mediaInfo.VDir, MediaInfo.DIRECTION_SEND);
    }

    @Test
    public void testCreateMediaProfileFromMediaInfo() {
        MediaInfo mediaInfo = new MediaInfo(MediaInfo.AUDIO_QUALITY_AMR_NB,
                MediaInfo.VIDEO_QUALITY_QCIF, MediaInfo.DIRECTION_SEND_RECEIVE,
                MediaInfo.DIRECTION_SEND_RECEIVE, MediaInfo.DIRECTION_INACTIVE,
                MediaInfo.GTTMODE_FULL);

        ImsStreamMediaProfile stramMediaProfile = ImsCallMediaUtils
                .createMediaProfileFromMediaInfo(mediaInfo);
        assertEquals(stramMediaProfile.getRttMode(), ImsStreamMediaProfile.RTT_MODE_FULL);

        MediaInfo newMediaInfo = new MediaInfo(MediaInfo.AUDIO_QUALITY_AMR_NB,
                MediaInfo.VIDEO_QUALITY_NONE, MediaInfo.DIRECTION_SEND_RECEIVE,
                MediaInfo.DIRECTION_SEND_RECEIVE, MediaInfo.DIRECTION_INACTIVE,
                MediaInfo.GTTMODE_FULL);

        ImsStreamMediaProfile newStramMediaProfile = ImsCallMediaUtils
                .createMediaProfileFromMediaInfo(newMediaInfo);
        assertEquals(newStramMediaProfile.getVideoDirection(),
                ImsStreamMediaProfile.DIRECTION_INVALID);
    }

    @Test
    public void testCreateVideoProfileFromMediaInfo() {
        MediaInfo mediaInfo = new MediaInfo(MediaInfo.AUDIO_QUALITY_AMR_NB,
                MediaInfo.VIDEO_QUALITY_NOTUSED, MediaInfo.DIRECTION_SEND_RECEIVE,
                MediaInfo.DIRECTION_SEND_RECEIVE, MediaInfo.DIRECTION_INACTIVE,
                MediaInfo.GTTMODE_FULL);

        VideoProfile videoProfile = ImsCallMediaUtils.createVideoProfileFromMediaInfo(mediaInfo);
        assertEquals(videoProfile.getVideoState(), VideoProfile.STATE_AUDIO_ONLY);

        MediaInfo mediaInfo1 = new MediaInfo(MediaInfo.AUDIO_QUALITY_AMR_NB,
                MediaInfo.VIDEO_QUALITY_QCIF, MediaInfo.DIRECTION_SEND_RECEIVE,
                MediaInfo.DIRECTION_INACTIVE, MediaInfo.DIRECTION_INACTIVE,
                MediaInfo.GTTMODE_FULL);

        VideoProfile videoProfile1 = ImsCallMediaUtils.createVideoProfileFromMediaInfo(mediaInfo1);
        assertEquals(videoProfile1.getVideoState(), VideoProfile.STATE_PAUSED);

        MediaInfo mediaInfo2 = new MediaInfo(MediaInfo.AUDIO_QUALITY_AMR_NB,
                MediaInfo.VIDEO_QUALITY_QCIF, MediaInfo.DIRECTION_SEND_RECEIVE,
                MediaInfo.DIRECTION_RECEIVE, MediaInfo.DIRECTION_INACTIVE,
                MediaInfo.GTTMODE_FULL);

        VideoProfile videoProfile2 = ImsCallMediaUtils.createVideoProfileFromMediaInfo(mediaInfo2);
        assertEquals(videoProfile2.getVideoState(), VideoProfile.STATE_RX_ENABLED);

        MediaInfo mediaInfo3 = new MediaInfo(MediaInfo.AUDIO_QUALITY_AMR_NB,
                MediaInfo.VIDEO_QUALITY_QCIF, MediaInfo.DIRECTION_SEND_RECEIVE,
                MediaInfo.DIRECTION_SEND, MediaInfo.DIRECTION_INACTIVE,
                MediaInfo.GTTMODE_FULL);

        VideoProfile videoProfile3 = ImsCallMediaUtils.createVideoProfileFromMediaInfo(mediaInfo3);
        assertEquals(videoProfile3.getVideoState(), VideoProfile.STATE_TX_ENABLED);

        MediaInfo mediaInfo4 = new MediaInfo(MediaInfo.AUDIO_QUALITY_AMR_NB,
                MediaInfo.VIDEO_QUALITY_QCIF, MediaInfo.DIRECTION_SEND_RECEIVE,
                MediaInfo.DIRECTION_SEND_RECEIVE, MediaInfo.DIRECTION_INACTIVE,
                MediaInfo.GTTMODE_FULL);

        VideoProfile videoProfile4 = ImsCallMediaUtils.createVideoProfileFromMediaInfo(mediaInfo4);
        assertEquals(videoProfile4.getVideoState(), VideoProfile.STATE_BIDIRECTIONAL);

    }

    @Test
    public void testGetDirectionFromMediaInfoForMediaProfile() {
        int dir = ImsCallMediaUtils.getDirectionFromMediaInfoForMediaProfile(
                MediaInfo.DIRECTION_INACTIVE);
        assertEquals(dir, ImsStreamMediaProfile.DIRECTION_INACTIVE);

        int dir1 = ImsCallMediaUtils.getDirectionFromMediaInfoForMediaProfile(
                MediaInfo.DIRECTION_SEND);
        assertEquals(dir1, ImsStreamMediaProfile.DIRECTION_SEND);

        int dir2 = ImsCallMediaUtils.getDirectionFromMediaInfoForMediaProfile(
                MediaInfo.DIRECTION_RECEIVE);
        assertEquals(dir2, ImsStreamMediaProfile.DIRECTION_RECEIVE);

        int dir3 = ImsCallMediaUtils.getDirectionFromMediaInfoForMediaProfile(
                MediaInfo.DIRECTION_SEND_RECEIVE);
        assertEquals(dir3, ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE);

        int dir4 = ImsCallMediaUtils.getDirectionFromMediaInfoForMediaProfile(
                MediaInfo.DIRECTION_INVALID);
        assertEquals(dir4, ImsStreamMediaProfile.DIRECTION_INVALID);

    }

    @Test
    public void testGetDirectionFromMediaProfileForMediaInfo() {
        int dir = ImsCallMediaUtils.getDirectionFromMediaProfileForMediaInfo(
                ImsStreamMediaProfile.DIRECTION_INACTIVE);
        assertEquals(dir, MediaInfo.DIRECTION_INACTIVE);

        int dir1 = ImsCallMediaUtils.getDirectionFromMediaProfileForMediaInfo(
                ImsStreamMediaProfile.DIRECTION_SEND);
        assertEquals(dir1, MediaInfo.DIRECTION_SEND);

        int dir2 = ImsCallMediaUtils.getDirectionFromMediaProfileForMediaInfo(
                ImsStreamMediaProfile.DIRECTION_RECEIVE);
        assertEquals(dir2, MediaInfo.DIRECTION_RECEIVE);

        int dir3 = ImsCallMediaUtils.getDirectionFromMediaProfileForMediaInfo(
                ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE);
        assertEquals(dir3, MediaInfo.DIRECTION_SEND_RECEIVE);

        int dir4 = ImsCallMediaUtils.getDirectionFromMediaProfileForMediaInfo(
                ImsStreamMediaProfile.DIRECTION_INVALID);
        assertEquals(dir4, MediaInfo.DIRECTION_INVALID);
    }

    @Test
    public void testGetDirectionFromVideoProfileForMediaInfo() {
        int dir = ImsCallMediaUtils.getDirectionFromVideoProfileForMediaInfo(
                VideoProfile.STATE_PAUSED);
        assertEquals(dir, MediaInfo.DIRECTION_INACTIVE);

        int dir1 = ImsCallMediaUtils.getDirectionFromVideoProfileForMediaInfo(
                VideoProfile.STATE_RX_ENABLED);
        assertEquals(dir1, MediaInfo.DIRECTION_RECEIVE);

        int dir2 = ImsCallMediaUtils.getDirectionFromVideoProfileForMediaInfo(
                VideoProfile.STATE_TX_ENABLED);
        assertEquals(dir2, MediaInfo.DIRECTION_SEND);

        int dir3 = ImsCallMediaUtils.getDirectionFromVideoProfileForMediaInfo(
                VideoProfile.STATE_BIDIRECTIONAL);
        assertEquals(dir3, MediaInfo.DIRECTION_SEND_RECEIVE);

        int dir4 = ImsCallMediaUtils.getDirectionFromVideoProfileForMediaInfo(
                VideoProfile.STATE_AUDIO_ONLY);
        assertEquals(dir4, MediaInfo.DIRECTION_INVALID);
    }

    @Test
    public void testGetAudioQualityFromMediaInfoForMediaProfile() {
        int auQualty = ImsCallMediaUtils.getAudioQualityFromMediaInfoForMediaProfile(
                MediaInfo.AUDIO_QUALITY_EVS);
        assertEquals(auQualty, ImsStreamMediaProfile.AUDIO_QUALITY_EVS_SWB);

        int auQualty1 = ImsCallMediaUtils.getAudioQualityFromMediaInfoForMediaProfile(
                MediaInfo.AUDIO_QUALITY_AMR_NB);
        assertEquals(auQualty1, ImsStreamMediaProfile.AUDIO_QUALITY_AMR);

        int auQualty2 = ImsCallMediaUtils.getAudioQualityFromMediaInfoForMediaProfile(
                MediaInfo.AUDIO_QUALITY_AMR_WB);
        assertEquals(auQualty2, ImsStreamMediaProfile.AUDIO_QUALITY_AMR_WB);

        int auQualty3 = ImsCallMediaUtils.getAudioQualityFromMediaInfoForMediaProfile(
                MediaInfo.AUDIO_QUALITY_G711_PCMU);
        assertEquals(auQualty3, ImsStreamMediaProfile.AUDIO_QUALITY_G711U);

        int auQualty4 = ImsCallMediaUtils.getAudioQualityFromMediaInfoForMediaProfile(
                MediaInfo.AUDIO_QUALITY_G711_PCMA);
        assertEquals(auQualty4, ImsStreamMediaProfile.AUDIO_QUALITY_G711A);

        int auQualty5 = ImsCallMediaUtils.getAudioQualityFromMediaInfoForMediaProfile(
                MediaInfo.AUDIO_QUALITY_EVS_NB);
        assertEquals(auQualty5, ImsStreamMediaProfile.AUDIO_QUALITY_EVS_NB);

        int auQualty6 = ImsCallMediaUtils.getAudioQualityFromMediaInfoForMediaProfile(
                MediaInfo.AUDIO_QUALITY_EVS_WB);
        assertEquals(auQualty6, ImsStreamMediaProfile.AUDIO_QUALITY_EVS_WB);

        int auQualty7 = ImsCallMediaUtils.getAudioQualityFromMediaInfoForMediaProfile(
                MediaInfo.AUDIO_QUALITY_EVS_SWB);
        assertEquals(auQualty7, ImsStreamMediaProfile.AUDIO_QUALITY_EVS_SWB);

        int auQualty8 = ImsCallMediaUtils.getAudioQualityFromMediaInfoForMediaProfile(
                MediaInfo.AUDIO_QUALITY_EVS_FB);
        assertEquals(auQualty8, ImsStreamMediaProfile.AUDIO_QUALITY_EVS_FB);
    }

    @Test
    public void testGetAudioQualityFromMediaProfileForMediaInfo() {
        int auQualty = ImsCallMediaUtils.getAudioQualityFromMediaProfileForMediaInfo(
                ImsStreamMediaProfile.AUDIO_QUALITY_EVS_FB);
        assertEquals(auQualty, MediaInfo.AUDIO_QUALITY_EVS_FB);

        int auQualty1 = ImsCallMediaUtils.getAudioQualityFromMediaProfileForMediaInfo(
                ImsStreamMediaProfile.AUDIO_QUALITY_AMR);
        assertEquals(auQualty1, MediaInfo.AUDIO_QUALITY_AMR_NB);
    }

    @Test
    public void testGetVideoQualityFromMediaInfoForMediaProfile() {
        int viQuality = ImsCallMediaUtils.getVideoQualityFromMediaInfoForMediaProfile(
                MediaInfo.VIDEO_QUALITY_QCIF);
        assertEquals(viQuality, ImsStreamMediaProfile.VIDEO_QUALITY_QCIF);

        int viQuality1 = ImsCallMediaUtils.getVideoQualityFromMediaInfoForMediaProfile(
                 MediaInfo.VIDEO_QUALITY_QVGA_LS);
        assertEquals(viQuality1, ImsStreamMediaProfile.VIDEO_QUALITY_QVGA_LANDSCAPE);
    }

    @Test
    public void testGetVideoQualityFromMediaProfileForMediaInfo() {
        int viQuality = ImsCallMediaUtils.getVideoQualityFromMediaProfileForMediaInfo(
                ImsStreamMediaProfile.VIDEO_QUALITY_QCIF);
        assertEquals(viQuality, MediaInfo.VIDEO_QUALITY_QCIF);

        int viQuality1 = ImsCallMediaUtils.getVideoQualityFromMediaProfileForMediaInfo(
                ImsStreamMediaProfile.VIDEO_QUALITY_QVGA_LANDSCAPE);
        assertEquals(viQuality1, MediaInfo.VIDEO_QUALITY_QVGA_LS);
    }

    @Test
    public void testGetTtyModeFromMediaInfoToTelecom() {
        int ttyMode = ImsCallMediaUtils.getTtyModeFromMediaInfoToTelecom(MediaInfo.GTTMODE_FULL);
        assertEquals(ttyMode, TelecomManager.TTY_MODE_FULL);

        int ttyMode1 = ImsCallMediaUtils.getTtyModeFromMediaInfoToTelecom(MediaInfo.GTTMODE_HCO);
        assertEquals(ttyMode1, TelecomManager.TTY_MODE_HCO);

        int ttyMode2 = ImsCallMediaUtils.getTtyModeFromMediaInfoToTelecom(MediaInfo.GTTMODE_VCO);
        assertEquals(ttyMode2, TelecomManager.TTY_MODE_VCO);

        int ttyMode3 = ImsCallMediaUtils.getTtyModeFromMediaInfoToTelecom(
                MediaInfo.GTTMODE_INVALID);
        assertEquals(ttyMode3, TelecomManager.TTY_MODE_OFF);
    }

    @Test
    public void testGetTtyModeFromTelecomToMediaInfo() {
        int ttyMode = ImsCallMediaUtils.getTtyModeFromTelecomToMediaInfo(
                TelecomManager.TTY_MODE_FULL);
        assertEquals(ttyMode, MediaInfo.GTTMODE_FULL);

        int ttyMode1 = ImsCallMediaUtils.getTtyModeFromTelecomToMediaInfo(
                TelecomManager.TTY_MODE_HCO);
        assertEquals(ttyMode1, MediaInfo.GTTMODE_HCO);

        int ttyMode2 = ImsCallMediaUtils.getTtyModeFromTelecomToMediaInfo(
                TelecomManager.TTY_MODE_VCO);
        assertEquals(ttyMode2, MediaInfo.GTTMODE_VCO);

        int ttyMode3 = ImsCallMediaUtils.getTtyModeFromTelecomToMediaInfo(
                TelecomManager.TTY_MODE_OFF);
        assertEquals(ttyMode3, MediaInfo.GTTMODE_INACTIVE);

        int ttyMode4 = ImsCallMediaUtils.getTtyModeFromTelecomToMediaInfo(-1);
        assertEquals(ttyMode4, MediaInfo.GTTMODE_INVALID);
    }

    @Test
    public void testGetVideoCallType() {
        ImsStreamMediaProfile imsStreamMediaProfile = new ImsStreamMediaProfile(
                ImsStreamMediaProfile.AUDIO_QUALITY_AMR, ImsStreamMediaProfile.DIRECTION_SEND,
                ImsStreamMediaProfile.VIDEO_QUALITY_QCIF,
                ImsStreamMediaProfile.DIRECTION_RECEIVE, ImsStreamMediaProfile.RTT_MODE_FULL);

        int videocallType = ImsCallMediaUtils.getVideoCallType(imsStreamMediaProfile);
        assertEquals(videocallType, ImsCallProfile.CALL_TYPE_VT_RX);

        ImsStreamMediaProfile imsStreamMediaProfile1 = new ImsStreamMediaProfile(
                ImsStreamMediaProfile.AUDIO_QUALITY_AMR, ImsStreamMediaProfile.DIRECTION_SEND,
                ImsStreamMediaProfile.VIDEO_QUALITY_QCIF,
                ImsStreamMediaProfile.DIRECTION_SEND, ImsStreamMediaProfile.RTT_MODE_FULL);

        int videocallType1 = ImsCallMediaUtils.getVideoCallType(imsStreamMediaProfile1);
        assertEquals(videocallType1, ImsCallProfile.CALL_TYPE_VT_TX);

        ImsStreamMediaProfile imsStreamMediaProfile2 = new ImsStreamMediaProfile(
                ImsStreamMediaProfile.AUDIO_QUALITY_AMR, ImsStreamMediaProfile.DIRECTION_SEND,
                ImsStreamMediaProfile.VIDEO_QUALITY_QCIF,
                ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE, ImsStreamMediaProfile.RTT_MODE_FULL);

        int videocallType2 = ImsCallMediaUtils.getVideoCallType(imsStreamMediaProfile2);
        assertEquals(videocallType2, ImsCallProfile.CALL_TYPE_VT);

        int videocallType3 = ImsCallMediaUtils.getVideoCallType(null);
        assertEquals(videocallType3, ImsCallProfile.CALL_TYPE_VT);
    }

    @Test
    public void testGetVideoDirectionFromCallType() {
        int vDir = ImsCallMediaUtils.getVideoDirectionFromCallType(ImsCallProfile
                .CALL_TYPE_VIDEO_N_VOICE);
        assertEquals(vDir, ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE);

        int vDir1 = ImsCallMediaUtils.getVideoDirectionFromCallType(ImsCallProfile
                .CALL_TYPE_VT_RX);
        assertEquals(vDir1, ImsStreamMediaProfile.DIRECTION_RECEIVE);

        int vDir2 = ImsCallMediaUtils.getVideoDirectionFromCallType(ImsCallProfile
                .CALL_TYPE_VT_TX);
        assertEquals(vDir2, ImsStreamMediaProfile.DIRECTION_SEND);

        int vDir3 = ImsCallMediaUtils.getVideoDirectionFromCallType(ImsCallProfile
                .CALL_TYPE_VT_NODIR);
        assertEquals(vDir3, ImsStreamMediaProfile.DIRECTION_INACTIVE);

        int vDir4 = ImsCallMediaUtils.getVideoDirectionFromCallType(ImsCallProfile
                .CALL_TYPE_VT);
        assertEquals(vDir4, ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE);

        int vDir5 = ImsCallMediaUtils.getVideoDirectionFromCallType(-1);
        assertEquals(vDir5, ImsStreamMediaProfile.DIRECTION_INVALID);
    }

    @Test
    public void testGetRttModeFromGTTMode() {
        int rttMode = ImsCallMediaUtils.getRttModeFromGTTMode(MediaInfo.GTTMODE_FULL);
        assertEquals(rttMode, ImsStreamMediaProfile.RTT_MODE_FULL);

        int rttMode1 = ImsCallMediaUtils.getRttModeFromGTTMode(MediaInfo.GTTMODE_HCO);
        assertEquals(rttMode1, ImsStreamMediaProfile.RTT_MODE_FULL);

        int rttMode2 = ImsCallMediaUtils.getRttModeFromGTTMode(MediaInfo.GTTMODE_VCO);
        assertEquals(rttMode2, ImsStreamMediaProfile.RTT_MODE_FULL);

        int rttMode3 = ImsCallMediaUtils.getRttModeFromGTTMode(MediaInfo.GTTMODE_INACTIVE);
        assertEquals(rttMode3, ImsStreamMediaProfile.RTT_MODE_DISABLED);
    }

    @Test
    public void testGetGttModeFromRttMode() {
        int gttMode = ImsCallMediaUtils.getGttModeFromRttMode(ImsStreamMediaProfile.RTT_MODE_FULL);
        assertEquals(gttMode, MediaInfo.GTTMODE_FULL);

        int gttMode1 = ImsCallMediaUtils.getGttModeFromRttMode(
                ImsStreamMediaProfile.RTT_MODE_DISABLED);
        assertEquals(gttMode1, MediaInfo.GTTMODE_INACTIVE);
    }

    @Test
    public void testGetDirectionFromGTTMode() {
        int dir = ImsCallMediaUtils.getDirectionFromGTTMode(MediaInfo.GTTMODE_FULL);
        assertEquals(dir, MediaInfo.DIRECTION_SEND_RECEIVE);

        int dir1 = ImsCallMediaUtils.getDirectionFromGTTMode(MediaInfo.GTTMODE_HCO);
        assertEquals(dir1, MediaInfo.DIRECTION_SEND_RECEIVE);

        int dir2 = ImsCallMediaUtils.getDirectionFromGTTMode(MediaInfo.GTTMODE_VCO);
        assertEquals(dir2, MediaInfo.DIRECTION_SEND_RECEIVE);

        int dir3 = ImsCallMediaUtils.getDirectionFromGTTMode(MediaInfo.GTTMODE_INACTIVE);
        assertEquals(dir3, MediaInfo.DIRECTION_INVALID);
    }

    @Test
    public void testIsAudioEvsCategory() {
        assertTrue(ImsCallMediaUtils.isAudioEvsCategory(
                ImsStreamMediaProfile.AUDIO_QUALITY_EVS_NB));
        assertTrue(ImsCallMediaUtils.isAudioEvsCategory(
                ImsStreamMediaProfile.AUDIO_QUALITY_EVS_WB));
        assertTrue(ImsCallMediaUtils.isAudioEvsCategory(
                ImsStreamMediaProfile.AUDIO_QUALITY_EVS_SWB));
        assertTrue(ImsCallMediaUtils.isAudioEvsCategory(
                ImsStreamMediaProfile.AUDIO_QUALITY_EVS_FB));
        assertFalse(ImsCallMediaUtils.isAudioEvsCategory(
                ImsStreamMediaProfile.AUDIO_QUALITY_NONE));
    }

    @Test
    public void testAudioQuality() {
        assertTrue(ImsCallMediaUtils.isAudioHDQuality(ImsStreamMediaProfile.AUDIO_QUALITY_AMR_WB));
        assertFalse(ImsCallMediaUtils.isAudioHDQuality(ImsStreamMediaProfile.AUDIO_QUALITY_AMR));
        assertTrue(ImsCallMediaUtils.isAudioUHDQuality(ImsStreamMediaProfile.AUDIO_QUALITY_EVS_FB));
        assertFalse(ImsCallMediaUtils.isAudioUHDQuality(ImsStreamMediaProfile.AUDIO_QUALITY_AMR));
    }

    @Test
    public void testIsDefaultMediaProfile() {
        ImsStreamMediaProfile mediaProfile = new ImsStreamMediaProfile(
                ImsStreamMediaProfile.AUDIO_QUALITY_AMR_WB,
                ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE,
                ImsStreamMediaProfile.VIDEO_QUALITY_NONE,
                ImsStreamMediaProfile.DIRECTION_INVALID);
        assertTrue(ImsCallMediaUtils.isDefaultMediaProfile(mediaProfile));

        ImsStreamMediaProfile mediaProfile1 = new ImsStreamMediaProfile(
                ImsStreamMediaProfile.AUDIO_QUALITY_NONE,
                ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE,
                ImsStreamMediaProfile.VIDEO_QUALITY_NONE,
                ImsStreamMediaProfile.DIRECTION_INVALID);
        assertTrue(ImsCallMediaUtils.isDefaultMediaProfile(mediaProfile1));

        ImsStreamMediaProfile mediaProfile2 = new ImsStreamMediaProfile(
                ImsStreamMediaProfile.AUDIO_QUALITY_NONE,
                ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE,
                ImsStreamMediaProfile.VIDEO_QUALITY_NONE,
                ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE);
        assertFalse(ImsCallMediaUtils.isDefaultMediaProfile(mediaProfile2));
    }

    @Test
    public void testIsDtmfEvent() {
        assertTrue(ImsCallMediaUtils.isDtmfEvent('0'));
        assertTrue(ImsCallMediaUtils.isDtmfEvent('9'));
        assertTrue(ImsCallMediaUtils.isDtmfEvent('A'));
        assertTrue(ImsCallMediaUtils.isDtmfEvent('D'));
        assertTrue(ImsCallMediaUtils.isDtmfEvent('*'));
        assertTrue(ImsCallMediaUtils.isDtmfEvent('#'));
        assertFalse(ImsCallMediaUtils.isDtmfEvent('Z'));
        assertFalse(ImsCallMediaUtils.isDtmfEvent('&'));
    }

    @Test
    public void testIsVideoProfileChanged() {
        ImsStreamMediaProfile mediaProfile = null;
        MediaInfo mediaInfo = null;
        assertFalse(ImsCallMediaUtils.isVideoProfileChanged(mediaProfile, mediaInfo));

        mediaProfile = new ImsStreamMediaProfile(
                ImsStreamMediaProfile.AUDIO_QUALITY_AMR_WB,
                ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE,
                ImsStreamMediaProfile.VIDEO_QUALITY_QCIF,
                ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE);

        mediaInfo = new MediaInfo(MediaInfo.AUDIO_QUALITY_AMR_NB,
                MediaInfo.VIDEO_QUALITY_QCIF, MediaInfo.DIRECTION_SEND_RECEIVE,
                MediaInfo.DIRECTION_SEND_RECEIVE, MediaInfo.DIRECTION_INACTIVE,
                MediaInfo.GTTMODE_FULL);

        assertFalse(ImsCallMediaUtils.isVideoProfileChanged(mediaProfile, mediaInfo));

        mediaProfile = new ImsStreamMediaProfile(
                ImsStreamMediaProfile.AUDIO_QUALITY_AMR_WB,
                ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE,
                ImsStreamMediaProfile.VIDEO_QUALITY_QCIF,
                ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE);

        mediaInfo = new MediaInfo(MediaInfo.AUDIO_QUALITY_AMR_NB,
                MediaInfo.VIDEO_QUALITY_NONE, MediaInfo.DIRECTION_SEND_RECEIVE,
                MediaInfo.DIRECTION_SEND_RECEIVE, MediaInfo.DIRECTION_INACTIVE,
                MediaInfo.GTTMODE_FULL);

        assertTrue(ImsCallMediaUtils.isVideoProfileChanged(mediaProfile, mediaInfo));
    }

    @Test
    public void testSetGttInfo() {
        MediaInfo mediaInfo = new MediaInfo(MediaInfo.AUDIO_QUALITY_AMR_NB,
                MediaInfo.VIDEO_QUALITY_QCIF, MediaInfo.DIRECTION_SEND_RECEIVE,
                MediaInfo.DIRECTION_SEND_RECEIVE, MediaInfo.DIRECTION_INACTIVE,
                MediaInfo.GTTMODE_INVALID);

        ImsCallMediaUtils.setGttInfo(mediaInfo, MediaInfo.DIRECTION_SEND_RECEIVE,
                MediaInfo.GTTMODE_FULL);
        assertEquals(mediaInfo.TDir, MediaInfo.DIRECTION_SEND_RECEIVE);
        assertEquals(mediaInfo.GTTMode, MediaInfo.GTTMODE_FULL);

        ImsCallMediaUtils.setGttInfo(null, MediaInfo.DIRECTION_INACTIVE,
                MediaInfo.GTTMODE_INVALID);
        assertEquals(mediaInfo.TDir, MediaInfo.DIRECTION_SEND_RECEIVE);
        assertEquals(mediaInfo.GTTMode, MediaInfo.GTTMODE_FULL);
    }

    @Test
    public void testSetRttInfo() {
        MediaInfo mediaInfo = new MediaInfo(MediaInfo.AUDIO_QUALITY_AMR_NB,
                MediaInfo.VIDEO_QUALITY_QCIF, MediaInfo.DIRECTION_SEND_RECEIVE,
                MediaInfo.DIRECTION_SEND_RECEIVE, MediaInfo.DIRECTION_INACTIVE,
                MediaInfo.GTTMODE_INVALID);

        ImsCallMediaUtils.setRttInfo(mediaInfo, MediaInfo.DIRECTION_SEND_RECEIVE, true);
        assertEquals(mediaInfo.TDir, MediaInfo.DIRECTION_SEND_RECEIVE);
        assertEquals(mediaInfo.GTTMode, MediaInfo.GTTMODE_FULL);

        ImsCallMediaUtils.setRttInfo(mediaInfo, MediaInfo.DIRECTION_SEND_RECEIVE, false);
        assertEquals(mediaInfo.TDir, MediaInfo.DIRECTION_SEND_RECEIVE);
        assertEquals(mediaInfo.GTTMode, MediaInfo.GTTMODE_INVALID);
    }

    @Test
    public void testSetMediaProfile() {
        ImsStreamMediaProfile mediaProfile = new ImsStreamMediaProfile(
                ImsStreamMediaProfile.AUDIO_QUALITY_NONE,
                ImsStreamMediaProfile.DIRECTION_INVALID,
                ImsStreamMediaProfile.VIDEO_QUALITY_NONE,
                ImsStreamMediaProfile.DIRECTION_INVALID);

        ImsCallMediaUtils.setMediaProfile(mediaProfile, ImsStreamMediaProfile.AUDIO_QUALITY_AMR_WB,
                ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE,
                ImsStreamMediaProfile.VIDEO_QUALITY_QCIF,
                ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE);

        assertEquals(mediaProfile.getVideoQuality(), ImsStreamMediaProfile.VIDEO_QUALITY_QCIF);
        assertEquals(mediaProfile.getVideoDirection(),
                ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE);
        assertEquals(mediaProfile.getRttMode(), ImsStreamMediaProfile.RTT_MODE_DISABLED);

        ImsCallMediaUtils.setMediaProfile(mediaProfile, ImsStreamMediaProfile.AUDIO_QUALITY_AMR_WB,
                ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE,
                ImsStreamMediaProfile.VIDEO_QUALITY_QCIF,
                ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE,
                ImsStreamMediaProfile.RTT_MODE_FULL);

        assertEquals(mediaProfile.getRttMode(), ImsStreamMediaProfile.RTT_MODE_FULL);
    }

    @Test
    public void testGetMediaProfileFromMediaInfo() {
        MediaInfo mediaInfo = new MediaInfo(MediaInfo.AUDIO_QUALITY_AMR_NB,
                MediaInfo.VIDEO_QUALITY_NONE, MediaInfo.DIRECTION_SEND_RECEIVE,
                MediaInfo.DIRECTION_SEND_RECEIVE, MediaInfo.DIRECTION_INACTIVE,
                MediaInfo.GTTMODE_INVALID);

        ImsStreamMediaProfile mediaProfile = ImsCallMediaUtils.getMediaProfileFromMediaInfo(
                mediaInfo);
        assertEquals(mediaProfile.getVideoDirection(), ImsStreamMediaProfile.DIRECTION_INVALID);
        assertEquals(mediaProfile.getAudioQuality(), ImsStreamMediaProfile.AUDIO_QUALITY_AMR);

        mediaProfile = ImsCallMediaUtils.getMediaProfileFromMediaInfo(null);
        assertEquals(mediaProfile.getVideoQuality(), ImsStreamMediaProfile.VIDEO_QUALITY_NONE);
        assertEquals(mediaProfile.getAudioQuality(), ImsStreamMediaProfile.AUDIO_QUALITY_NONE);
    }

    @Test
    public void testUpdateCallProfileFromMediaInfo() {
        ICallContext context = Mockito.mock(ICallContext.class);
        ImsCallProfile profile = new ImsCallProfile();
        MediaInfo mediaInfo = Mockito.mock(MediaInfo.class);
        mediaInfo.AQuality = ImsStreamMediaProfile.AUDIO_QUALITY_AMR_WB;
        mediaInfo.VQuality = ImsStreamMediaProfile.VIDEO_QUALITY_QCIF;
        mediaInfo.GTTMode = ImsStreamMediaProfile.RTT_MODE_DISABLED;
        when(mMockCarrierConfig.getBoolean(CarrierConfigManager.KEY_RTT_SUPPORTED_BOOL, false))
                .thenReturn(true);
        ImsCallMediaUtils.updateCallProfileFromMediaInfo(context, profile, mediaInfo);
        assertEquals(0, profile.getCallExtraInt(ImsCallMediaUtils.MEDIA_TEXT_DIRECTION));
        assertEquals(profile.getMediaProfile().mAudioQuality,
                ImsStreamMediaProfile.AUDIO_QUALITY_AMR_WB);
        assertEquals(profile.getMediaProfile().mVideoQuality,
                ImsStreamMediaProfile.VIDEO_QUALITY_QCIF);
        assertEquals(profile.getMediaProfile().mRttMode, ImsStreamMediaProfile.RTT_MODE_DISABLED);
    }
}
