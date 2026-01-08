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
package com.android.imsstack.imsservice.mmtel.videocall.base;

import android.content.ContentResolver;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsStreamMediaProfile;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.enabler.mtc.IUMtcMedia;
import com.android.imsstack.enabler.mtc.MediaInfo;
import com.android.imsstack.imsservice.mmtel.util.VideoDimension;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.ImsUtils;

import java.io.File;
import java.io.FileDescriptor;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Set;

/**
 * This class provides a table for video dimension.
 */
public class VideoCallUtils {
    /**
     * Video codec information.
     */
    public static final int VIDEO_QUALITY_CIF_LANDSCAPE = (1 << 11);
    public static final int VIDEO_QUALITY_CIF_PORTRAIT = (1 << 12);
    public static final int VIDEO_QUALITY_QCIF_PORTRAIT = (1 << 13);
    public static final int VIDEO_QUALITY_SQCIF_LANDSCAPE = (1 << 14);
    public static final int VIDEO_QUALITY_SQCIF_PORTRAIT = (1 << 15);
    public static final int VIDEO_QUALITY_SIF_LANDSCAPE = (1 << 16);
    public static final int VIDEO_QUALITY_SIF_PORTRAIT = (1 << 17);
    public static final int VIDEO_QUALITY_HD_LANDSCAPE = (1 << 18);
    public static final int VIDEO_QUALITY_HD_PORTRAIT = (1 << 19);

    public static final int ORIENTATION_PORTRAIT = 1;
    public static final int ORIENTATION_LANDSCAPE = 2;

    private static final String PAUSE_IMAGE_FILE = ImsUtils.STORAGE_ROOT_DIR + "/pause_img.jpg";

    private static final LinkedHashMap<Integer, Integer> sVideoQualityForMediaProfileAndMediaInfo;
    /**
     * Table for video quality and video dimensions.
     */
    private static final LinkedHashMap<Integer, VideoDimension> sVideoDimensions;
    /**
     * Table for orientation-reversed video quality.
     */
    private static final LinkedHashMap<Integer, Integer> sOrientationReversedVideoQualities;

    /**
     * DEBUGGING PURPOSE
     */
    public static void displayVideoQuality() {
        Set<Map.Entry<Integer, Integer>> vqSet
                = sVideoQualityForMediaProfileAndMediaInfo.entrySet();

        if (vqSet != null) {
            for (Map.Entry<Integer, Integer> entry : vqSet) {
                ImsLog.w("VideoQuality :: key=" + entry.getKey().intValue()
                        + ", value=" + entry.getValue().intValue());
            }
        }
    }

    public static String getPauseImageFile(Uri uri, int videoQuality) {
        VideoDimension vd = getVideoDimension(videoQuality);

        if (vd == null) {
            return null;
        }

        // For backward compatibility
        if (uri.getScheme() == null) {
            // Consider it as a file path
            String s = uri.toString();

            if ((s != null) && s.startsWith("/")) {
                uri = Uri.parse("file://" + s);
            }
        }

        Bitmap bitmap = null;
        ContentResolver cr = AppContext.getInstance().getContentResolver();

        try {
            ParcelFileDescriptor pfd = cr.openFileDescriptor(uri, "r");

            if (pfd == null) {
                return null;
            }

            bitmap = decodeImage(pfd.getFileDescriptor(), vd);

            pfd.close();
        } catch (FileNotFoundException e) {
            ImsLog.e(e.toString(), e);
        } catch (IOException e) {
            ImsLog.e(e.toString(), e);
        }

        if (bitmap == null) {
            return null;
        }

        File file = new File(PAUSE_IMAGE_FILE);

        try {
            if (file.exists()) {
                file.delete();
            }

            file.createNewFile();

            FileOutputStream out = new FileOutputStream(file);

            bitmap.compress(Bitmap.CompressFormat.JPEG, 100, out);

            out.close();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            return null;
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        } finally {
            bitmap.recycle();
        }

        return file.getAbsolutePath();
    }

    public static int getOrientationFromMtcMediaSession(int orientation) {
        if (orientation == IUMtcMedia.Notify.NOTIFY_ORIENTATION_PORTRAIT) {
            return ORIENTATION_PORTRAIT;
        } else if (orientation == IUMtcMedia.Notify.NOTIFY_ORIENTATION_LANDSCAPE) {
            return ORIENTATION_LANDSCAPE;
        } else {
            // Unknown: not-reachable
            return 0;
        }
    }

    public static VideoDimension getReversedVideoDimension(int videoQuality) {
        Integer vq = sOrientationReversedVideoQualities.get(Integer.valueOf(videoQuality));
        return (vq != null) ? sVideoDimensions.get(vq) : null;
    }

    public static VideoDimension getVideoDimension(int videoQuality) {
        return sVideoDimensions.get(Integer.valueOf(videoQuality));
    }

    public static int getNegotiatedVideoQuality(ImsCallProfile profile,
            int videoCapabilities) {
        int videoQuality = profile.getMediaProfile().getVideoQuality();

        if (videoQuality == ImsStreamMediaProfile.VIDEO_QUALITY_NONE) {
            return MediaInfo.VIDEO_QUALITY_NONE;
        } else if ((videoCapabilities & ImsStreamMediaProfile.VIDEO_QUALITY_QCIF) != 0) {
            return MediaInfo.VIDEO_QUALITY_QCIF;
        }

        Set<Map.Entry<Integer, Integer>> vqSet
                = sVideoQualityForMediaProfileAndMediaInfo.entrySet();

        if (vqSet != null) {
            for (Map.Entry<Integer, Integer> entry : vqSet) {
                Integer vqForMP = entry.getKey();

                if ((vqForMP != null) && ((vqForMP.intValue() & videoQuality) != 0)) {
                    Integer vqForMI = entry.getValue();
                    return (vqForMI != null) ?
                            vqForMI.intValue() : MediaInfo.VIDEO_QUALITY_NONE;
                }
            }
        }

        return MediaInfo.VIDEO_QUALITY_NONE;
    }

    public static int getVideoQualityFromMediaInfoForMediaProfile(int videoQuality) {
        Set<Map.Entry<Integer, Integer>> vqSet
                = sVideoQualityForMediaProfileAndMediaInfo.entrySet();

        if (vqSet != null) {
            for (Map.Entry<Integer, Integer> entry : vqSet) {
                Integer vqForMI = entry.getValue();

                if ((vqForMI != null) && (vqForMI.intValue() == videoQuality)) {
                    Integer vqForMP = entry.getKey();
                    return (vqForMP != null) ?
                            vqForMP.intValue() : ImsStreamMediaProfile.VIDEO_QUALITY_NONE;
                }
            }
        }

        return ImsStreamMediaProfile.VIDEO_QUALITY_NONE;
    }

    public static int getVideoQualityFromMediaProfileForMediaInfo(int videoQuality) {
        Integer vq = sVideoQualityForMediaProfileAndMediaInfo.get(videoQuality);
        return (vq != null) ? vq.intValue() : MediaInfo.VIDEO_QUALITY_NONE;
    }

    public static boolean isVideoPortrait(int videoQuality) {
        switch (videoQuality) {
        case MediaInfo.VIDEO_QUALITY_QVGA_PR: // FALL-THROUGH
        case MediaInfo.VIDEO_QUALITY_VGA_PR: // FALL-THROUGH
        case MediaInfo.VIDEO_QUALITY_CIF_PR: // FALL-THROUGH
        case MediaInfo.VIDEO_QUALITY_QCIF_PR: // FALL-THROUGH
        case MediaInfo.VIDEO_QUALITY_SQCIF_PR: // FALL-THROUGH
        case MediaInfo.VIDEO_QUALITY_SIF_PR: // FALL-THROUGH
        case MediaInfo.VIDEO_QUALITY_HD_PR:
            return true;
        default:
            return false;
        }
    }

    private static int calculateInSampleSize(BitmapFactory.Options options, VideoDimension vd) {
        // Raw height and width of image
        final int height = options.outHeight;
        final int width = options.outWidth;
        int inSampleSize = 1;

        if ((height > vd.getHeight()) || (width > vd.getWidth())) {
            final int halfHeight = height / 2;
            final int halfWidth = width / 2;

            // Calculate the largest inSampleSize value that is a power of 2 and keeps both
            // height and width larger than the requested height and width.
            while (((halfHeight / inSampleSize) > vd.getHeight())
                    && ((halfWidth / inSampleSize) > vd.getWidth())) {
                inSampleSize *= 2;
            }
        }

        return inSampleSize;
    }

    private static Bitmap decodeImage(FileDescriptor fd, VideoDimension vd) {
        if (fd == null) {
            return null;
        }

        BitmapFactory.Options options = new BitmapFactory.Options();

        // Each pixel is stored on 4 bytes
        options.inPreferredConfig = Bitmap.Config.ARGB_8888;

        // Find width/height of this image
        options.inJustDecodeBounds = true;
        BitmapFactory.decodeFileDescriptor(fd, null, options);

        // Calculate inSampleSize
        options.inSampleSize = calculateInSampleSize(options, vd);

        // Decode bitmap with inSampleSize set
        options.inJustDecodeBounds = false;

        Bitmap bitmap = BitmapFactory.decodeFileDescriptor(fd, null, options);
        return scaleImage(bitmap, vd);
    }

    private static Bitmap scaleImage(Bitmap bitmap, VideoDimension vd) {
        if (bitmap == null) {
            return null;
        }

        int width = bitmap.getWidth();
        int height = bitmap.getHeight();

        float scaleWidth = ((float)vd.getWidth()) / width;
        float scaleHeight = ((float)vd.getHeight()) / height;

        Matrix matrix = new Matrix();

        matrix.postScale(scaleWidth, scaleHeight);

        return Bitmap.createBitmap(bitmap, 0, 0, width, height, matrix, false);
    }

    static {
        /**
         * Media profile and media info.
         */
        sVideoQualityForMediaProfileAndMediaInfo = new LinkedHashMap<Integer, Integer>();
        sVideoQualityForMediaProfileAndMediaInfo.put(
                ImsStreamMediaProfile.VIDEO_QUALITY_QCIF,
                MediaInfo.VIDEO_QUALITY_QCIF);
        sVideoQualityForMediaProfileAndMediaInfo.put(
                VIDEO_QUALITY_QCIF_PORTRAIT,
                MediaInfo.VIDEO_QUALITY_QCIF_PR);
        sVideoQualityForMediaProfileAndMediaInfo.put(
                VIDEO_QUALITY_HD_PORTRAIT,
                MediaInfo.VIDEO_QUALITY_HD_PR);
        sVideoQualityForMediaProfileAndMediaInfo.put(
                VIDEO_QUALITY_HD_LANDSCAPE,
                MediaInfo.VIDEO_QUALITY_HD_LS);
        sVideoQualityForMediaProfileAndMediaInfo.put(
                ImsStreamMediaProfile.VIDEO_QUALITY_VGA_PORTRAIT,
                MediaInfo.VIDEO_QUALITY_VGA_PR);
        sVideoQualityForMediaProfileAndMediaInfo.put(
                ImsStreamMediaProfile.VIDEO_QUALITY_VGA_LANDSCAPE,
                MediaInfo.VIDEO_QUALITY_VGA_LS);
        sVideoQualityForMediaProfileAndMediaInfo.put(
                VIDEO_QUALITY_CIF_PORTRAIT,
                MediaInfo.VIDEO_QUALITY_CIF_PR);
        sVideoQualityForMediaProfileAndMediaInfo.put(
                VIDEO_QUALITY_CIF_LANDSCAPE,
                MediaInfo.VIDEO_QUALITY_CIF_LS);
        sVideoQualityForMediaProfileAndMediaInfo.put(
                VIDEO_QUALITY_SIF_PORTRAIT,
                MediaInfo.VIDEO_QUALITY_SIF_PR);
        sVideoQualityForMediaProfileAndMediaInfo.put(
                VIDEO_QUALITY_SIF_LANDSCAPE,
                MediaInfo.VIDEO_QUALITY_SIF_LS);
        sVideoQualityForMediaProfileAndMediaInfo.put(
                ImsStreamMediaProfile.VIDEO_QUALITY_QVGA_PORTRAIT,
                MediaInfo.VIDEO_QUALITY_QVGA_PR);
        sVideoQualityForMediaProfileAndMediaInfo.put(
                ImsStreamMediaProfile.VIDEO_QUALITY_QVGA_LANDSCAPE,
                MediaInfo.VIDEO_QUALITY_QVGA_LS);
        sVideoQualityForMediaProfileAndMediaInfo.put(
                VIDEO_QUALITY_SQCIF_PORTRAIT,
                MediaInfo.VIDEO_QUALITY_SQCIF_PR);
        sVideoQualityForMediaProfileAndMediaInfo.put(
                VIDEO_QUALITY_SQCIF_LANDSCAPE,
                MediaInfo.VIDEO_QUALITY_SQCIF_LS);

        /**
         * Video dimensions
         */
        sVideoDimensions = new LinkedHashMap<Integer, VideoDimension>();
        sVideoDimensions.put(MediaInfo.VIDEO_QUALITY_QCIF, new VideoDimension(176, 144));
        sVideoDimensions.put(MediaInfo.VIDEO_QUALITY_QVGA_LS, new VideoDimension(320, 240));
        sVideoDimensions.put(MediaInfo.VIDEO_QUALITY_QVGA_PR, new VideoDimension(240, 320));
        sVideoDimensions.put(MediaInfo.VIDEO_QUALITY_VGA_LS, new VideoDimension(640, 480));
        sVideoDimensions.put(MediaInfo.VIDEO_QUALITY_VGA_PR, new VideoDimension(480, 640));
        sVideoDimensions.put(MediaInfo.VIDEO_QUALITY_CIF_LS, new VideoDimension(352, 288));
        sVideoDimensions.put(MediaInfo.VIDEO_QUALITY_CIF_PR, new VideoDimension(288, 352));
        sVideoDimensions.put(MediaInfo.VIDEO_QUALITY_QCIF_PR, new VideoDimension(144, 176));
        sVideoDimensions.put(MediaInfo.VIDEO_QUALITY_SQCIF_LS, new VideoDimension(128, 96));
        sVideoDimensions.put(MediaInfo.VIDEO_QUALITY_SQCIF_PR, new VideoDimension(96, 128));
        sVideoDimensions.put(MediaInfo.VIDEO_QUALITY_SIF_LS, new VideoDimension(352, 240));
        sVideoDimensions.put(MediaInfo.VIDEO_QUALITY_SIF_PR, new VideoDimension(240, 352));
        sVideoDimensions.put(MediaInfo.VIDEO_QUALITY_HD_LS, new VideoDimension(1280, 720));
        sVideoDimensions.put(MediaInfo.VIDEO_QUALITY_HD_PR, new VideoDimension(720, 1280));

        /**
         * Orientation-reversed video quailities
         */
        sOrientationReversedVideoQualities = new LinkedHashMap<Integer, Integer>();
        sOrientationReversedVideoQualities.put(
                MediaInfo.VIDEO_QUALITY_QCIF, MediaInfo.VIDEO_QUALITY_QCIF_PR);
        sOrientationReversedVideoQualities.put(
                MediaInfo.VIDEO_QUALITY_QVGA_LS, MediaInfo.VIDEO_QUALITY_QVGA_PR);
        sOrientationReversedVideoQualities.put(
                MediaInfo.VIDEO_QUALITY_QVGA_PR, MediaInfo.VIDEO_QUALITY_QVGA_LS);
        sOrientationReversedVideoQualities.put(
                MediaInfo.VIDEO_QUALITY_VGA_LS, MediaInfo.VIDEO_QUALITY_VGA_PR);
        sOrientationReversedVideoQualities.put(
                MediaInfo.VIDEO_QUALITY_VGA_PR, MediaInfo.VIDEO_QUALITY_VGA_LS);
        sOrientationReversedVideoQualities.put(
                MediaInfo.VIDEO_QUALITY_CIF_LS, MediaInfo.VIDEO_QUALITY_CIF_PR);
        sOrientationReversedVideoQualities.put(
                MediaInfo.VIDEO_QUALITY_CIF_PR, MediaInfo.VIDEO_QUALITY_CIF_LS);
        sOrientationReversedVideoQualities.put(
                MediaInfo.VIDEO_QUALITY_QCIF_PR, MediaInfo.VIDEO_QUALITY_QCIF);
        sOrientationReversedVideoQualities.put(
                MediaInfo.VIDEO_QUALITY_SQCIF_LS, MediaInfo.VIDEO_QUALITY_SQCIF_PR);
        sOrientationReversedVideoQualities.put(
                MediaInfo.VIDEO_QUALITY_SQCIF_PR, MediaInfo.VIDEO_QUALITY_SQCIF_LS);
        sOrientationReversedVideoQualities.put(
                MediaInfo.VIDEO_QUALITY_SIF_LS, MediaInfo.VIDEO_QUALITY_SIF_PR);
        sOrientationReversedVideoQualities.put(
                MediaInfo.VIDEO_QUALITY_SIF_PR, MediaInfo.VIDEO_QUALITY_SIF_LS);
        sOrientationReversedVideoQualities.put(
                MediaInfo.VIDEO_QUALITY_HD_LS, MediaInfo.VIDEO_QUALITY_HD_PR);
        sOrientationReversedVideoQualities.put(
                MediaInfo.VIDEO_QUALITY_HD_PR, MediaInfo.VIDEO_QUALITY_HD_LS);
    }
}
