/**
 * SettingUtil
 * Role
 *         Provide utility methods that are used in setting menu.
 *         Update setting value and display log to show db table columns.
 */

package com.android.imsstack.test.menu;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteException;
import android.preference.CheckBoxPreference;
import android.preference.EditTextPreference;
import android.preference.ListPreference;
import android.preference.Preference;

import com.android.imsstack.R;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.SubsInfoInterface;
import com.android.imsstack.core.config.ProviderInterface;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.SettingsUtils;

import java.util.HashMap;

public class SettingUtil {
    /** key, bitmask for each key */
    private static final HashMap<String, Integer> sAdminFeatures
            = new HashMap<String, Integer>();

    /** admin_xxx in gims_subscriber */
    public static void init(Context c) {
        if (sAdminFeatures.isEmpty()) {
            sAdminFeatures.put(c.getString(R.string.ims_key_admin_ims),
                    ProviderInterface.Subscriber.AdminFeatures.IMS);
            sAdminFeatures.put(c.getString(R.string.ims_key_admin_isim),
                    ProviderInterface.Subscriber.AdminFeatures.ISIM);
            sAdminFeatures.put(c.getString(R.string.ims_key_admin_usim),
                    ProviderInterface.Subscriber.AdminFeatures.USIM);
            sAdminFeatures.put(c.getString(R.string.ims_key_admin_testmode_gcf),
                    ProviderInterface.Subscriber.AdminFeatures.TESTMODE_GCF);
            sAdminFeatures.put(c.getString(R.string.ims_key_admin_testmode),
                    ProviderInterface.Subscriber.AdminFeatures.TESTMODE);
            sAdminFeatures.put(c.getString(R.string.ims_key_admin_debug),
                    ProviderInterface.Subscriber.AdminFeatures.DEBUG);
        }
    }

    public static int getBitmaskForAdminFeatures(String key) {
        Integer value = sAdminFeatures.get(key);
        return (value == null) ? 0 : value.intValue();
    }

    public static void setSummary(Preference preference, Object newValue) {
        if ( (preference == null) || (newValue == null) ) {
            return;
        }

        if (preference instanceof CheckBoxPreference) {

            CheckBoxPreference checkbox = (CheckBoxPreference)preference;
            String value = newValue.toString();

            checkbox.setChecked(Boolean.valueOf(value));
            checkbox.setSummary(value);
        }
        else if (preference instanceof EditTextPreference) {

            EditTextPreference editText = (EditTextPreference)preference;

            editText.setText(newValue.toString());
            editText.setSummary(newValue.toString());
        }
        else if (preference instanceof ListPreference) {

            ListPreference list = (ListPreference)preference;
            int valueIndex = list.findIndexOfValue(newValue.toString());

            if (valueIndex >= 0) {
                list.setValueIndex(valueIndex);
                CharSequence[] entries = list.getEntries();
                list.setSummary(entries[valueIndex]);
            }
        }
    }

    public static void displayColumns(String[] columns, int slotID) {
        SubsInfoInterface subsInfo = AgentFactory.getInstance().getAgent(
                SubsInfoInterface.class, slotID);

        if (subsInfo == null) {
            return;
        }

        if (!subsInfo.isDebugEnabled()) {
            return;
        }

        if (columns == null) {
            return;
        }

        for (int i = 0; i <  columns.length; ++i) {
            if (columns[i] == null) {
                ImsLog.d("column at index (" + i + ") is null");
                continue;
            }

            ImsLog.v("Column at index (" + i + ")" + columns[i]);
        }
    }

    public static Cursor getSettingsTable(SQLiteDatabase db, String tableName) {
        if (db == null) {
            return null;
        }

        if (tableName == null) {
            return null;
        }

        Cursor cursor = null;

        try {
            cursor = db.rawQuery("select * from " + tableName, null);
        } catch (SQLiteException e) {
            e.printStackTrace();

            if (cursor != null) {
                cursor.close();
                cursor = null;
            }
        }

        return cursor;
    }

    public static int getColumnIndex(String columns[], String column) {
        for (int i = 0; i <  columns.length; ++i) {
            if (column.equalsIgnoreCase(columns[i])) {
                return i;
            }
        }

        ImsLog.w(column + " is not exist on table");
        return (-1);
    }

    /**
     * @param value on/off value for test mode (0: disable or 1: enable)
     */
    public static void setTestModeForApp(Context context, int value) {
        updateCallSetting(context, null, value, SettingsUtils.CallSettings.KEY_IMS_ADMIN_TESTMODE);
    }

    public static void updateCallSetting(Context context,
            String value_str, int value_int, String key) {
        ImsLog.d("updateCallSetting :: " + key
                + " - value_str=" + value_str + ", value_int=" + value_int);

        ContentResolver cr = context.getContentResolver();
        String selection = SettingsUtils.CallSettings.KEY_NAME + "=" + "'" + key + "'";
        ContentValues cv = new ContentValues();

        cv.put(SettingsUtils.CallSettings.KEY_VALUE_STR, (value_str == null) ? "" : value_str);
        cv.put(SettingsUtils.CallSettings.KEY_VALUE_INT, value_int);

        try {
            int result = cr.update(SettingsUtils.CallSettings.CONTENT_URI, cv, selection, null);
            ImsLog.d("updateCallSetting :: result=" + result);
        } catch (Throwable t) {
            ImsLog.e("updateCallSetting :: " + t.toString());
            t.printStackTrace();
        }
    }
}
