package com.android.imsstack.core.config;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.pm.PackageManager;
import android.content.pm.PackageInfo;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteException;
import android.database.sqlite.SQLiteFullException;
import android.os.Build;
import android.provider.Telephony;
import android.telephony.TelephonyManager;
import android.text.TextUtils;

import com.android.imsstack.core.ImsGlobal;
import com.android.imsstack.core.OperatorInfo;
import com.android.imsstack.external.ims.ImsExternalFeature;
import com.android.imsstack.external.ims.ImsFeatureProvider;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.DBUtils;
import com.android.imsstack.util.FeatureUtils;
import com.android.imsstack.util.ImsConstants;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.ImsPrivateProperties;
import com.android.imsstack.util.ImsProperties;
import com.android.imsstack.util.ImsUtils;
import com.android.imsstack.util.Log;
import com.android.imsstack.util.MSimUtils;
import com.android.imsstack.util.SimCarrierId;
import com.android.imsstack.test.menu.SettingUtil;

import java.util.HashMap;
import java.util.StringTokenizer;

public class ProviderDBUpdateHelper {
    private static final String TAG = "ImsStack_ProviderDBUpdateHelper";

    private static final int SIP_FEATURE_TAG_MEDIA_STREAM_VIDEO = 0x00000200;

    private static final String IMSI = "IMSI";
    private static final String VERSION = "ver";
    private static final String IMS_VERSION = "6.2";

    private final ConfigXMLLoader mXMLLoader;
    private SQLiteDatabase mImsDb = null;
    private SQLiteDatabase mDb_prov = null;

    private final int mSlotId;

    public ProviderDBUpdateHelper(int slotId, ConfigXMLLoader xmlLoader) {
        mSlotId = slotId;
        mXMLLoader = xmlLoader;
    }

    public static void restoreConfigFromXml(Context context, int slotId) {
        Log.w(TAG, "restoreConfigFromXml");

        if (context == null) {
            return;
        }

        ConfigDBLoaderImpl dbLoader = new ConfigDBLoaderImpl(slotId);

        if (!dbLoader.dbUpdateByCompulsion(context)) {
            Log.e(TAG, "Updating ImsConfig failed");
        }

        ProviderDBUpdateHelper dbUpdateHelper =
                new ProviderDBUpdateHelper(slotId, dbLoader.getXmlLoader());

        dbUpdateHelper.updateDBForOperatorSpecific(context);
    }

    public boolean dbUpdateForDisableVoLTE(Context context) {
        if (init(context) != true) {
            release();
            return false;
        }

        ContentValues cvs = new ContentValues();
        cvs.put(ProviderInterface.Subscriber.SERVICES, "0x00000000");

        ContentValues cvu = new ContentValues();
        cvu.put(ProviderInterface.MMTel.MMTEL_PROVISIONING, "false");

        ContentValues cvm = new ContentValues();
        cvm.put(ProviderInterface.DBInfo.MAKE_DB_ON_NEXT_TIME, "1");

        String selection = DBUtils.selectForSlotId(mSlotId);
        boolean result = true;

        mImsDb.beginTransaction();
        try {
            // gims_subscriber
            if (mImsDb.update(ProviderInterface.Subscriber.TABLE_NAME, cvs, selection, null) > 0) {
                Log.i(TAG, "update: " + ProviderInterface.Subscriber.TABLE_NAME
                        + "/" + ProviderInterface.Subscriber.SERVICES + " -> 0");
            }

            // gims_subscriber_fake
            if (mImsDb.update(ProviderInterface.Subscriber.TABLE_NAME_FAKE,
                    cvs, selection, null) > 0) {
                Log.i(TAG, "update :" + ProviderInterface.Subscriber.TABLE_NAME_FAKE
                        + "/" + ProviderInterface.Subscriber.SERVICES + " -> 0");
            }

            // gims_mmtel
            if (mImsDb.update(ProviderInterface.MMTel.TABLE_NAME, cvu, selection, null) > 0) {
                Log.i(TAG, "update: " + ProviderInterface.MMTel.TABLE_NAME
                        + "/" + ProviderInterface.MMTel.MMTEL_PROVISIONING + " -> false");
            }

            // gims_modified_time
            if (mImsDb.update(ProviderInterface.DBInfo.TABLE_NAME, cvm, selection, null) > 0) {
                Log.i(TAG, "update :" + ProviderInterface.DBInfo.TABLE_NAME
                        + "/" + ProviderInterface.DBInfo.MAKE_DB_ON_NEXT_TIME
                        + " -> 1");
            }
            mImsDb.setTransactionSuccessful();
        } catch (SQLiteFullException e) {
            Log.e(TAG, e.toString());
            result = false;
        } catch (Exception e) {
            Log.e(TAG, e.toString());
            e.printStackTrace();
        } finally {
            DBUtils.DB.endTransaction(mImsDb);
        }

        release();

        return result;
    }

    public boolean dbUpdateForDisableEmergency(Context context) {
        if (init(context) != true) {
            release();
            return false;
        }

        ContentValues cve = new ContentValues();
        cve.put(ProviderInterface.COM_EMERGENCY.SUPPORT_OVER_LTE, "false");

        boolean result = true;
        mImsDb.beginTransaction();

        try {
            // gims_com_emergency
            if (mImsDb.update(ProviderInterface.COM_EMERGENCY.TABLE_NAME, cve
                                , ImsDbController.selectForSlot(mSlotId), null) > 0) {
                Log.i(TAG, "update: " + ProviderInterface.COM_EMERGENCY.TABLE_NAME
                        + "/" + ProviderInterface.COM_EMERGENCY.SUPPORT_OVER_LTE + " -> false");
            }
            mImsDb.setTransactionSuccessful();
        } catch (SQLiteFullException e) {
            Log.e(TAG, e.toString());
            result = false;
        } catch (Exception e) {
            Log.e(TAG, e.toString());
            e.printStackTrace();
        } finally {
            DBUtils.DB.endTransaction(mImsDb);
        }

        release();

        return result;
    }

    public boolean dbUpdateForKR(Context context) {
        if (init(context) != true) {
            release();
            return false;
        }

        if (isIMSIChanged(context)) {
            setFeatureTagsForKR(context);
            setImsFeatureForSmsOnly(context);
        }

        // Operator Specific update
        setImsPropertyForKR(context);
        setTestModeRelatedConfigsForKR(context);
        setIMSIvalue(context);
        release();

        return true;
    }

    public boolean dbUpdateForATT(Context context) {
        if (init(context) != true) {
            release();
            return false;
        }

        setCustomUserAgentHeader(context);

        if (OperatorInfo.isNaOpen()) {
            boolean configEnabled = !(ImsProperties.getSysSimOperator(mSlotId).contains("TRF"));

            updateConfigForRoaming(configEnabled);
            updateConfigForUSSI(configEnabled);
        }

        Log.w(TAG, "Update anonymous display name");
        if (DBUtils.DB.ImmediateMode.putString(mSlotId, mImsDb,
                ProviderInterface.Subscriber.TABLE_NAME_FAKE,
                ProviderInterface.Subscriber.IMPU_0,
                "\"Anonymous\"<sip:anonymous@anonymous.invalid>")) {
        } else {
            Log.e(TAG, "Failed updating anonymous display name");
        }

        release();

        return true;
    }

    public boolean dbUpdateForTMUS(Context context) {
        if (init(context) != true) {
            release();
            return false;
        }

        if (ImsProperties.isVoNrEnabled()) {
            //To update User-Agent's "VoLTE" to "EPSFB"
            String strUA = DBUtils.DB.getString(mSlotId,
                mImsDb, ProviderInterface.SIP.TABLE_NAME_COM,
                ProviderInterface.SIP.SERVICE_VERSION, "");
            String strUAFMT = DBUtils.DB.getString(mSlotId,
                mImsDb, ProviderInterface.SIP.TABLE_NAME_COM,
                ProviderInterface.SIP.USER_AGENT_TEMPLATE, "");

            if (!TextUtils.isEmpty(strUA) && strUA.contains("VoLTE")) {
                DBUtils.DB.ImmediateMode.putString(mSlotId,
                    mImsDb, ProviderInterface.SIP.TABLE_NAME_COM,
                    ProviderInterface.SIP.SERVICE_VERSION, strUA.replace("VoLTE", "EPSFB"));
            }

            if (!TextUtils.isEmpty(strUAFMT) && strUAFMT.contains("VoLTE")) {
                DBUtils.DB.ImmediateMode.putString(mSlotId,
                    mImsDb, ProviderInterface.SIP.TABLE_NAME_COM,
                    ProviderInterface.SIP.USER_AGENT_TEMPLATE, strUAFMT.replace("VoLTE", "EPSFB"));
            }
        }

        if (DBUtils.DB.ImmediateMode.putString(mSlotId, mImsDb,
                ProviderInterface.Subscriber.TABLE_NAME_FAKE,
                ProviderInterface.Subscriber.IMPU_0,
                "\"Anonymous\"<sip:anonymous@anonymous.invalid>")) {
        } else {
            Log.e(TAG, "Failed updating anonymous display name");
        }

        setCustomUserAgentHeader(context);

        release();

        return true;
    }

    public boolean dbUpdateForTMUSOnSimLoaded(Context context) {
        if (!init(context)) {
           release();
           return false;
        }

        if (OperatorInfo.isNaOpen()) {
            String simOperator = ImsProperties.getSysSimOperator(mSlotId);

            boolean wfcEnabled = (isGoogleFiSim() || simOperator.contains("CCA")) ? false : true;
            updateConfigForVoWiFi(wfcEnabled);
        }

        release();

        return true;
    }

    public boolean dbUpdateForDCM(Context context) {
        if (init(context) != true) {
            release();
            return false;
        }

        boolean isDeviceVtEnabled = ImsFeatureProvider.hasVt(context);
        int enabledService = ProviderInterface.Subscriber.AdminServices.VOLTE;

        if (isDeviceVtEnabled) {
            enabledService |= ProviderInterface.Subscriber.AdminServices.VILTE;
        }

        DBUtils.DB.ImmediateMode.putHex(mSlotId,
                mImsDb, ProviderInterface.Subscriber.TABLE_NAME,
                ProviderInterface.Subscriber.SERVICES, enabledService);

        release();

        return true;
    }

    public boolean dbUpdateForKDDI(Context context) {
        if (init(context) != true) {
            release();
            return false;
        }

        setCustomUserAgentHeader(context);

        release();

        return true;
    }

    public boolean dbUpdateForSBM(Context context) {
        if (init(context) != true) {
            release();
            return false;
        }

        //For OSU
        HashMap<String, Integer> hashModel = new HashMap<String, Integer>();
        hashModel.put("801LG", 1);
        hashModel.put("X5-LG", 2);
        hashModel.put("802LG", 3);
        hashModel.put("901LG", 4);
        hashModel.put("A001LG", 5);

        boolean bIsContainsKey = hashModel.containsKey(ImsProperties.MODEL);
        Log.i(TAG, "dbUpdateForSBM - MODEL : " + ImsProperties.MODEL +
                ", containsKey : " + bIsContainsKey);

        if (bIsContainsKey) {
            int nModelNum = hashModel.get(ImsProperties.MODEL);

            if (nModelNum < 3) { //Enable XCAP
                DBUtils.DB.ImmediateMode.putString(mSlotId,
                        mImsDb, ProviderInterface.MMTel.TABLE_NAME,
                        ProviderInterface.MMTel.MMTEL_PROVISIONING, "true");
                DBUtils.DB.ImmediateMode.putString(mSlotId,
                        mImsDb, ProviderInterface.GBA.TABLE_NAME,
                        ProviderInterface.GBA.GBA_ENABLED, "true");
            }

            if (nModelNum < 4) { //Remove subfork
                DBUtils.DB.ImmediateMode.putString(mSlotId,
                        mImsDb, ProviderInterface.SIP.TABLE_NAME_COM,
                        ProviderInterface.SIP.USER_AGENT_TEMPLATE, "#IMEISV");
            }

            if (nModelNum < 5) { //Revert RTP Inactivity timer to 20s
                DBUtils.DB.ImmediateMode.putString(mSlotId,
                        mImsDb, ProviderInterface.SessionVoIP.TABLE_NAME,
                        ProviderInterface.SessionVoIP.TIME_ACTIVE_MEDIA_TH, "20");
            }
        }

        setCustomUserAgentHeader(context);

        release();

        return true;
    }

    public boolean dbUpdateForVZW(Context context) {
        if (!init(context)) {
            release();
            return false;
        }

        setCustomUserAgentHeader(context);

        if (FeatureUtils.isSMSOnlySupported(context)) {
            setDeviceMode_VZW(context, 2);
        } else if (FeatureUtils.isCdmaLessSupported(context)) {
            setDeviceMode_VZW(context, 3);
        } else {
            if (!FeatureUtils.isHVolteSupported(context)) {
                setDeviceMode_VZW(context, 0);
            }
        }

        setTraceOption_VZW(context);

        release();

        return true;
    }

    public boolean dbUpdateForVZWMVNOs (Context context) {
        if (!init(context)) {
           release();
           return false;
        }

        String laopBrand = "";

        Log.i(TAG, "VZWMVNOs - LAOP_BRAND : " + laopBrand);

        // LRA / OPEN / AMZ case
        if ("LRACG".equalsIgnoreCase(laopBrand) ||
                "NAO".equalsIgnoreCase(laopBrand) ||
                "AMZ".equalsIgnoreCase(laopBrand)) {
            if(ImsFeatureProvider.hasVt(context)) {
                updateConfigForVT(true);
            } else {
                updateConfigForVT(false);
            }

            if (FeatureUtils.isVoWiFiSupported(context)) {
                updateConfigForVoWiFi(true);
            } else {
                updateConfigForVoWiFi(false);
            }
        } else {
           // default
           release();
           return false;
        }

        release();

        return true;
    }

    public boolean dbUpdateForGlobal(Context context) {
        if (init(context) != true) {
            release();
            return false;
        }

        setCustomUserAgentHeader(context);
        setEmergencyCapabiltityForGlobal(context);
        setVoWiFiIndicatorForGlobal();

        release();

        return true;
    }

    public boolean dbUpdateForSPR(Context context) {
        if (init(context) != true) {
            release();
            return false;
        }

        setCustomUserAgentHeader(context);

        release();

        return true;
    }

    public boolean dbUpdateForCanada(Context context) {
        if (init(context) != true) {
            release();
            return false;
        }

        setCustomUserAgentHeader(context);

        release();

        return true;
    }

    public boolean dbUpdateForUSC(Context context) {
        if (init(context) != true) {
            release();
            return false;
        }

        setCustomUserAgentHeader(context);

        release();

        return true;
    }


    public boolean dbUpdateForACG(Context context) {
        if (init(context) != true) {
            release();
            return false;
        }

        setCustomUserAgentHeader(context);

        release();

        return true;
    }

    public boolean dbUpdateSvcModifiedTime(Context context, int svcModifiedTime) {
        if (init(context) != true) {
            release();
            return false;
        }

        ContentValues cvm = new ContentValues();
        cvm.put(ProviderInterface.DBInfo.MAKE_DB_ON_NEXT_TIME, svcModifiedTime);

        boolean result = true;
        mImsDb.beginTransaction();

        try {
            // gims_modified_time
            if (mImsDb.update(ProviderInterface.DBInfo.TABLE_NAME, cvm
                                , ImsDbController.selectForSlot(mSlotId), null) > 0) {
                Log.i(TAG, "update :" + ProviderInterface.DBInfo.TABLE_NAME
                        + "/" + ProviderInterface.DBInfo.MAKE_DB_ON_NEXT_TIME
                        + " -> " + svcModifiedTime);
            }

            mImsDb.setTransactionSuccessful();
        } catch (SQLiteFullException e) {
            Log.e(TAG, e.toString());
            result = false;
        } catch (Exception e) {
            Log.e(TAG, e.toString());
            e.printStackTrace();
        } finally {
            DBUtils.DB.endTransaction(mImsDb);
        }

        release();

        return true;
    }


    public void updateDBForOperatorSpecific(Context context) {
        String op = ImsGlobal.getOperator(mSlotId);
        String co = ImsGlobal.getCountry(mSlotId);

        Log.i(TAG, "update-config :: op=" + op + ", co=" + co);

        if (ImsGlobal.equalsCountry(co, "KR")) {
            dbUpdateForKR(context);
        } else if (ImsGlobal.equalsOperatorCountry(op, co, "ATT", "US")) {
            dbUpdateForATT(context);
        } else if (ImsGlobal.equalsOperatorCountry(op, co, "TMO", "US")
                    || ImsGlobal.equalsOperator(op, "MPCS")
                    || ImsGlobal.equalsOperatorCountry(op, co, "TRF", "US")) {
            dbUpdateForTMUS(context);
        } else if (ImsGlobal.equalsOperator(op, "VZW")) {
            dbUpdateForVZW(context);
        } else if (ImsGlobal.equalsOperator(op, "DCM")) {
            dbUpdateForDCM(context);
        } else if (ImsGlobal.equalsOperator(op, "KDDI")) {
            dbUpdateForKDDI(context);
        } else if (ImsGlobal.equalsOperator(op, "SBM")) {
            dbUpdateForSBM(context);
        } else if (OperatorInfo.isEnablerTypeGlobal(mSlotId)) {
            dbUpdateForGlobal(context);
        } else if (ImsGlobal.equalsOperator(op, "SPR")) {
            dbUpdateForSPR(context);
        } else if (ImsGlobal.equalsOperator(op, "USC")) {
            dbUpdateForUSC(context);
        } else if (ImsGlobal.equalsOperator(op, "ACG")) {
            dbUpdateForACG(context);
        } else if (ImsGlobal.equalsCountry(co, "CA")) {
            dbUpdateForCanada(context);
        }
    }

    // Operator specific initialization on SIM LOADED -- starts
    public static boolean isDBUpdateRequiredForOperatorSpecificOnSimLoaded(int slotId) {
        if ("KR".equals(ImsProperties.TARGET_COUNTRY)) {
            return true;
        }

        String op = ImsGlobal.getOperator(slotId);

        if (OperatorInfo.isNaOpen()) {
            if (ImsGlobal.equalsOperator(op, "ATT")
                || ImsGlobal.equalsOperator(op, "TMO")
                || ImsGlobal.equalsOperator(op, "VZW")) {
                return true;
            }
        }

        if (ImsGlobal.equalsOperator(op, "TMO")) {
            return true;
        }

        if (ImsGlobal.equalsOperator(op, "TEL")) {
           return true;
        }

        return false;
    }

    public boolean updateDBForOperatorSpecificOnSimLoaded(Context context) {
        String op = ImsGlobal.getOperator(mSlotId);
        String co = ImsGlobal.getCountry(mSlotId);

        Log.i(TAG, "update-config-on-sim-loaded :: op=" + op + ", co=" + co);

        // If operator requires to change IMS configuration whenever SIM is loaded,
        // please add the logic in here.
        if (ImsGlobal.equalsCountry(co, "KR")) {
            dbUpdateForKR(context);
            return false;
        }

        if (ImsGlobal.equalsOperatorCountry(op, co, "ATT", "US")) {
            return dbUpdateForATT(context);
        }

        if (ImsGlobal.equalsOperatorCountry(op, co, "TMO", "US")) {
            return dbUpdateForTMUSOnSimLoaded(context);
        }

        if (ImsGlobal.equalsOperatorCountry(op, co, "VZW", "US")) {
            return dbUpdateForVZWMVNOs(context);
        }

        if (ImsGlobal.equalsOperatorCountry(op, co, "TEL", "AU")) {
            return dbUpdateForGlobal(context);
        }

        return false;
    }
    // Operator specific initialization on SIM LOADED -- ends

    public void updateServiceAvailability(Context context) {
        if (!OperatorInfo.isEnablerTypeForNonOperator(mSlotId)) {
            return;
        }

        Log.i(TAG, "updateServiceAvailability :: slotId=" + mSlotId);

        if (init(context) != true) {
            release();
            return;
        }

        final int iSERVICE_VOLTE      = ProviderInterface.Subscriber.AdminServices.VOLTE;
        final int iSERVICE_VILTE      = ProviderInterface.Subscriber.AdminServices.VILTE;
        final int iSERVICE_VOWIFI     = ProviderInterface.Subscriber.AdminServices.VOWIFI;

        boolean bVoLTEEnabled  = OperatorInfo.isSupportVolte(mSlotId);
        boolean bVideo = OperatorInfo.isSupportVt(mSlotId);
        boolean bWLAN  = OperatorInfo.isSupportVowifi(mSlotId);
        boolean bVoice = bVoLTEEnabled || bWLAN;
        boolean bLTE   = bVoLTEEnabled || bVideo;

        int adminServices = DBUtils.DB.getHex(mSlotId,
                mImsDb, ProviderInterface.Subscriber.TABLE_NAME,
                ProviderInterface.Subscriber.SERVICES, 0);

        Log.d(TAG, "Current admin_services/Subscriber : " + adminServices);

        if (bVoLTEEnabled) {
            adminServices |= iSERVICE_VOLTE;
        }
        else {
            adminServices &= (~iSERVICE_VOLTE);
        }

        if (bWLAN) {
            adminServices |= iSERVICE_VOWIFI;
        }
        else {
            adminServices &= (~iSERVICE_VOWIFI);
        }

        if (bVideo) {
            adminServices |= iSERVICE_VILTE;
        }
        else {
            adminServices &= (~iSERVICE_VILTE);
        }
        Log.d(TAG, "Updated admin_Services/Subscriber : " + adminServices);

        DBUtils.DB.ImmediateMode.putHex(mSlotId,
                mImsDb, ProviderInterface.Subscriber.TABLE_NAME,
                ProviderInterface.Subscriber.SERVICES, adminServices);

        Log.d(TAG, "updateServiceAvailability : Voice/Video/VxLTE/VxWIFI : "
            + bVoice + "/" + bVideo + "/" + bLTE + "/" + bWLAN);

        final int iNW_LTE            = 0x20000000;
        final int iNW_WLAN           = 0x01000000;
        final int iAOS_CONDITION_SET_EPDG_PREFERENCE = 0x00000020;

        /////// BEGIN of SwitchVT
        int featureTag = DBUtils.DB.getHex(mSlotId,
                mImsDb, ProviderInterface.SIP.TABLE_NAME_COM,
                ProviderInterface.SIP.FEATURE_TAGS, 0);
        Log.d(TAG, "Current Feature Tag : " + featureTag);

        if (bVoice == true && bVideo == true) {
            String aos_conf_uc = DBUtils.DB.getString(mSlotId,
                    mImsDb, ProviderInterface.AOS.TABLE_NAME,
                    ProviderInterface.AOS.CONF, "");
            String app_uc_conf_uc = DBUtils.DB.getString(mSlotId,
                    mImsDb, ProviderInterface.COM_APP_UC.TABLE_NAME,
                    ProviderInterface.COM_APP_UC.CONF, "");
            String media_conf_uc = DBUtils.DB.getString(mSlotId,
                    mImsDb, ProviderInterface.MEDIA.TABLE_NAME,
                    ProviderInterface.MEDIA.CONF, "");
            String media_cap_conf_uc = DBUtils.DB.getString(mSlotId,
                    mImsDb, ProviderInterface.COM_MEDIA_CAPABILITIES.TABLE_NAME,
                    ProviderInterface.COM_MEDIA_CAPABILITIES.CONF, "");

            DBUtils.DB.ImmediateMode.putString(mSlotId,
                    mImsDb, ProviderInterface.AOS.TABLE_NAME,
                    ProviderInterface.AOS.CONF, aos_conf_uc);
            DBUtils.DB.ImmediateMode.putString(mSlotId,
                    mImsDb, ProviderInterface.COM_APP_UC.TABLE_NAME,
                    ProviderInterface.COM_APP_UC.CONF, app_uc_conf_uc);
            DBUtils.DB.ImmediateMode.putString(mSlotId,
                    mImsDb, ProviderInterface.MEDIA.TABLE_NAME,
                    ProviderInterface.MEDIA.CONF, media_conf_uc);
            DBUtils.DB.ImmediateMode.putString(mSlotId,
                    mImsDb, ProviderInterface.COM_MEDIA_CAPABILITIES.TABLE_NAME,
                    ProviderInterface.COM_MEDIA_CAPABILITIES.CONF, media_cap_conf_uc);

            featureTag |= SIP_FEATURE_TAG_MEDIA_STREAM_VIDEO;
            //DBUtils.DB.putString(mSlotId, mImsDb, ProviderInterface.UC_APP.TABLE_NAME,
            //        ProviderInterface.UC_APP.CONVERT, "1");
            //DBUtils.DB.putString(mSlotId, mImsDb, ProviderInterface.SessionVt.TABLE_NAME,
            //        ProviderInterface.SessionVt.LOCAL_FEATURE, "31");
        }
        else {
            // By default, update service.voip/media.voip
            String aos_conf_voip = DBUtils.DB.getString(mSlotId,
                    mImsDb, ProviderInterface.AOS.TABLE_NAME,
                    ProviderInterface.AOS.CONF, "");
            String app_uc_conf_voip = DBUtils.DB.getString(mSlotId,
                    mImsDb, ProviderInterface.COM_APP_UC.TABLE_NAME,
                    ProviderInterface.COM_APP_UC.CONF, "");
            String media_conf_voip = DBUtils.DB.getString(mSlotId,
                    mImsDb, ProviderInterface.MEDIA.TABLE_NAME,
                    ProviderInterface.MEDIA.CONF, "");
            String media_cap_conf_voip = DBUtils.DB.getString(mSlotId,
                    mImsDb, ProviderInterface.COM_MEDIA_CAPABILITIES.TABLE_NAME,
                    ProviderInterface.COM_MEDIA_CAPABILITIES.CONF, "");

            DBUtils.DB.ImmediateMode.putString(mSlotId,
                    mImsDb, ProviderInterface.AOS.TABLE_NAME,
                    ProviderInterface.AOS.CONF, aos_conf_voip);
            DBUtils.DB.ImmediateMode.putString(mSlotId,
                    mImsDb, ProviderInterface.COM_APP_UC.TABLE_NAME,
                    ProviderInterface.COM_APP_UC.CONF, app_uc_conf_voip);
            DBUtils.DB.ImmediateMode.putString(mSlotId,
                    mImsDb, ProviderInterface.MEDIA.TABLE_NAME,
                    ProviderInterface.MEDIA.CONF, media_conf_voip);
            DBUtils.DB.ImmediateMode.putString(mSlotId,
                    mImsDb, ProviderInterface.COM_MEDIA_CAPABILITIES.TABLE_NAME,
                    ProviderInterface.COM_MEDIA_CAPABILITIES.CONF, media_cap_conf_voip);

            featureTag &= (~SIP_FEATURE_TAG_MEDIA_STREAM_VIDEO);
            //DBUtils.DB.putString(mSlotId, mImsDb, ProviderInterface.UC_APP.TABLE_NAME,
            //        ProviderInterface.UC_APP.CONVERT, "0");
            //DBUtils.DB.putString(mSlotId, mImsDb, ProviderInterface.SessionVt.TABLE_NAME,
            //        ProviderInterface.SessionVt.LOCAL_FEATURE, "0");
        }
        DBUtils.DB.ImmediateMode.putHex(mSlotId,
                mImsDb, ProviderInterface.SIP.TABLE_NAME_COM,
                ProviderInterface.SIP.FEATURE_TAGS, featureTag);
        //////// END OF SwitchVT

        /////// BEGIN of SwitchLTE/WLAN
        featureTag = DBUtils.DB.getHex(mSlotId,
                mImsDb, ProviderInterface.AoSConnection.TABLE_NAME,
                ProviderInterface.AoSConnection.ACCESS_POLICY_0, 0);
        Log.d(TAG, "Current Feature Tag : " + featureTag);

        // Only VoLTE : 1, Only VoWiFi : 2, VoLTE+VoWiFi : 2, Only VZW : 1
        int ePDGScheme = 0;
        if (bLTE) {
            featureTag |= iNW_LTE;
        }
        else {
            featureTag &= (~iNW_LTE);
        }
        if (bWLAN) {
            featureTag |= iNW_WLAN;
            ePDGScheme = 2;
        }
        else {
            featureTag &= (~iNW_WLAN);
            ePDGScheme = 0;
        }

        // 1. Access Policy Updating
        DBUtils.DB.ImmediateMode.putHex(mSlotId,
                mImsDb, ProviderInterface.AoSConnection.TABLE_NAME,
                ProviderInterface.AoSConnection.ACCESS_POLICY_0, featureTag);

        // 2. EPDG Scheme Updating
        DBUtils.DB.ImmediateMode.putInt(mSlotId,
                mImsDb, ProviderInterface.AoSConnection.TABLE_NAME,
                "aos_connection_0_epdg_scheme", ePDGScheme);

        // 3. EPDG Preference Updating
        featureTag = DBUtils.DB.getHex(mSlotId,
                mImsDb, ProviderInterface.AoSCondition.TABLE_NAME,
                ProviderInterface.AoSCondition.FEATURES_0, 0);
        if (bWLAN) {
            featureTag |= iAOS_CONDITION_SET_EPDG_PREFERENCE;
        }
        else {
            featureTag &= (~iAOS_CONDITION_SET_EPDG_PREFERENCE);
        }
        DBUtils.DB.ImmediateMode.putHex(mSlotId,
                mImsDb, ProviderInterface.AoSCondition.TABLE_NAME,
                ProviderInterface.AoSCondition.FEATURES_0, featureTag);

        Log.d(TAG, "FEATURE_VOLTE --> " + ((bVoice && bLTE) ? "1" : "0"));
        DBUtils.DB.ImmediateMode.putString(mSlotId,
                mImsDb, ProviderInterface.FEATURE.TABLE_NAME,
                ProviderInterface.FEATURE.FEATURE_VOLTE, ((bVoice && bLTE) ? "1" : "0"));

        Log.d(TAG, "FEATURE_VOWIFI --> " + ((bVoice && bWLAN) ? "1" : "0"));
        DBUtils.DB.ImmediateMode.putString(mSlotId,
                mImsDb, ProviderInterface.FEATURE.TABLE_NAME,
                ProviderInterface.FEATURE.FEATURE_VOWIFI, ((bVoice && bWLAN) ? "1" : "0"));

        Log.d(TAG, "FEATURE_VT --> " + ((bVideo && bLTE) ? "1" : "0"));
        DBUtils.DB.ImmediateMode.putString(mSlotId,
                mImsDb, ProviderInterface.FEATURE.TABLE_NAME,
                ProviderInterface.FEATURE.FEATURE_VT, ((bVideo && bLTE) ? "1" : "0"));
        /////// END of SwitchLTE/WLAN

        release();
    }

    private boolean init(Context context) {
        mImsDb = DBUtils.DB.open(ProviderInterface.DBFP, SQLiteDatabase.OPEN_READWRITE);

        if (mImsDb == null) {
            return false;
        }

        return true;
    }

    private void release() {
        DBUtils.DB.close(mImsDb);
        mImsDb = null;
        DBUtils.DB.close(mDb_prov);
        mDb_prov = null;
    }

    private void setTestModeRelatedConfigsForKR(Context context) {
        boolean testModeEnabled = isTestModeEnabled();

        // To synchronize with "ims_admin_testmode" of CallSettingProvider
        // when restarting Ims process
        SettingUtil.setTestModeForApp(context, testModeEnabled ? 1 : 0);

        if (!testModeEnabled) {
            if (!ImsDbController.Subscriber.setAdminFeatures(mSlotId
                    , ProviderInterface.Subscriber.AdminFeatures.ISIM
                    , ProviderInterface.Subscriber.AdminFeatures.USIM)) {
                Log.d(TAG, "Updating ISIM/USIM feature failed");
            }
            setCustomUserAgentHeader(context);
        }
    }

    private boolean updateConfigForRoaming(boolean roaming) {
        String value = roaming ? "1" : "0";
        ContentValues cvs = new ContentValues();

        cvs.put(ProviderInterface.FEATURE.FEATURE_VOLTE_IN_ROAMING, value);
        cvs.put(ProviderInterface.FEATURE.FEATURE_VT_IN_ROAMING, value);

        return updateConfig(ProviderInterface.FEATURE.TABLE_NAME, cvs);
    }

    private boolean updateConfigForUSSI(boolean ussi) {
        String value = ussi ? "true" : "false";
        ContentValues cvs = new ContentValues();

        cvs.put(ProviderInterface.MMTel.MMTEL_USSD_OVER_IMS, value);

        return updateConfig(ProviderInterface.MMTel.TABLE_NAME, cvs);
    }

    private boolean updateConfigForVT(boolean vt) {
        String value = vt ? "1" : "0";
        ContentValues cvs = new ContentValues();

        cvs.put(ProviderInterface.FEATURE.FEATURE_VT, value);

        return updateConfig(ProviderInterface.FEATURE.TABLE_NAME, cvs);
    }

    private boolean updateConfigForVoWiFi(boolean vowifi) {
        String value = vowifi ? "1" : "0";
        ContentValues cvs = new ContentValues();

        cvs.put(ProviderInterface.FEATURE.FEATURE_VOWIFI, value);

        return updateConfig(ProviderInterface.FEATURE.TABLE_NAME, cvs);
    }

    private boolean updateConfig(String table, ContentValues cvs) {
        Log.i(TAG, "updateConfig :: table=" + table);

        mImsDb.beginTransaction();

        try {
            int count = mImsDb.update(table, cvs, DBUtils.selectForSlotId(mSlotId), null);
            mImsDb.setTransactionSuccessful();
            return (count > 0);
        } catch (SQLiteFullException e) {
            Log.e(TAG, "SQLiteFullException");
            e.printStackTrace();
        } catch (Exception e) {
            Log.e(TAG, "Exception");
            e.printStackTrace();
        } finally {
            DBUtils.DB.endTransaction(mImsDb);
        }

        return false;
    }

    private void setCustomUserAgentHeader(Context context) {
        boolean usePredefinedUserAgent = ImsPrivateProperties.Persistent.getBoolean(
                ImsPrivateProperties.Persistent.KEY_USE_PREDEFINED_USER_AGENT, false, mSlotId);

        if (usePredefinedUserAgent) {
            Log.i(TAG, "Use predefined user-agent");
            return;
        }

        String strUserAgent = "";
        String strUA = DBUtils.DB.getString(mSlotId,
                mImsDb, ProviderInterface.SIP.TABLE_NAME_COM,
                ProviderInterface.SIP.SERVICE_VERSION,"");
        String strUAFMT = DBUtils.DB.getString(mSlotId,
                mImsDb, ProviderInterface.SIP.TABLE_NAME_COM,
                ProviderInterface.SIP.USER_AGENT_TEMPLATE,"");

        if (!TextUtils.isEmpty(strUAFMT)) {
            UserAgent userAgent = new UserAgent(context);
            updateOperatorSpecificRule(context, strUAFMT, userAgent);
            userAgent.createUserAgent(strUAFMT);
            strUserAgent = userAgent.getUserAgent();
        } else if (TextUtils.isEmpty(strUA)) {
            strUserAgent = ImsProperties.SW_VERSION + " XXX-IMS-client 6.0";
        } else {
            Log.w(TAG,"setCustomUserAgentHeader - No Need to Update Current UserAgent is "
                    + strUA);
            return;
        }

        Log.w(TAG, "setCustomUserAgentHeader - " + strUserAgent);
        // COM_SIP
        if (DBUtils.DB.ImmediateMode.putString(mSlotId, mImsDb,
                ProviderInterface.SIP.TABLE_NAME_COM,
                ProviderInterface.SIP.SERVICE_VERSION, strUserAgent)) {
        } else {
            Log.e(TAG, "setCustomUserAgentHeader_SIP Failed updating");
        }

        // COM_SIP_VT
        if (DBUtils.DB.ImmediateMode.putString(mSlotId, mImsDb,
                ProviderInterface.SIP.TABLE_NAME_COM_VT,
                ProviderInterface.SIP.SERVICE_VERSION, strUserAgent)) {
        } else {
            Log.e(TAG, "setCustomUserAgentHeader_VT Failed updating");
        }

        // COM_SIP_SMS
        if (DBUtils.DB.ImmediateMode.putString(mSlotId, mImsDb,
                ProviderInterface.SIP.TABLE_NAME_COM_SMS,
                ProviderInterface.SIP.SERVICE_VERSION, strUserAgent)) {
        } else {
            Log.e(TAG, "setCustomUserAgentHeader_SMS Failed updating");
        }

    }

    private void setEmergencyCapabiltityForGlobal(Context context) {
        if (ImsProperties.TARGET_COUNTRY.equals("AU")) {
            // AU is controlled at ECallConfigUtil.java
            return;
        }

        if (OperatorInfo.isUnknownOperator(mSlotId)) {
            // For No UICC case
            dbUpdateForDisableEmergency(context);
        }
    }

    private void setVoWiFiIndicatorForGlobal() {
        if (!"AME".equalsIgnoreCase(ImsProperties.TARGET_REGION)) {
            return;
        }

        DBUtils.DB.ImmediateMode.putString(mSlotId,
                mImsDb, ProviderInterface.Resource.TABLE_NAME,
                ProviderInterface.Resource.VOWIFI_ICON_INDICATOR_RESOURCE,
                "stat_sys_vowifi_default_text");
    }

    private void setDeviceMode_VZW(Context context, int mode) {
        Cursor c = null;

        try {
            c = DBUtils.CP.getFirstCursor(mSlotId,
                    context.getContentResolver(), ProviderInterface.Setting.CONTENT_URI);

            if (c != null) {
                ContentValues cvs = new ContentValues();

                cvs.put(ProviderInterface.Setting.DEVICE_MODE, mode);

                int nUpdatedRow = DBUtils.CP.update(mSlotId,
                        context.getContentResolver(), ProviderInterface.Setting.CONTENT_URI, cvs);

                if (nUpdatedRow > 0) {
                    Log.w(TAG, "Device Mode = " + mode);
                }
            }
        } catch (Throwable t) {
            t.printStackTrace();
        } finally {
            if (c != null) {
                c.close();
                c = null;
            }
        }
    }

    private void setTraceOption_VZW(Context context) {
        if (ImsConstants.DBG) {
            return;
        }

        Cursor c = null;

        try {
            c = DBUtils.CP.getFirstCursor(mSlotId,
                    context.getContentResolver(), ProviderInterface.Engine.CONTENT_URI);

            if (c != null) {
                ContentValues cvs = new ContentValues();

                cvs.put(ProviderInterface.Engine.TRACE_OPTION, "0x0001000F");

                int nUpdatedRow = DBUtils.CP.update(mSlotId,
                        context.getContentResolver(), ProviderInterface.Engine.CONTENT_URI, cvs);

                if (nUpdatedRow > 0) {
                    Log.w(TAG, "Trace Option : log is enabled");
                }
            }
        } catch (Throwable t) {
            t.printStackTrace();
        } finally {
            if (c != null) {
                c.close();
                c = null;
            }
        }
    }

    private boolean isIMSIChanged(Context context) {
        String formerIMSI = null;
        Cursor cursor = null;

        try {
            if (mImsDb != null) {
                cursor = mImsDb.rawQuery("select * from "
                        + ProviderInterface.DBInfo.TABLE_NAME, null);

                if ((cursor != null) && cursor.moveToFirst()) {
                    formerIMSI = cursor.getString(cursor.getColumnIndex(IMSI));
                }
            }
        } catch (SQLiteException e) {
            e.printStackTrace();
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }

        String newIMSI = getImsi(null);

        if (ImsLog.isDebuggable()) {
            Log.d(TAG, "newIMSI : [" + newIMSI + "]");
        }

        if (TextUtils.isEmpty(formerIMSI)) {
            Log.d(TAG, "formerIMSI is null");
            return true;
        }

        if (newIMSI != null) {
            if (formerIMSI.equals(newIMSI)) {
                return false;
            } else {
                Log.d(TAG, "IMSI value is changed");
                return true;
            }
        }

        return false;
    }

    private boolean isTestModeEnabled() {
        int adminFeatures = ImsDbController.Subscriber.getAdminFeatures(mSlotId);
        return ImsDbController.isTestModeEnabled(adminFeatures);
    }

    private String getTTAVersion(Context context) {
        //default version setting
        String ttaVersion = "3.0";
        Log.d(TAG, "TTA Version : " + ttaVersion);
        return ttaVersion;
    }

    private void setIMSIvalue(Context context) {
        String newIMSI = getImsi("");
        boolean result = DBUtils.DB.ImmediateMode.putString(mSlotId, mImsDb,
                ProviderInterface.DBInfo.TABLE_NAME, ProviderInterface.DBInfo.IMSI, newIMSI);
        if (!result) {
            Log.d(TAG, "SlotId/Operator/Country["
                + mSlotId + "/" + OperatorInfo.getOperator(mSlotId) + "/"
                + OperatorInfo.getCountry(mSlotId) + "] - "
                + "Failed to update IMSI value");
        }
    }

    private void setFeatureTagsForKR(Context context) {
        if (FeatureUtils.isVoLTEOnlySupported(context)) {
            int featureTags = DBUtils.DB.getHex(mSlotId,
                    mImsDb, ProviderInterface.SIP.TABLE_NAME_COM,
                    ProviderInterface.SIP.FEATURE_TAGS, 0);

            if (featureTags != 0) {
                int currentFeatureTags = featureTags;

                // Remove "video" feature tag
                featureTags &= (~SIP_FEATURE_TAG_MEDIA_STREAM_VIDEO);

                if (currentFeatureTags != featureTags) {
                    DBUtils.DB.ImmediateMode.putHex(mSlotId,
                            mImsDb, ProviderInterface.SIP.TABLE_NAME_COM,
                            ProviderInterface.SIP.FEATURE_TAGS, featureTags);
                }
            }
        }
    }

    private void setImsFeatureForSmsOnly(Context context) {
        Cursor cursor = null;
        try {
            cursor = DBUtils.DB.getFirstCursor(mSlotId, mImsDb,
                    ProviderInterface.FEATURE.TABLE_NAME);

            if (cursor != null) {
                int index = cursor.getColumnIndex(ProviderInterface.FEATURE.FEATURE_VOLTE);

                if (index >= 0) {
                    int dbValue = cursor.getInt(index);
                    int featureValue = ImsFeatureProvider.hasVoLte(context) ? 1 : 0;

                    if (dbValue != featureValue) {
                        if (!DBUtils.DB.ImmediateMode.putInt(mSlotId, mImsDb,
                                ProviderInterface.FEATURE.TABLE_NAME,
                                ProviderInterface.FEATURE.FEATURE_VOLTE, featureValue)) {
                            Log.d(TAG, "Failed to update FEATURE_VOLTE");
                        }
                    }
                }
            }
        } catch (SQLiteException e) {
            e.printStackTrace();
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
    }

    private void setImsPropertyForKR(Context context) {
        ImsPrivateProperties.Ephemeral.set(ImsPrivateProperties.Ephemeral.KEY_KR_TTA_VERSION,
                getTTAVersion(context), mSlotId);
    }

    private static boolean contains(String[] stringArray, String value) {
        if (TextUtils.isEmpty(value)) {
            return false;
        }

        for (String item : stringArray) {
            if (value.contains(item)) {
                ImsLog.d("This model's pcscf order changed :: " + ImsLog.hiddenString(value));
                return true;
            }
        }

        return false;
    }

    private void updateOperatorSpecificRule(Context context, String serviceVer,
        UserAgent userAgent) {

        if (ImsGlobal.isCountry(mSlotId, "KR")) {

            if ("OPEN".equals(ImsProperties.TARGET_OPERATOR)) {
                userAgent.addRule("OPERATOR", "OMD");
            }
        }
    }

    private String getImsi(String def) {
        TelephonyManager tm = null;

        if (MSimUtils.isMultiSimEnabled()) {
            tm = AppContext.getTelephonyManager(MSimUtils.getSubId(mSlotId));
        } else {
            tm = AppContext.getTelephonyManager();
        }

        String imsi = (tm != null) ? tm.getSubscriberId() : null;

        return (imsi != null) ? imsi : def;
    }

    private boolean isGoogleFiSim() {
        TelephonyManager tm = AppContext.getTelephonyManager(MSimUtils.getSubId(mSlotId));
        return (tm != null) && tm.getSimCarrierId() == SimCarrierId.GOOGLE_FI;
    }

    private class UserAgent {

        private Context mContext = null;
        private String mUserAgent = "";
        private HashMap<String, String> mapCustRule = null;

        static private final int MODEL_TYPE_NORMAL = 1;
        static private final int MODEL_TYPE_TRIM1 = 2;
        static private final int MODEL_TYPE_TRIM2 = 3;

        public UserAgent(Context context) {

            mContext = context;
            mapCustRule = new HashMap<String, String>();
        }

        public void createUserAgent(String template) {
            String strUserAgent = "";

            mUserAgent = template;
            updateByRule();

            StringTokenizer st = new StringTokenizer(mUserAgent, "#");
            String temp = "";

            while (st.hasMoreTokens()) {

                temp = st.nextToken();

                switch (temp) {

                    case "DEVICE_ID":
                        strUserAgent = strUserAgent + getImei();
                        break;

                    case "MODEL":
                        strUserAgent = strUserAgent + getModel(MODEL_TYPE_NORMAL);
                        break;

                    case "MODEL_TRIM1":
                        strUserAgent = strUserAgent + getModel(MODEL_TYPE_TRIM1);
                        break;

                    case "MODEL_TRIM2":
                        strUserAgent = strUserAgent + getModel(MODEL_TYPE_TRIM2);
                        break;

                    case "SW_VERSION_SHORT":
                        strUserAgent = strUserAgent + ImsProperties.SW_VERSION_3CHARS;
                        break;

                    case "SOFTWARE_VERSION_SHORT":
                        strUserAgent = strUserAgent + ImsProperties.SW_VERSION_SHORT;
                        break;

                    case "SW_VERSION":
                        strUserAgent = strUserAgent + ImsProperties.SW_VERSION;
                        break;

                    case "KR_SW_VERSION":
                        strUserAgent = strUserAgent + getKRSWVersion();
                        break;

                    case "IMS_VERSION":
                        strUserAgent = strUserAgent + IMS_VERSION + getMinorVersion();
                        break;

                    case "COUNTRY":
                        strUserAgent = strUserAgent + OperatorInfo.getCountry(mSlotId);
                        break;

                    case "OPERATOR":
                        strUserAgent = strUserAgent + ImsProperties.IMS_TARGET_OPERATOR;
                        break;

                    case "OS_VERSION":
                        strUserAgent = strUserAgent + Build.VERSION.RELEASE;
                        break;

                    case "SVN":
                        strUserAgent = strUserAgent + getSvn();
                        break;

                    case "MCC":
                        strUserAgent = strUserAgent + getMcc();
                        break;

                    case "MNC":
                        strUserAgent = strUserAgent + getMnc(false);
                        break;

                    case "MNC_PAD":
                        strUserAgent = strUserAgent + getMnc(true);
                        break;

                    case "TTAV":
                        strUserAgent = strUserAgent + getTTAVersion(mContext);
                        break;

                    case "TAC":
                        strUserAgent = strUserAgent + getTaccode();
                        break;

                    case "DEVICE_TYPE":
                        strUserAgent = strUserAgent + getDeviceType();
                        break;

                    case "IMEISV":
                        strUserAgent = strUserAgent + getImeiSV();
                        break;

                    case "MESSAGEAPP_VERSION":
                        strUserAgent = strUserAgent + getMessageAppVersion(mContext);
                        break;

                    default:
                        strUserAgent = strUserAgent + temp;
                        break;
                }
            }

            mUserAgent = strUserAgent;

        }

        public String getUserAgent() {
            return mUserAgent;
        }

        private String getKRSWVersion() {
            String mStrModel = ImsProperties.MODEL;
            String mStrHydraName = "";
            String mStrKRSWversion = "";

            if ("LGU".equalsIgnoreCase(ImsProperties.TARGET_OPERATOR)
                    || "OPEN".equalsIgnoreCase(ImsProperties.TARGET_OPERATOR)) {
                mStrKRSWversion = "ver" + ImsProperties.SW_VERSION_3CHARS;
            }
            else if ("KT".equalsIgnoreCase(ImsProperties.TARGET_OPERATOR)){
                switch(mStrHydraName) {
                    case "Plus":
                        mStrKRSWversion = mStrModel.substring(mStrModel.indexOf('-') + 1)
                            + "PK" + ImsProperties.SW_VERSION_3CHARS;
                        break;
                    case "Prime":
                        mStrKRSWversion = mStrModel.substring(mStrModel.indexOf('-') + 1)
                            + 'K' + ImsProperties.SW_VERSION_3CHARS;
                        break;
                    default:
                        mStrKRSWversion = ImsProperties.SW_VERSION;
                        break;
                }
            }
            else {
                mStrKRSWversion = ImsProperties.SW_VERSION;
            }
            return mStrKRSWversion;
        }

        private String getMcc() {

            String strMCC = OperatorInfo.getSimMccMnc(mSlotId);

            if (strMCC.isEmpty()) {
                Log.e(TAG, "No UICC or Can't read MccMnc");
                return "000";
            }
            if (strMCC.length() >= 5) {
                strMCC = strMCC.substring(0, 3);
            }

            return strMCC;
        }

        private String getMnc(boolean addZeroPad) {

            String strMNC = OperatorInfo.getSimMccMnc(mSlotId);

            if (strMNC.isEmpty()) {
                Log.e(TAG, "No UICC or Can't read MccMnc");
                return "00";
            }

            if (strMNC.length() >= 5) {
                strMNC = strMNC.substring(3);
            }

            if (addZeroPad && (strMNC.length() == 2)) {
                strMNC = "0" + strMNC;
            }

            return strMNC;
        }

        public void addRule(String source, String target) {
            mapCustRule.put(source, target);
        }

        private void updateByRule() {

            String out = mUserAgent;
            for (String key : mapCustRule.keySet()) {

                mUserAgent = mUserAgent.replace(key, mapCustRule.get(key));
                Log.w(TAG, "updateByRule    = [" + key + "][" + mapCustRule.get(key) + "]");
            }
        }

        private String getModel(int type) {
            String model = ImsProperties.MODEL;

            switch (type) {

            case MODEL_TYPE_NORMAL:
                break;
            case MODEL_TYPE_TRIM1:
                if (model.contains(" ")) {
                    model = model.substring(0, model.indexOf(" "));
                }
                break;
            case MODEL_TYPE_TRIM2:
                if (model.contains("-")) {
                    model = model.substring(model.indexOf("-") + 1, model.length());
                }
                break;
            default:
                break;
            }
            return model;
        }

        private String getImei() {
            TelephonyManager tm = AppContext.getTelephonyManager();

            if (tm == null) {
               return "000000000000000";
            }

            return tm.getImei(mSlotId);
        }

        private String getImeiSV() {
            String imei = getImei();
            //3GPP 23.003 6.2.2 Composition of IMEISV (TAC+SNR+SVN)
            return imei.substring(0, imei.length() - 1) + getSvn();
        }

        private String getTaccode() {

            String imei = getImei();

            if ((imei == null) || (imei.length() < 14)) {
                return "00000000";
            }

            return imei.substring(0, 8);
        }

        private String getDeviceType() {

            if (ImsProperties.MODEL.contains("P530L")
                    || ImsProperties.MODEL.contains("LM-T600")) {
                return "Device_Type/Android_PAD";
            }
            return "Device_Type/Android_Phone";
        }

        private String getMinorVersion() {
            if (!ImsExternalFeature.FEATURE_VOLTE_OPEN) {
                return "";
            }

            String co = ImsGlobal.getCountry(mSlotId);
            if (ImsGlobal.equalsCountry(co, "KR")
                    || ImsGlobal.equalsCountry(co, "US")
                    || ImsGlobal.equalsCountry(co, "CA")) {
                return "";
            }

            String swVersion = Build.VERSION.RELEASE;
            if (swVersion.isEmpty()) {
                return "";
            }

            return "." + swVersion;
        }
    }

    private String getMessageAppVersion(Context context) {
        String version = "";

        String defaultSmsPackageName = Telephony.Sms.getDefaultSmsPackage(context);

        try {
            PackageInfo pInfo = context.getPackageManager().getPackageInfo(
                    defaultSmsPackageName, 0);
            version = pInfo.versionName;
        } catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        }

        Log.w(TAG, "defaultSmsPackageName : " + defaultSmsPackageName +
                ", version of default message : " + version);
        return version;
    }

    private String getSvn() {
        TelephonyManager tm = AppContext.getTelephonyManager();
        String svn = null;

        if (tm != null) {
            svn = tm.getDeviceSoftwareVersion(mSlotId);
        }

        return (svn == null) ? "" : svn;
    }
}
