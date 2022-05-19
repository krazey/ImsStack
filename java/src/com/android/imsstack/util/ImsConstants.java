package com.android.imsstack.util;

/**
 * This class provides constant values for IMS.
 */
public final class ImsConstants {
    public static final boolean DBG = !android.os.Build.TYPE.equals("user");
    public static final String PACKAGE_NAME = "com.android.imsstack";
    /**
     * N : /data/data/com.android.imsstack
     * N-MR1 (Device Encrypted storage) : /data/user_de/0/com.android.imsstack
     */
    public static final String IMS_STORAGE_ROOT_DIR = "/data/user_de/0/com.android.imsstack";
    public static final String PATH_XML = IMS_STORAGE_ROOT_DIR + "/xml";

    public static final boolean PLATFORM_LAMPLITE = false;

    public static final boolean USE_GOOGLE_NATIVE_APPS = true;

    public static final boolean USE_CARRIER_CONFIG = true;
}
