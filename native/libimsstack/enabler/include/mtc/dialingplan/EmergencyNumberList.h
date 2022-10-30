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

#ifndef EMERGENCY_NUMBER_LIST_H_
#define EMERGENCY_NUMBER_LIST_H_

#include "AString.h"
#include "ImsMap.h"

class AStringBuffer;

class EmergencyNumberList
{
public:
    EmergencyNumberList(IN IMS_SINT32 nSlotID);
    virtual ~EmergencyNumberList();

private:
    EmergencyNumberList(IN const EmergencyNumberList& objRHS);
    EmergencyNumberList& operator=(IN const EmergencyNumberList& objRHS);

public:
    AString GetEmergencyServiceURN(IN const AString& strNumber);
    // will be deleted_S
    IMS_SINT32 GetEmergencyServiceCategory(IN const AString& strNumber);
    // will be deleted_E

private:
    IMS_SINT32 GetEmergencyServiceCategory(IN const AString& strNumber, OUT IMS_SINT32& nSource);
    IMS_SINT32 GetEmergencyServiceCategoryEx(
            IN const AString& strNumber, IN IMS_SINT32 nSource = ENL_NETWORK);
    IMS_SINT32 GetPrimaryENLSource() const;
    AString HandleENLForNetwork(IN const AString& strNumber, IN IMS_SINT32 nESCV);
    AString HandleENLForUICC(IN const AString& strNumber, IN IMS_SINT32 nESCV);
    AString HandleNoENL(IN const AString& strNumber);
    IMS_BOOL GetENL(OUT AStringBuffer& objENL, IN IMS_SINT32 nSource = ENL_NETWORK);
    IMS_SINT32 GetESCV(IN const AString& strNumber, IN const AString& strENL);
    AString GetOperatorConfigURN(IN const AString& strNumber);
    IMS_RESULT ReadOperatorConfig();
    AString TranslateAsEmergencyServiceURN(IN IMS_SINT32 nESCV);
    const IMS_CHAR* ConvertSourceToString(IN IMS_SINT32 nSource);
    IMS_BOOL IsMultipleESCV(IN IMS_SINT32 nESCV);

public:
    // Emergency Service Category Value
    enum
    {
        ESCV_INVALID = (-1),
        ESCV_NONE = 0,
        ESCV_POLICE = 0x00000001,
        ESCV_AMBULANCE = 0x00000002,
        ESCV_FIREBRIGADE = 0x00000004,
        ESCV_MARINEGUARD = 0x00000008,
        ESCV_MOUNTAINRESCUE = 0x00000010
    };

    // Source of the emergency number list
    enum
    {
        ENL_UICC = 0,
        ENL_NETWORK = 1
    };

    enum
    {
        URN_MAPPING_TYPE_NONE = 0,
        URN_MAPPING_TYPE_3GPP_STANDARD,  // 24.229 : the UE shall map any one of multiple ESCV.
        URN_MAPPING_TYPE_GENERIC_ONLY,   // use generic sos urn regardless of ESCV
        URN_MAPPING_TYPE_GENERIC_MULTIPLE_ESCV  // use generic sos urn  when  ESCV is multiple.
    };

private:
    IMS_SINT32 m_nSlotID;
    IMS_SINT32 m_nURNMappingType;
    IMSMap<AString, AString> m_objEMCConfigStrMap;
};

#endif
