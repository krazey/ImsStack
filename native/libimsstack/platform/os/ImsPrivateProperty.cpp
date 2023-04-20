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
#include "ImsPrivateProperty.h"
#include "PlatformApi.h"
#include "ServiceMemory.h"

PUBLIC GLOBAL const IMS_CHAR ImsPrivateProperties::Ephemeral::KEY_KR_TTA_VERSION[] =
        "kr_tta_version";
PUBLIC GLOBAL const IMS_CHAR ImsPrivateProperties::Ephemeral::KEY_REG_0_BT[] = "reg_0_bt";
PUBLIC GLOBAL const IMS_CHAR ImsPrivateProperties::Ephemeral::KEY_REG_1_MT[] = "reg_1_mt";
PUBLIC GLOBAL const IMS_CHAR ImsPrivateProperties::Ephemeral::KEY_REG_2_CF[] = "reg_2_cf";
PUBLIC GLOBAL const IMS_CHAR ImsPrivateProperties::Ephemeral::KEY_REG_3_UWT[] = "reg_3_uwt";
PUBLIC GLOBAL const IMS_CHAR ImsPrivateProperties::Ephemeral::KEY_REG_4_AWT[] = "reg_4_awt";
PUBLIC GLOBAL const IMS_CHAR ImsPrivateProperties::Ephemeral::KEY_SMS_NETWORK_REG_BIND[] =
        "sms_network_reg_bind";
PUBLIC GLOBAL const IMS_CHAR
        ImsPrivateProperties::Ephemeral::KEY_THIRD_PARTY_DIALER_FOR_VIDEO_CALL[] =
                "third_party_dialer_for_video_call";

PUBLIC GLOBAL const IMS_CHAR ImsPrivateProperties::Persistent::KEY_SHOW_CODEC_INFO[] =
        "show_codec_info";
PUBLIC GLOBAL const IMS_CHAR ImsPrivateProperties::Persistent::KEY_WIFI_TEST[] = "wifi_test";
PUBLIC GLOBAL const IMS_CHAR ImsPrivateProperties::Persistent::KEY_CARRIER_SIGNAL_PCO_TEST[] =
        "carrier_signal_pco_test";
PUBLIC GLOBAL const IMS_CHAR ImsPrivateProperties::Persistent::KEY_SIP_DEVICE_ID[] =
        "sip_device_id";
PUBLIC GLOBAL const IMS_CHAR ImsPrivateProperties::Persistent::KEY_SIM_OPERATOR[] = "sim_operator";
PUBLIC GLOBAL const IMS_CHAR ImsPrivateProperties::Persistent::KEY_SIM_COUNTRY[] = "sim_country";
PUBLIC GLOBAL const IMS_CHAR ImsPrivateProperties::Persistent::KEY_SIM_OPERATOR_SUB[] =
        "sim_operator_sub";

PUBLIC GLOBAL const IMS_CHAR ImsPrivateProperties::Persistent::KEY_ISIM_ENABLED[] = "isim_enabled";
PUBLIC GLOBAL const IMS_CHAR ImsPrivateProperties::Persistent::KEY_USIM_ENABLED[] = "usim_enabled";
PUBLIC GLOBAL const IMS_CHAR ImsPrivateProperties::Persistent::KEY_PRIMARY_IMPU[] = "primary_impu";

PUBLIC GLOBAL const IMS_CHAR ImsPrivateProperties::Persistent::KEY_CONFIG_PCSCF_ADDRESS_LIST[] =
        "config_pcscf_address_list";
PUBLIC GLOBAL const IMS_CHAR ImsPrivateProperties::Persistent::KEY_CONFIG_IMPI[] = "config_impi";
PUBLIC GLOBAL const IMS_CHAR ImsPrivateProperties::Persistent::KEY_CONFIG_IMPU_LIST[] =
        "config_impu_list";
PUBLIC GLOBAL const IMS_CHAR ImsPrivateProperties::Persistent::KEY_CONFIG_HOME_DOMAIN_NAME[] =
        "config_home_domain_name";
PUBLIC GLOBAL const IMS_CHAR ImsPrivateProperties::Persistent::KEY_TEST_IMS_DEREGISTER[] =
        "test_ims_deregister";
PUBLIC GLOBAL const IMS_CHAR ImsPrivateProperties::Persistent::KEY_TEST_LOG_OPTIONS[] =
        "test_log_options";

PRIVATE
ImsPrivateProperty::ImsPrivateProperty() {}

PRIVATE
ImsPrivateProperty::~ImsPrivateProperty() {}

PUBLIC GLOBAL ImsPrivateProperty* ImsPrivateProperty::GetInstance()
{
    static ImsPrivateProperty* s_pProperty = IMS_NULL;

    if (s_pProperty == IMS_NULL)
    {
        s_pProperty = new ImsPrivateProperty();
    }

    return s_pProperty;
}

PUBLIC VIRTUAL AString ImsPrivateProperty::Get(IN const AString& strKey, IN IMS_SINT32 nSlotId)
{
    return PlatformApi::GetPrivateProperty(IMS_FALSE, strKey, nSlotId);
}

PUBLIC VIRTUAL IMS_BOOL ImsPrivateProperty::GetBoolean(
        IN const AString& strKey, IN IMS_SINT32 nSlotId)
{
    AString strValue = Get(strKey, nSlotId);
    return strValue.EqualsIgnoreCase("true");
}

PUBLIC VIRTUAL IMS_SINT32 ImsPrivateProperty::GetInt(
        IN const AString& strKey, IN IMS_SINT32 nSlotId)
{
    AString strValue = Get(strKey, nSlotId);
    IMS_BOOL bOK = IMS_FALSE;
    IMS_SINT32 nValue = strValue.ToInt32(&bOK);

    return bOK ? nValue : (-1);
}

PUBLIC VIRTUAL void ImsPrivateProperty::Set(
        IN const AString& strKey, IN const AString& strValue, IN IMS_SINT32 nSlotId)
{
    PlatformApi::SetPrivateProperty(IMS_FALSE, strKey, strValue, nSlotId);
}

PUBLIC VIRTUAL void ImsPrivateProperty::SetBoolean(
        IN const AString& strKey, IN IMS_BOOL bValue, IN IMS_SINT32 nSlotId)
{
    AString strValue = bValue ? "true" : "false";

    Set(strKey, strValue, nSlotId);
}

PUBLIC VIRTUAL void ImsPrivateProperty::SetInt(
        IN const AString& strKey, IN IMS_SINT32 nValue, IN IMS_SINT32 nSlotId)
{
    AString strValue;
    strValue.SetNumber(nValue);

    Set(strKey, strValue, nSlotId);
}

PUBLIC VIRTUAL AString ImsPrivateProperty::GetPersistent(
        IN const AString& strKey, IN IMS_SINT32 nSlotId)
{
    return PlatformApi::GetPrivateProperty(IMS_TRUE, strKey, nSlotId);
}

PUBLIC VIRTUAL IMS_BOOL ImsPrivateProperty::GetPersistentBoolean(
        IN const AString& strKey, IN IMS_SINT32 nSlotId)
{
    AString strValue = GetPersistent(strKey, nSlotId);
    return strValue.EqualsIgnoreCase("true");
}

PUBLIC VIRTUAL IMS_SINT32 ImsPrivateProperty::GetPersistentInt(
        IN const AString& strKey, IN IMS_SINT32 nSlotId)
{
    AString strValue = GetPersistent(strKey, nSlotId);
    IMS_BOOL bOK = IMS_FALSE;
    IMS_SINT32 nValue = strValue.ToInt32(&bOK);

    return bOK ? nValue : (-1);
}

PUBLIC VIRTUAL void ImsPrivateProperty::SetPersistent(
        IN const AString& strKey, IN const AString& strValue, IN IMS_SINT32 nSlotId)
{
    PlatformApi::SetPrivateProperty(IMS_TRUE, strKey, strValue, nSlotId);
}

PUBLIC VIRTUAL void ImsPrivateProperty::SetPersistentBoolean(
        IN const AString& strKey, IN IMS_BOOL bValue, IN IMS_SINT32 nSlotId)
{
    AString strValue = bValue ? "true" : "false";

    SetPersistent(strKey, strValue, nSlotId);
}

PUBLIC VIRTUAL void ImsPrivateProperty::SetPersistentInt(
        IN const AString& strKey, IN IMS_SINT32 nValue, IN IMS_SINT32 nSlotId)
{
    AString strValue;
    strValue.SetNumber(nValue);

    SetPersistent(strKey, strValue, nSlotId);
}
