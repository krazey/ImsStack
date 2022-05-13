/*
    Author
    <table>
    date        author                  description
    --------    --------------          ----------
    20160228    hwangoo.park@           Created
    </table>

    Description
*/

package com.android.imsstack.imsservice.mmtel;

import android.os.Bundle;
import android.telephony.ims.ImsCallProfile;

import com.android.imsstack.core.ImsGlobal;
import com.android.imsstack.core.OperatorInfo;
import com.android.imsstack.enabler.mtc.SuppInfo;
import com.android.imsstack.external.ims.ImsCallProfileEx;
import com.android.imsstack.imsservice.mmtel.base.ICallContext;
import com.android.imsstack.util.SODConfig;

/**
 * Extension class for operator specific supplementary information handling.
 */
public final class ImsSuppInfoUtils {
    // Internal usage
    public static final String EXTRA_GEOLOCATION = "geolocation";

    public static void init() {
        /**
         * If there is any information to pass the call framework or call application,
         * it needs to be registered in SuppInfoUtils object.
         */
    }

    public static void addCallExtraForApp(ICallContext context,
            final SuppInfo suppInfo, ImsCallProfile outProfile) {
        // Common

        // Operator specific operations
        if (ImsGlobal.isOperator(context.getSlotId(), "ATT")) {
            ATT.addCallExtraForApp(suppInfo, outProfile);
        }
    }

    public static void addSuppInfoForIms(ICallContext context,
            final ImsCallProfile profile, SuppInfo outSuppInfo) {
        // Common

        // Operator specific operations
        if (ImsGlobal.isOperator(context.getSlotId(), "ATT")) {
            ATT.addSuppInfoForIms(profile, outSuppInfo);
        } else if (ImsGlobal.isOperator(context.getSlotId(), "DCM")) {
            DCM.addSuppInfoForIms(profile, outSuppInfo);
        } else {
            String enablerType = OperatorInfo.getEnablerType(context.getSlotId());

            if (SODConfig.isEnablerTypeGlobal(enablerType)) {
                Global.addSuppInfoForIms(profile, outSuppInfo);
            } else if (SODConfig.isEnablerTypeCanada(enablerType)) {
                Canada.addSuppInfoForIms(profile, outSuppInfo);
            }
        }
    }

    public static String getCallExtraNameForBoolean(ICallContext context, int suppInfo) {
        // no-op
        return null;
    }

    public static String getCallExtraNameForInt(ICallContext context,int suppInfo) {
        // no-op
        return null;
    }

    public static String getCallExtraNameForString(ICallContext context,int suppInfo) {
        return null;
    }

    public static class ATT {
        public static void addCallExtraForApp(final SuppInfo suppInfo,
                ImsCallProfile outProfile) {
            setCallExtraInt(suppInfo, SuppInfo.TYPE_CDIV_CAUSE,
                    ImsCallProfileEx.EXTRA_CDIV_CAUSE, outProfile);
        }

        public static void addSuppInfoForIms(final ImsCallProfile profile,
                SuppInfo outSuppInfo) {
        }
    }

    public static class DCM {
        /** boolean */
        public static final int SUPP_TYPE_SUBADDRESS = 101;

        public static void addSuppInfoForIms(final ImsCallProfile profile,
                SuppInfo outSuppInfo) {
            Bundle oemCallExtras = profile.getCallExtras().getBundle(
                        ImsCallProfile.EXTRA_OEM_EXTRAS);

            // Checks if subaddress is set or not
            boolean isSubaddress = getCallExtraBoolean(
                    oemCallExtras, ImsCallProfileEx.EXTRA_SUBADDRESS, false);

            if (isSubaddress) {
                outSuppInfo.addService_bool(SUPP_TYPE_SUBADDRESS, true);
            }
        }
    }

    public static class Global {
        /** boolean */
        public static final int SUPP_TYPE_GEOLOCATION = 101;

        public static void addSuppInfoForIms(final ImsCallProfile profile,
                SuppInfo outSuppInfo) {
            Bundle callExtras = profile.getCallExtras();

            if (hasCallExtra(callExtras, EXTRA_GEOLOCATION)) {
                outSuppInfo.addService_bool(SUPP_TYPE_GEOLOCATION,
                        getCallExtraBoolean(callExtras, EXTRA_GEOLOCATION, true));
            }
        }
    }

    public static class Canada {
        /** boolean */
        public static final int SUPP_TYPE_GEOLOCATION = 101;

        public static void addSuppInfoForIms(final ImsCallProfile profile,
                SuppInfo outSuppInfo) {
            Bundle callExtras = profile.getCallExtras();

            if (hasCallExtra(callExtras, EXTRA_GEOLOCATION)) {
                outSuppInfo.addService_bool(SUPP_TYPE_GEOLOCATION,
                        getCallExtraBoolean(callExtras, EXTRA_GEOLOCATION, true));
            }
        }
    }

    private static String getCallExtra(Bundle callExtras,
            String key, String defaultValue) {
        if (callExtras == null) {
            return defaultValue;
        }

        return callExtras.getString(key, defaultValue);
    }

    private static boolean getCallExtraBoolean(Bundle callExtras,
            String key, boolean defaultValue) {
        if (callExtras == null) {
            return defaultValue;
        }

        return callExtras.getBoolean(key, defaultValue);
    }

    private static int getCallExtraInt(Bundle callExtras,
            String key, int defaultValue) {
        if (callExtras == null) {
            return defaultValue;
        }

        return callExtras.getInt(key, defaultValue);
    }

    private static boolean hasCallExtra(Bundle callExtras, String key) {
        if (callExtras == null) {
            return false;
        }

        return callExtras.containsKey(key);
    }

    private static void setCallExtra(SuppInfo si, int type,
            String key, ImsCallProfile outProfile) {
        SuppInfo.SuppService ss = si.getService(type);

        if (ss != null) {
            outProfile.setCallExtra(key, ss.strValue);
        } else {
            if (outProfile.getCallExtras() != null) {
                outProfile.getCallExtras().remove(key);
            }
        }
    }

    private static void setCallExtraBoolean(SuppInfo si, int type,
            String key, ImsCallProfile outProfile) {
        SuppInfo.SuppService ss = si.getService(type);

        if (ss != null) {
            outProfile.setCallExtraBoolean(key, ss.boolValue);
        } else {
            if (outProfile.getCallExtras() != null) {
                outProfile.getCallExtras().remove(key);
            }
        }
    }

    private static void setCallExtraInt(SuppInfo si, int type,
            String key, ImsCallProfile outProfile) {
        SuppInfo.SuppService ss = si.getService(type);

        if (ss != null) {
            outProfile.setCallExtraInt(key, ss.intValue);
        } else {
            if (outProfile.getCallExtras() != null) {
                outProfile.getCallExtras().remove(key);
            }
        }
    }
}
