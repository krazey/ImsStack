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

import android.telecom.TelecomManager;
import android.telecom.VideoProfile;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsStreamMediaProfile;
import android.util.Range;

import com.android.imsstack.enabler.mtc.AudioCodecAttributes;
import com.android.imsstack.enabler.mtc.MediaInfo;
import com.android.imsstack.enabler.mtc.MtcCallUtils;
import com.android.imsstack.imsservice.mmtel.base.ICallContext;
import com.android.imsstack.imsservice.mmtel.videocall.base.VideoCallUtils;

import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Set;

public class ImsCallMediaUtils {
    /**
     * Indicates if the video direction is overridden based on call type.
     * This feature should be enabled by Android Native implementation.
     */
    private static final boolean FEATURE_OVERRIDE_VIDEO_DIRECTION_FROM_CALL_TYPE = true;

    /**
     * DTMF characters
     */
    private static final String DTMF_EVENT = "0123456789*#ABCD";

    private static final LinkedHashMap<Integer, Integer> sAudioQualityForMediaProfileAndMediaInfo;

    /**
     * To save text media direction as an extra on ImsCallProfile
     */
    public static final String MEDIA_TEXT_DIRECTION = "media_text_direction";

    /**
     * DEBUGGING PURPOSE
     */
    public static void displayVideoQuality() {
        VideoCallUtils.displayVideoQuality();
    }

    public static void clearMediaProfile(ImsStreamMediaProfile profile) {
        if (profile != null) {
            profile.copyFrom(new ImsStreamMediaProfile(ImsStreamMediaProfile.AUDIO_QUALITY_NONE,
                    ImsStreamMediaProfile.DIRECTION_INVALID,
                    ImsStreamMediaProfile.VIDEO_QUALITY_NONE,
                    ImsStreamMediaProfile.DIRECTION_INVALID,
                    ImsStreamMediaProfile.RTT_MODE_DISABLED));
        }
    }

    public static VideoProfile cloneVideoProfile(VideoProfile profile) {
        return (profile == null) ?
                null : new VideoProfile(profile.getVideoState(), profile.getQuality());
    }

    // FIXME: May we use MediaInfo instead of ImsCallProfile?
    public static MediaInfo createMediaInfoForCallAccept(final ImsCallProfile profile,
            final int callType, final int audioCapabilities, final int videoCapabilities) {
        switch (callType) {
            case ImsCallProfile.CALL_TYPE_VOICE: // FALL-THROUGH
            case ImsCallProfile.CALL_TYPE_VOICE_N_VIDEO:
                return createMediaInfoForVoiceCallOnCallAccept(profile, audioCapabilities);

            case ImsCallProfile.CALL_TYPE_VIDEO_N_VOICE: // FALL-THROUGH
            case ImsCallProfile.CALL_TYPE_VT: // FALL-THROUGH
            case ImsCallProfile.CALL_TYPE_VT_TX: // FALL-THROUGH
            case ImsCallProfile.CALL_TYPE_VT_RX: // FALL-THROUGH
            case ImsCallProfile.CALL_TYPE_VT_NODIR:
                return createMediaInfoForVideoCallOnCallAccept(profile, callType, audioCapabilities,
                        videoCapabilities);

            default:
                break;
        }

        return new MediaInfo(
                MediaInfo.AUDIO_QUALITY_NONE, MediaInfo.VIDEO_QUALITY_NONE,
                MediaInfo.DIRECTION_INVALID, MediaInfo.DIRECTION_INVALID,
                MediaInfo.DIRECTION_INVALID, MediaInfo.GTTMODE_INVALID);
    }

    public static MediaInfo createMediaInfoFromMediaProfile(final ImsStreamMediaProfile profile) {
        return new MediaInfo(
                getAudioQualityFromMediaProfileForMediaInfo(profile.getAudioQuality()),
                getVideoQualityFromMediaProfileForMediaInfo(profile.getVideoQuality()),
                getDirectionFromMediaProfileForMediaInfo(profile.getAudioDirection()),
                getDirectionFromMediaProfileForMediaInfo(profile.getVideoDirection()),
                profile.isRttCall() ?
                        MediaInfo.DIRECTION_SEND_RECEIVE : MediaInfo.DIRECTION_INVALID,
                getGttModeFromRttMode(profile.getRttMode()));
    }

    public static MediaInfo createMediaInfoFromVideoProfile(final VideoProfile profile) {
        return new MediaInfo(MediaInfo.AUDIO_QUALITY_NONE, MediaInfo.VIDEO_QUALITY_NONE,
                MediaInfo.DIRECTION_SEND_RECEIVE,
                getDirectionFromVideoProfileForMediaInfo(profile.getVideoState()),
                MediaInfo.DIRECTION_INVALID, MediaInfo.GTTMODE_INVALID);
    }

    public static ImsStreamMediaProfile createMediaProfileFromMediaInfo(final MediaInfo mi) {
        int audioQuality = getAudioQualityFromMediaInfoForMediaProfile(mi.audioQuality);
        int audioDirection = getDirectionFromMediaInfoForMediaProfile(mi.audioDir);
        int videoQuality = getVideoQualityFromMediaInfoForMediaProfile(mi.videoQuality);
        int videoDirection = getDirectionFromMediaInfoForMediaProfile(mi.videoDir);
        int rttMode = getRttModeFromGTTMode(mi.gttMode);

        if (videoQuality == ImsStreamMediaProfile.VIDEO_QUALITY_NONE) {
            videoDirection = ImsStreamMediaProfile.DIRECTION_INVALID;
        }

        ImsStreamMediaProfile mediaProfile = new ImsStreamMediaProfile(audioQuality, audioDirection,
                videoQuality, videoDirection, rttMode);
        updateMediaProfileFromMediaInfoForAudioCodecAttributes(mediaProfile, mi);

        return mediaProfile;
    }

    public static VideoProfile createVideoProfileFromMediaInfo(final MediaInfo mi) {
        int videoState = VideoProfile.STATE_AUDIO_ONLY;

        if (MtcCallUtils.hasVideoQuality(mi)) {
            switch (mi.videoDir) {
            case MediaInfo.DIRECTION_INACTIVE:
                videoState = VideoProfile.STATE_PAUSED;
                break;
            case MediaInfo.DIRECTION_RECEIVE:
                videoState = VideoProfile.STATE_RX_ENABLED;
                break;
            case MediaInfo.DIRECTION_SEND:
                videoState = VideoProfile.STATE_TX_ENABLED;
                break;
            case MediaInfo.DIRECTION_SEND_RECEIVE:
                videoState = VideoProfile.STATE_BIDIRECTIONAL;
                break;
            default:
                break;
            }
        }

        return new VideoProfile(videoState);
    }

    public static int getDirectionFromMediaInfoForMediaProfile(int direction) {
        switch (direction) {
        case MediaInfo.DIRECTION_INACTIVE:
            return ImsStreamMediaProfile.DIRECTION_INACTIVE;
        case MediaInfo.DIRECTION_SEND:
            return ImsStreamMediaProfile.DIRECTION_SEND;
        case MediaInfo.DIRECTION_RECEIVE:
            return ImsStreamMediaProfile.DIRECTION_RECEIVE;
        case MediaInfo.DIRECTION_SEND_RECEIVE:
            return ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE;
        default:
            return ImsStreamMediaProfile.DIRECTION_INVALID;
        }
    }

    public static int getDirectionFromMediaProfileForMediaInfo(int direction) {
        switch (direction) {
        case ImsStreamMediaProfile.DIRECTION_INACTIVE:
            return MediaInfo.DIRECTION_INACTIVE;
        case ImsStreamMediaProfile.DIRECTION_SEND:
            return MediaInfo.DIRECTION_SEND;
        case ImsStreamMediaProfile.DIRECTION_RECEIVE:
            return MediaInfo.DIRECTION_RECEIVE;
        case ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE:
            return MediaInfo.DIRECTION_SEND_RECEIVE;
        default:
            return MediaInfo.DIRECTION_INVALID;
        }
    }

    public static int getDirectionFromVideoProfileForMediaInfo(int videoState) {
        if (VideoProfile.isPaused(videoState)) {
            return MediaInfo.DIRECTION_INACTIVE;
        } else if (VideoProfile.isBidirectional(videoState)) {
            return MediaInfo.DIRECTION_SEND_RECEIVE;
        } else if (VideoProfile.isTransmissionEnabled(videoState)) {
            return MediaInfo.DIRECTION_SEND;
        } else if (VideoProfile.isReceptionEnabled(videoState)) {
            return MediaInfo.DIRECTION_RECEIVE;
        } else {
            return MediaInfo.DIRECTION_INVALID;
        }
    }

    public static int getAudioQualityFromMediaInfoForMediaProfile(int audioQuality) {
        if (audioQuality == MediaInfo.AUDIO_QUALITY_EVS) {
            return ImsStreamMediaProfile.AUDIO_QUALITY_EVS_SWB;
        }

        int smpAQ = ImsStreamMediaProfile.AUDIO_QUALITY_NONE;
        Set<Map.Entry<Integer, Integer>> aqSet
                = sAudioQualityForMediaProfileAndMediaInfo.entrySet();

        if (aqSet != null) {
            for (Map.Entry<Integer, Integer> entry : aqSet) {
                if (audioQuality == entry.getValue().intValue()) {
                    smpAQ = entry.getKey().intValue();
                    break;
                }
            }
        }

        return smpAQ;
    }

    public static int getAudioQualityFromMediaProfileForMediaInfo(int audioQuality) {
        Integer aq = sAudioQualityForMediaProfileAndMediaInfo.get(audioQuality);
        return (aq != null) ? aq.intValue() : MediaInfo.AUDIO_QUALITY_NONE;
    }

    public static int getVideoQualityFromMediaInfoForMediaProfile(int videoQuality) {
        return VideoCallUtils.getVideoQualityFromMediaInfoForMediaProfile(videoQuality);
    }

    public static int getVideoQualityFromMediaProfileForMediaInfo(int videoQuality) {
        return VideoCallUtils.getVideoQualityFromMediaProfileForMediaInfo(videoQuality);
    }

    public static int getTtyModeFromMediaInfoToTelecom(int ttyMode) {
        switch (ttyMode) {
        case MediaInfo.GTTMODE_FULL:
            return TelecomManager.TTY_MODE_FULL;
        case MediaInfo.GTTMODE_HCO:
            return TelecomManager.TTY_MODE_HCO;
        case MediaInfo.GTTMODE_VCO:
            return TelecomManager.TTY_MODE_VCO;
        default:
            return TelecomManager.TTY_MODE_OFF;
        }
    }

    public static int getTtyModeFromTelecomToMediaInfo(int ttyMode) {
        switch (ttyMode) {
        case TelecomManager.TTY_MODE_FULL:
            return MediaInfo.GTTMODE_FULL;
        case TelecomManager.TTY_MODE_HCO:
            return MediaInfo.GTTMODE_HCO;
        case TelecomManager.TTY_MODE_VCO:
            return MediaInfo.GTTMODE_VCO;
        case TelecomManager.TTY_MODE_OFF:
            return MediaInfo.GTTMODE_INACTIVE;
        default:
            return MediaInfo.GTTMODE_INVALID;
        }
    }

    public static int getVideoCallType(ImsStreamMediaProfile profile) {
        // P-OS: CALL_TYPE_VT_NODIR is not used for video state control
        if (profile != null) {
            if (profile.getVideoDirection() == ImsStreamMediaProfile.DIRECTION_RECEIVE) {
                return ImsCallProfile.CALL_TYPE_VT_RX;
            } else if (profile.getVideoDirection() == ImsStreamMediaProfile.DIRECTION_SEND) {
                return ImsCallProfile.CALL_TYPE_VT_TX;
            }
        }

        return ImsCallProfile.CALL_TYPE_VT;
    }

    public static int getVideoDirectionFromCallType(int callType) {
        if ((callType == ImsCallProfile.CALL_TYPE_VIDEO_N_VOICE)
                || (callType == ImsCallProfile.CALL_TYPE_VT)) {
            return ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE;
        } else if (callType == ImsCallProfile.CALL_TYPE_VT_RX) {
            return ImsStreamMediaProfile.DIRECTION_RECEIVE;
        } else if (callType == ImsCallProfile.CALL_TYPE_VT_TX) {
            return ImsStreamMediaProfile.DIRECTION_SEND;
        } else if (callType == ImsCallProfile.CALL_TYPE_VT_NODIR) {
            return ImsStreamMediaProfile.DIRECTION_INACTIVE;
        }

        return ImsStreamMediaProfile.DIRECTION_INVALID;
    }

    public static int getRttModeFromGTTMode(int gttMode) {
        switch (gttMode) {
        case MediaInfo.GTTMODE_FULL:        // FALL-THROUGH
        case MediaInfo.GTTMODE_HCO:         // FALL-THROUGH
        case MediaInfo.GTTMODE_VCO:
            return ImsStreamMediaProfile.RTT_MODE_FULL;
        case MediaInfo.GTTMODE_INACTIVE:    // FALL-THROUGH
        default:
            return ImsStreamMediaProfile.RTT_MODE_DISABLED;
        }
    }

    public static int getGttModeFromRttMode(int rttMode) {
        switch (rttMode) {
        case ImsStreamMediaProfile.RTT_MODE_FULL:       // FALL-THROUGH
            return MediaInfo.GTTMODE_FULL;
        case ImsStreamMediaProfile.RTT_MODE_DISABLED:   // FALL-THROUGH
        default:
            return MediaInfo.GTTMODE_INACTIVE;
        }
    }

    public static int getDirectionFromGTTMode(int gttMode) {
        // VZW requires "sendrecv" direction if TTY is enabled
        switch (gttMode) {
        case MediaInfo.GTTMODE_FULL:
            return MediaInfo.DIRECTION_SEND_RECEIVE;
        case MediaInfo.GTTMODE_HCO:
            // MediaInfo.DIRECTION_RECEIVE;
            return MediaInfo.DIRECTION_SEND_RECEIVE;
        case MediaInfo.GTTMODE_VCO:
            // MediaInfo.DIRECTION_SEND;
            return MediaInfo.DIRECTION_SEND_RECEIVE;
        case MediaInfo.GTTMODE_INACTIVE: // FALL-THROUGH
        default:
            return MediaInfo.DIRECTION_INVALID;
        }
    }

    public static boolean isAudioEvsCategory(int audioQuality) {
        return (audioQuality == ImsStreamMediaProfile.AUDIO_QUALITY_EVS_NB)
                || (audioQuality == ImsStreamMediaProfile.AUDIO_QUALITY_EVS_WB)
                || (audioQuality == ImsStreamMediaProfile.AUDIO_QUALITY_EVS_SWB)
                || (audioQuality == ImsStreamMediaProfile.AUDIO_QUALITY_EVS_FB);
    }

    public static boolean isAudioHDQuality(int audioQuality) {
        return (audioQuality == ImsStreamMediaProfile.AUDIO_QUALITY_AMR_WB)
                || (audioQuality == ImsStreamMediaProfile.AUDIO_QUALITY_EVS_WB);
    }

    public static boolean isAudioUHDQuality(int audioQuality) {
        return (audioQuality == ImsStreamMediaProfile.AUDIO_QUALITY_EVS_SWB)
                || (audioQuality == ImsStreamMediaProfile.AUDIO_QUALITY_EVS_FB);
    }

    public static boolean isDefaultMediaProfile(ImsStreamMediaProfile profile) {
        // AOSP: AMR_WB, QCT: NONE
        return (profile == null) ? true :
                (((profile.getAudioQuality() == ImsStreamMediaProfile.AUDIO_QUALITY_AMR_WB)
                        || (profile.getAudioQuality() == ImsStreamMediaProfile.AUDIO_QUALITY_NONE))
                    && (profile.getAudioDirection() == ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE)
                    && (profile.getVideoQuality() == ImsStreamMediaProfile.VIDEO_QUALITY_NONE)
                    && (profile.getVideoDirection() == ImsStreamMediaProfile.DIRECTION_INVALID));
    }

    public static boolean isDtmfEvent(char c) {
        return DTMF_EVENT.contains(String.valueOf(c));
    }

    public static boolean isVideoProfileChanged(ImsStreamMediaProfile profile,
            MediaInfo mi) {
        if ((profile == null) || (mi == null)) {
            return false;
        }

        int videoQuality = getVideoQualityFromMediaInfoForMediaProfile(mi.videoQuality);
        int videoDirection = getDirectionFromMediaInfoForMediaProfile(mi.videoDir);

        return (videoQuality != profile.getVideoQuality())
                || (videoDirection != profile.getVideoDirection());
    }

    public static void setGttInfo(MediaInfo mi, int tDirection, int gttMode) {
        if (mi != null) {
            mi.textDir = tDirection;
            mi.gttMode = gttMode;
        }
    }

    public static void setRttInfo(MediaInfo mi, int tDirection, boolean isRttOn) {
        if (mi != null) {
            mi.textDir = tDirection;
            mi.gttMode = isRttOn ? MediaInfo.GTTMODE_FULL : MediaInfo.GTTMODE_INVALID;
        }
    }

    public static void setMediaProfile(ImsStreamMediaProfile profile,
            int audioQuality, int audioDirection, int videoQuality, int videoDirection) {
        if (profile != null) {
            profile.copyFrom(new ImsStreamMediaProfile(audioQuality, audioDirection, videoQuality,
                    videoDirection, ImsStreamMediaProfile.RTT_MODE_DISABLED));
        }
    }

    public static void setMediaProfile(ImsStreamMediaProfile profile,
            int audioQuality, int audioDirection, int videoQuality, int videoDirection,
            int rttMode) {
        if (profile != null) {
            profile.copyFrom(new ImsStreamMediaProfile(audioQuality, audioDirection, videoQuality,
                    videoDirection, rttMode));
        }
    }

    public static ImsStreamMediaProfile getMediaProfileFromMediaInfo(MediaInfo mi) {
        int audioQuality = ImsStreamMediaProfile.AUDIO_QUALITY_NONE;
        int audioDirection = ImsStreamMediaProfile.DIRECTION_INVALID;
        int videoQuality = ImsStreamMediaProfile.VIDEO_QUALITY_NONE;
        int videoDirection = ImsStreamMediaProfile.DIRECTION_INVALID;
        int rttMode = ImsStreamMediaProfile.RTT_MODE_DISABLED;

        if (mi != null) {
            audioQuality = getAudioQualityFromMediaInfoForMediaProfile(mi.audioQuality);
            audioDirection = getDirectionFromMediaInfoForMediaProfile(mi.audioDir);
            videoQuality = getVideoQualityFromMediaInfoForMediaProfile(mi.videoQuality);
            videoDirection = getDirectionFromMediaInfoForMediaProfile(mi.videoDir);
            rttMode = getRttModeFromGTTMode(mi.gttMode);

            if (videoQuality == ImsStreamMediaProfile.VIDEO_QUALITY_NONE) {
                videoDirection = ImsStreamMediaProfile.DIRECTION_INVALID;
            }
        }

        return new ImsStreamMediaProfile(audioQuality, audioDirection, videoQuality,
                    videoDirection, rttMode);
    }

    public static String toString(VideoProfile profile) {
        if (profile == null) {
            return "[ VideoProfile: null ]";
        }

        StringBuilder sb = new StringBuilder();

        sb.append("[ VideoProfile: state=");

        int videoState = profile.getVideoState();

        if (videoState == VideoProfile.STATE_AUDIO_ONLY) {
            sb.append("AudioOnly");
        } else {
            if (VideoProfile.isBidirectional(videoState)) {
                sb.append("Bidirectional");
            } else {
                if (VideoProfile.isTransmissionEnabled(videoState)) {
                    sb.append("TxEnabled");
                } else if (VideoProfile.isReceptionEnabled(videoState)) {
                    sb.append("RxEnabled");
                }
            }

            if (VideoProfile.isPaused(videoState)) {
                if (VideoProfile.isVideo(videoState)) {
                    sb.append("|");
                }

                sb.append("Paused");
            }
        }

        sb.append("(");
        sb.append(videoState);
        sb.append(")");

        sb.append(", quality=");
        sb.append(profile.getQuality());
        sb.append(" ]");

        return sb.toString();
    }

    public static void updateCallProfileFromMediaInfo(ICallContext context,
            ImsCallProfile profile, final MediaInfo mi) {
        int audioQuality = getAudioQualityFromMediaInfoForMediaProfile(mi.audioQuality);
        int videoQuality = getVideoQualityFromMediaInfoForMediaProfile(mi.videoQuality);
        int rttMode = getRttModeFromGTTMode(mi.gttMode);
        int audioDirection = getDirectionFromMediaInfoForMediaProfile(mi.audioDir);
        int videoDirection = getDirectionFromMediaInfoForMediaProfile(mi.videoDir);

        profile.getMediaProfile().copyFrom(new ImsStreamMediaProfile(audioQuality, audioDirection,
                videoQuality, videoDirection, rttMode));

        updateMediaProfileFromMediaInfoForAudioCodecAttributes(profile.getMediaProfile(), mi);
        updateCallProfileFromMediaInfoForRtt(profile, mi);
    }

    /**
     * Updates the {@link ImsCallProfile} with the audio codec attributes from a
     * {@link MediaInfo} object.
     *
     * @param profile The ImsCallProfile to update.
     * @param mi The MediaInfo object containing the new audio codec attributes.
     */
    public static void updateMediaProfileFromMediaInfoForAudioCodecAttributes(
            ImsStreamMediaProfile mediaProfile, final MediaInfo mi) {
        AudioCodecAttributes fromAttributes = mi.getAudioCodecAttributes();

        if (fromAttributes == null) {
            return;
        }

        Range<Float> bitrateRange = new Range<>(fromAttributes.mBitrateStartKbps,
                fromAttributes.mBitrateEndKbps);
        Range<Float> bandwidthRange = new Range<>(fromAttributes.mBandwidthStartKhz,
                fromAttributes.mBandwidthEndKhz);

        android.telephony.ims.AudioCodecAttributes toAttributes =
                new android.telephony.ims.AudioCodecAttributes(fromAttributes.mBitrateKbps,
                bitrateRange, fromAttributes.mBandwidthKhz, bandwidthRange);
        mediaProfile.setAudioCodecAttributes(toAttributes);
    }

    public static void updateCallProfileFromMediaInfoForRtt(ImsCallProfile profile,
            final MediaInfo mi) {
        profile.getMediaProfile().setRttMode(getRttModeFromGTTMode(mi.gttMode));
        profile.setCallExtraInt(MEDIA_TEXT_DIRECTION, mi.textDir);
    }

    /**
     * Updates the audio codec attributes of an {@link ImsCallProfile} from another
     * {@link ImsCallProfile}.
     *
     * This method copies the {@link android.telephony.ims.AudioCodecAttributes} from the
     * {@code fromProfile} to the {@code toProfile}. A new attributes object is created for the
     * {@code toProfile} to avoid sharing instances.
     *
     * If the attributes in {@code fromProfile} are null, this method does nothing.
     *
     * @param toProfile The destination {@link ImsCallProfile} to update.
     * @param fromProfile The source {@link ImsCallProfile} containing the attributes to copy.
     */
    public static void updateCallProfileForAudioCodecAttributes(
            ImsCallProfile toProfile, ImsCallProfile fromProfile) {
        android.telephony.ims.AudioCodecAttributes fromAttributes =
                fromProfile.getMediaProfile().getAudioCodecAttributes();

        if (fromAttributes ==  null) {
            return;
        }

        Range<Float> bitrateRange = fromAttributes.getBitrateRangeKbps();
        Range<Float> bandwidthRange = fromAttributes.getBandwidthRangeKhz();

        android.telephony.ims.AudioCodecAttributes toAttributes =
                new android.telephony.ims.AudioCodecAttributes(
                fromAttributes.getBitrateKbps(),
                        new Range<>(bitrateRange.getLower(), bitrateRange.getUpper()),
                fromAttributes.getBandwidthKhz(),
                        new Range<>(bandwidthRange.getLower(), bandwidthRange.getUpper()));

        toProfile.getMediaProfile().setAudioCodecAttributes(toAttributes);
    }

    private static MediaInfo createMediaInfoForVideoCallOnCallAccept(
            ImsCallProfile profile, int callType, int audioCapabilities, int videoCapabilities) {
        int audioQuality = getNegotiatedAudioQuality(profile, audioCapabilities);
        int videoQuality = VideoCallUtils.getNegotiatedVideoQuality(profile, videoCapabilities);
        int audioDirection = getMediaDirectionFromProfile(profile, true);
        int videoDirection = getMediaDirectionFromProfile(profile, false);

        if (FEATURE_OVERRIDE_VIDEO_DIRECTION_FROM_CALL_TYPE) {
            int videoDir = getVideoDirectionFromCallType(callType);

            if ((videoDir != ImsStreamMediaProfile.DIRECTION_INVALID)
                    && (callType != ImsCallProfile.CALL_TYPE_VIDEO_N_VOICE)) {
                // Override the video direction based on the call type
                videoDirection = getDirectionFromMediaProfileForMediaInfo(videoDir);
            }
        }

        return new MediaInfo(audioQuality, videoQuality, audioDirection, videoDirection,
                MediaInfo.DIRECTION_INVALID, MediaInfo.GTTMODE_INVALID);
    }

    private static MediaInfo createMediaInfoForVoiceCallOnCallAccept(ImsCallProfile profile,
            int audioCapabilities) {
        int audioQuality = getNegotiatedAudioQuality(profile, audioCapabilities);
        int audioDirection = getMediaDirectionFromProfile(profile, true);

        return new MediaInfo(audioQuality, MediaInfo.VIDEO_QUALITY_NONE,
                audioDirection, MediaInfo.DIRECTION_INVALID,
                profile.getCallExtraInt(
                        ImsCallMediaUtils.MEDIA_TEXT_DIRECTION, MediaInfo.DIRECTION_INVALID),
                getGttModeFromRttMode(profile.getMediaProfile().getRttMode()));
    }

    private static int getMediaDirectionFromProfile(ImsCallProfile profile, boolean isAudio) {
        return switch (isAudio
                ? profile.getMediaProfile().getAudioDirection()
                : profile.getMediaProfile().getVideoDirection()) {
            case ImsStreamMediaProfile.DIRECTION_RECEIVE -> MediaInfo.DIRECTION_RECEIVE;
            case ImsStreamMediaProfile.DIRECTION_SEND -> MediaInfo.DIRECTION_SEND;
            case ImsStreamMediaProfile.DIRECTION_INACTIVE -> MediaInfo.DIRECTION_INACTIVE;
            case ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE -> MediaInfo.DIRECTION_SEND_RECEIVE;
            default -> MediaInfo.DIRECTION_INVALID;
        };
    }

    private static int getNegotiatedAudioQuality(ImsCallProfile profile,
            int audioCapabilities) {
        int audioQuality = profile.getMediaProfile().getAudioQuality();

        // Checks the current audio quality only.
        if (audioQuality == ImsStreamMediaProfile.AUDIO_QUALITY_G711U) {
            return MediaInfo.AUDIO_QUALITY_G711_PCMU;
        } else if (audioQuality == ImsStreamMediaProfile.AUDIO_QUALITY_G711A) {
            return MediaInfo.AUDIO_QUALITY_G711_PCMA;
        }

        // Checks the current audio quality and the capabilities.
        if ((audioQuality == ImsStreamMediaProfile.AUDIO_QUALITY_AMR)
                || (audioCapabilities == ImsStreamMediaProfile.AUDIO_QUALITY_AMR)) {
            return MediaInfo.AUDIO_QUALITY_AMR_NB;
        } else if ((audioQuality == ImsStreamMediaProfile.AUDIO_QUALITY_AMR_WB)
                || (audioCapabilities == ImsStreamMediaProfile.AUDIO_QUALITY_AMR_WB)) {
            return MediaInfo.AUDIO_QUALITY_AMR_WB;
        } else {
            int miAQ = getAudioQualityFromMediaProfileForMediaInfo(audioQuality);

            if (miAQ != MediaInfo.AUDIO_QUALITY_NONE) {
                return miAQ;
            } else if (audioCapabilities == ImsStreamMediaProfile.AUDIO_QUALITY_EVS_SWB) {
                return MediaInfo.AUDIO_QUALITY_EVS_SWB;
            }
        }

        return MediaInfo.AUDIO_QUALITY_AMR_WB;
    }

    static {
        /**
         * Media profile and media info.
         */
        sAudioQualityForMediaProfileAndMediaInfo = new LinkedHashMap<Integer, Integer>();
        sAudioQualityForMediaProfileAndMediaInfo.put(
                ImsStreamMediaProfile.AUDIO_QUALITY_AMR,
                MediaInfo.AUDIO_QUALITY_AMR_NB);
        sAudioQualityForMediaProfileAndMediaInfo.put(
                ImsStreamMediaProfile.AUDIO_QUALITY_AMR_WB,
                MediaInfo.AUDIO_QUALITY_AMR_WB);
        sAudioQualityForMediaProfileAndMediaInfo.put(
                ImsStreamMediaProfile.AUDIO_QUALITY_G711U,
                MediaInfo.AUDIO_QUALITY_G711_PCMU);
        sAudioQualityForMediaProfileAndMediaInfo.put(
                ImsStreamMediaProfile.AUDIO_QUALITY_G711A,
                MediaInfo.AUDIO_QUALITY_G711_PCMA);
        sAudioQualityForMediaProfileAndMediaInfo.put(
                ImsStreamMediaProfile.AUDIO_QUALITY_EVS_NB,
                MediaInfo.AUDIO_QUALITY_EVS_NB);
        sAudioQualityForMediaProfileAndMediaInfo.put(
                ImsStreamMediaProfile.AUDIO_QUALITY_EVS_WB,
                MediaInfo.AUDIO_QUALITY_EVS_WB);
        sAudioQualityForMediaProfileAndMediaInfo.put(
                ImsStreamMediaProfile.AUDIO_QUALITY_EVS_SWB,
                MediaInfo.AUDIO_QUALITY_EVS_SWB);
        sAudioQualityForMediaProfileAndMediaInfo.put(
                ImsStreamMediaProfile.AUDIO_QUALITY_EVS_FB,
                MediaInfo.AUDIO_QUALITY_EVS_FB);
    }
}
