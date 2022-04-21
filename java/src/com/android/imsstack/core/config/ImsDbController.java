package com.android.imsstack.core.config;

import android.net.Uri;

import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.DBUtils;
import com.android.imsstack.util.MSimUtils;

import java.util.Locale;

public final class ImsDbController {

    public static boolean isAdminFeatureEnabled(int adminFeatures, int feature) {
        return (adminFeatures & feature) == feature;
    }

    public static boolean isDebugEnabled(int adminFeatures) {
        return isAdminFeatureEnabled(adminFeatures,
                ProviderInterface.Subscriber.AdminFeatures.DEBUG);
    }

    public static boolean isDMEnabled(int adminFeatures) {
        return isAdminFeatureEnabled(adminFeatures,
                ProviderInterface.Subscriber.AdminFeatures.DM);
    }

    public static boolean isImsEnabled(int adminFeatures) {
        return isAdminFeatureEnabled(adminFeatures,
                ProviderInterface.Subscriber.AdminFeatures.IMS);
    }

    public static boolean isIsimEnabled(int adminFeatures) {
        return isAdminFeatureEnabled(adminFeatures,
                ProviderInterface.Subscriber.AdminFeatures.ISIM);
    }

    public static boolean isTestModeEnabled(int adminFeatures) {
        return isAdminFeatureEnabled(adminFeatures,
                ProviderInterface.Subscriber.AdminFeatures.TESTMODE);
    }

    public static boolean isTestModeForGcfEnabled(int adminFeatures) {
        return isAdminFeatureEnabled(adminFeatures,
                ProviderInterface.Subscriber.AdminFeatures.TESTMODE_GCF);
    }

    public static boolean isUsimEnabled(int adminFeatures) {
        return isAdminFeatureEnabled(adminFeatures,
                ProviderInterface.Subscriber.AdminFeatures.USIM);
    }

    public static String selectForSlot(int slotId) {
        if (slotId < 0) {
            slotId = MSimUtils.DEFAULT_SLOT_ID;
        }

        return String.format(Locale.US, "%s='%d'", ProviderInterface.ID, slotId);
    }

    public static class Subscriber {
        public static final int TABLE_DEFAULT = 0;
        public static final int TABLE_FAKE = 2;

        public static Uri getUri(int tableId) {
            Uri uri = null;

            if (tableId == TABLE_FAKE) {
                uri = Uri.parse("content://" + ProviderInterface.AUTHORITY
                        + "/" + ProviderInterface.Subscriber.TABLE_NAME_FAKE);
            } else {
                uri = ProviderInterface.Subscriber.CONTENT_URI;
            }

            return uri;
        }

        public static int getAdminFeatures() {
            return getAdminFeatures(MSimUtils.DEFAULT_SLOT_ID);
        }

        public static int getAdminFeatures(int slotId) {
            return getAdminFeatures(getUri(TABLE_DEFAULT), slotId);
        }

        public static String getAdminPcscf(int slotId) {
            return getAdminPcscf(getUri(TABLE_DEFAULT), slotId);
        }

        public static int getAdminFeatures(int tableId, int slotId) {
            return getAdminFeatures(getUri(tableId), slotId);
        }

        public static int getAdminFeatures(Uri uri, int slotId) {
            return DBUtils.CP.getInt(AppContext.get().getContentResolver(),
                    uri, selectForSlot(slotId), ProviderInterface.Subscriber.FEATURES, 0);
        }

        public static String getAdminPcscf(Uri uri, int slotId) {
            return DBUtils.CP.getString(AppContext.get().getContentResolver(),
                    uri, selectForSlot(slotId), ProviderInterface.Subscriber.PCSCF, "");
        }

        public static String getHomeDomainName(int slotId) {
            return DBUtils.CP.getString(AppContext.get().getContentResolver(),
                    getUri(TABLE_DEFAULT), selectForSlot(slotId),
                    ProviderInterface.Subscriber.HOME_DOMAIN_NAME, "");
        }

        public static String getImpi(int slotId) {
            return DBUtils.CP.getString(AppContext.get().getContentResolver(),
                    getUri(TABLE_DEFAULT), selectForSlot(slotId),
                    ProviderInterface.Subscriber.IMPI, "");
        }

        public static String getImpu(int slotId) {
            return DBUtils.CP.getString(AppContext.get().getContentResolver(),
                    getUri(TABLE_DEFAULT), selectForSlot(slotId),
                    ProviderInterface.Subscriber.IMPU_0, "");
        }

        public static String getPcscfAddress(int slotId) {
            return DBUtils.CP.getString(AppContext.get().getContentResolver(),
                    getUri(TABLE_DEFAULT), selectForSlot(slotId),
                    ProviderInterface.Subscriber.PCSCF_ADDRESS_0, "");
        }

        public static int getPcscfPort(int slotId) {
            return DBUtils.CP.getInt(AppContext.get().getContentResolver(),
                    getUri(TABLE_DEFAULT), selectForSlot(slotId),
                    ProviderInterface.Subscriber.PCSCF_PORT_0, -1);
        }

        public static boolean setAdminFeatures(int enabledFeatures, int disabledFeatures) {
            return setAdminFeatures(MSimUtils.DEFAULT_SLOT_ID, enabledFeatures, disabledFeatures);
        }

        public static boolean setAdminFeatures(int slotId,
                int enabledFeatures, int disabledFeatures) {
            return setAdminFeatures(getUri(TABLE_DEFAULT),
                    slotId, enabledFeatures, disabledFeatures);
        }

        public static boolean setAdminFeatures(int tableId, int slotId,
                int enabledFeatures, int disabledFeatures) {
            return setAdminFeatures(getUri(tableId),
                    slotId, enabledFeatures, disabledFeatures);
        }

        public static boolean setAdminFeatures(Uri uri, int slotId,
                int enabledFeatures, int disabledFeatures) {
            int adminFeatures = getAdminFeatures(uri, slotId);
            int oldAdminFeatures = adminFeatures;

            if (enabledFeatures > 0) {
                adminFeatures |= enabledFeatures;
            }

            if (disabledFeatures > 0) {
                adminFeatures &= (~disabledFeatures);
            }

            if (oldAdminFeatures == adminFeatures) {
                // No need to update this item
                return true;
            }

            String value = String.format("0x%08X", adminFeatures);

            return DBUtils.CP.putString(AppContext.get().getContentResolver(),
                    uri, ProviderInterface.Subscriber.FEATURES, value, selectForSlot(slotId));
        }
    }
}
