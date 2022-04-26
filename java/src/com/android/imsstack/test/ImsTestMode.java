package com.android.imsstack.test;

import android.content.Context;
import android.net.Uri;

import com.android.imsstack.core.config.ImsDbController;
import com.android.imsstack.core.config.ProviderInterface;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.DBUtils;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.MSimUtils;

import java.util.HashMap;
import java.util.Locale;
import java.util.Map;

/**
 * IMS Test Mode
 */
public final class ImsTestMode {
    public static final int CONFIG_NONE = 0;
    /**
     * Bitmask: Framework level test cases (0x01 ~ 0x80)
     */
    public static final int CONFIG_IMS_OFF = 0x00000001;
    public static final int CONFIG_DEBUG = 0x00000002;
    public static final int CONFIG_GENERIC_TEST_MODE = 0x00000004;
    public static final int CONFIG_CALL_OVER_WIFI = 0x00000008;
    /**
     * Bitmask: Call level test cases (0x0100 ~ 0x8000)
     */
    public static final int CONFIG_LOCAL_HOLD_TONE = 0x00000100;

    /**
     * Bitmask: for getExtraTestmask()
     */
    public static final int TEST_MASK_ROAMING_CONDITION = 0x0002;
    public static final int TEST_MASK_IMS_STATUS_TO_UICC_OFF = 0x0008;

    private static final Uri SETTING_CONTENT_URI
            = Uri.parse("content://" + ProviderInterface.AUTHORITY + "/gims_setting");
    private static final String SETTING_TEST_MASK = "setting_test_mask";

    private static ImsTestMode sImsTestMode = new ImsTestMode();

    private static Map<Integer, IImsTestMode> mTM
            = new HashMap<Integer, IImsTestMode>(MSimUtils.getMaxSimSlot());


    public static ImsTestMode getInstance() {
        return sImsTestMode;
    }

    public ImsTestMode() {
    }

    public synchronized void init(Context context, int slotId) {
        ImsLog.d("SLOT = " + slotId);

        IImsTestMode itm = mTM.get(slotId);
        if (itm != null) {
            return;
        }

        TestMode tm = new TestMode(slotId, context);
        tm.init();
        mTM.put(slotId, tm);
    }

    public synchronized void cleanUp(int slotId) {
        ImsLog.d("SLOT = " + slotId);

        TestMode tm = (TestMode)mTM.get(slotId);
        if (tm == null) {
            return;
        }

        tm.cleanup();
        mTM.remove(slotId);
    }

    public synchronized IImsTestMode getTestMode(int slotId) {
        IImsTestMode itm = mTM.get(slotId);
        if (itm != null) {
            return itm;
        }

        TestMode tm = new TestMode(slotId, AppContext.get());
        tm.init();
        mTM.put(slotId, tm);
        return tm;
    }

    private static String getConfigValue(Context context, Uri contentUri, String selection,
            String columnName) {
        return DBUtils.CP.getString(context.getContentResolver(),
                contentUri, selection, columnName, null);
    }

    private class TestMode implements IImsTestMode {
        private int mSlotId = 0;
        private int mTestmodes = CONFIG_NONE;
        private int mExtraTestmask = 0;
        private Context mContext;

        public TestMode(int slotId, Context context) {
            mSlotId = slotId;
            mContext = context;
        }

        public void init() {
            initFromConfig();

            if (isGenericTestMode() && isDebugEnabled()) {
                enableTestmode(CONFIG_LOCAL_HOLD_TONE);
            }

            ImsLog.i("[ ImsTestMode("+ mSlotId + ") : testmodes="
                    + String.format("%08x", mTestmodes)
                    + ", extraTestmask=" + String.format("%08x", mExtraTestmask) + " ]");
        }

        public void cleanup() {
        }

        @Override
        public int getExtraTestmask() {
            return mExtraTestmask;
        }

        @Override
        public boolean isCallOverWifiEnabled() {
            return isConfigEnabled(CONFIG_CALL_OVER_WIFI);
        }

        @Override
        public boolean isDebugEnabled() {
            return isConfigEnabled(CONFIG_DEBUG);
        }

        @Override
        public boolean isDebuggable() {
            return isDebugEnabled() || isGenericTestMode();
        }

        @Override
        public boolean isImsOff() {
            return isConfigEnabled(CONFIG_IMS_OFF);
        }

        @Override
        public boolean isLocalHoldToneEnabled() {
            return isConfigEnabled(CONFIG_LOCAL_HOLD_TONE);
        }

        @Override
        public boolean isGenericTestMode() {
            return isConfigEnabled(CONFIG_GENERIC_TEST_MODE);
        }

        private void initFromConfig() {
            if (mContext == null) {
                return;
            }

            int adminFeatures = ImsDbController.Subscriber.getAdminFeatures(mSlotId);

            ImsLog.i("ImsTestMode(" + mSlotId + ") : adminFeatures=" +
                String.format("%08X", adminFeatures));

            if (!ImsDbController.isImsEnabled(adminFeatures)) {
                enableTestmode(CONFIG_IMS_OFF);
            }

            if (ImsDbController.isTestModeEnabled(adminFeatures)) {
                enableTestmode(CONFIG_GENERIC_TEST_MODE);
            }

            if (ImsDbController.isDebugEnabled(adminFeatures)) {
                enableTestmode(CONFIG_DEBUG);
            }

            String selection = String.format(Locale.US, "%s='%d'", ProviderInterface.ID, mSlotId);
            String config = getConfigValue(mContext,
                    ProviderInterface.AoSConnection.CONTENT_URI,
                    selection,
                    ProviderInterface.AoSConnection.PROFILE_NAME_0);

            // Local hold tone generation if call is held by the remote party
            if ((config != null) && config.equalsIgnoreCase("wifi")) {
                enableTestmode(CONFIG_CALL_OVER_WIFI);
            }

            // Carrier specific test mask
            config = getConfigValue(mContext, SETTING_CONTENT_URI, selection, SETTING_TEST_MASK);

            if (config != null) {
                int testmask = 0;

                try {
                    testmask = Integer.parseInt(config);
                } catch (Throwable t) {
                    t.printStackTrace();
                }

                setExtraTestmask(testmask);
            }
        }

        private void enableTestmode(int enabledTestmodes) {
            mTestmodes |= enabledTestmodes;
        }

        private void disableTestmode(int disabledTestmodes) {
            mTestmodes &= (~disabledTestmodes);
        }

        private void setExtraTestmask(int testmask) {
            mExtraTestmask = testmask;
        }

        private boolean isConfigEnabled(int config) {
            return (mTestmodes & config) == config;
        }
    }
}
