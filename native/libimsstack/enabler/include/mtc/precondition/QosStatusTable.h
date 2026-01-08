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

#ifndef QOS_STATUS_TABLE_H_
#define QOS_STATUS_TABLE_H_

#include "ImsList.h"
#include "SdpAttribute.h"
#include "SdpMedia.h"
#include "offeranswer/SdpPrecondition.h"

class IMedia;
class IMediaDescriptor;

class QosStatusRecord
{
public:
    inline QosStatusRecord() :
            eSdpMediaType(SdpMedia::TYPE_INVALID),
            eAttrType(SdpAttribute::ATTRIBUTE_INVALID),
            eStatusType(SdpPrecondition::STATUS_INVALID),
            eDirTag(SdpPrecondition::DIRECTION_NONE),
            eStrengthTag(SdpPrecondition::STRENGTH_NOTUSED),
            bDesiredCheck(IMS_FALSE),
            bLocalResourceConfirmed(IMS_FALSE)
    {
    }

    inline QosStatusRecord(IN IMS_SINT32 _eSdpMediaType, IN IMS_SINT32 _eAttrType,
            IN IMS_SINT32 _eStatusType, IN IMS_SINT32 _eDirTag, IN IMS_SINT32 _eStrengthTag) :
            eSdpMediaType(_eSdpMediaType),
            eAttrType(_eAttrType),
            eStatusType(_eStatusType),
            eDirTag(_eDirTag),
            eStrengthTag(_eStrengthTag),
            bDesiredCheck(IMS_FALSE),
            bLocalResourceConfirmed(IMS_FALSE)
    {
    }

    inline ~QosStatusRecord() {}

public:
    inline QosStatusRecord& operator=(IN const QosStatusRecord& objRhs)
    {
        if (this != &objRhs)
        {
            eSdpMediaType = objRhs.eSdpMediaType;
            eAttrType = objRhs.eAttrType;
            eStatusType = objRhs.eStatusType;
            eDirTag = objRhs.eDirTag;
            eStrengthTag = objRhs.eStrengthTag;
            bDesiredCheck = objRhs.bDesiredCheck;
            bLocalResourceConfirmed = objRhs.bLocalResourceConfirmed;
        }
        return (*this);
    }

    IMS_BOOL operator==(const QosStatusRecord& objRhs) const
    {
        if (this == &objRhs)
        {
            return IMS_TRUE;
        }

        return eSdpMediaType == objRhs.eSdpMediaType && eAttrType == objRhs.eAttrType &&
                eStatusType == objRhs.eStatusType && eDirTag == objRhs.eDirTag &&
                eStrengthTag == objRhs.eStrengthTag && bDesiredCheck == objRhs.bDesiredCheck &&
                bLocalResourceConfirmed == objRhs.bLocalResourceConfirmed;
    }

public:
    /* Definition of SDPMedia.h
        TYPE_INVALID = (-1),
        TYPE_AUDIO = 0,
        TYPE_VIDEO,
        TYPE_TEXT,
    */
    IMS_SINT32 eSdpMediaType;

    /* Type of attribute - Definition of SDPAttribute.h
        // Extensions
        CURR = 18,                  //RFC 3312, Integration of Resource Management and SIP
        DES,                        //RFC 3312, Integration of Resource Management and SIP
        CONF,                       //RFC 3312, Integration of Resource Management and SIP
    */
    IMS_SINT32 eAttrType;

    /* Definition of SDPPrecondition.h
    // Type of "status-type"
        STATUS_LOCAL = 1,
        STATUS_REMOTE = 2,
    */
    IMS_SINT32 eStatusType;

    /* Definition of SDPPrecondition.h
    // Type of "direction-tag"
        DIRECTION_NONE = 0,
        DIRECTION_SEND,
        DIRECTION_RECV,
        DIRECTION_SENDRECV,
    */
    IMS_SINT32 eDirTag;

    /* Definition of SDPPrecondition.h
    // Type of "strength-tag"
        STRENGTH_MANDATORY = 0,
        STRENGTH_OPTIONAL,
        STRENGTH_NONE,
        STRENGTH_FAILURE,
        STRENGTH_UNKNOWN,
        STRENGTH_NOTUSED,
    */
    IMS_SINT32 eStrengthTag;

    // Whether if Desired Status is checked by remote view or not.
    // Only used for SDPAttribute::DES type.
    IMS_BOOL bDesiredCheck;

    // Only used for SDPAttribute::CUR type.
    IMS_BOOL bLocalResourceConfirmed;
};

class QosStatusTable
{
public:
    QosStatusTable();
    virtual ~QosStatusTable();

private:
    QosStatusTable(IN const QosStatusTable& objRhs);
    QosStatusTable& operator=(IN const QosStatusTable& objRhs);

public:
    /**
     * @brief Gets the records that match the given parameter.
     *
     * @param eSdpMediaType TYPE_AUDIO, TYPE_VIDEO, TYPE_TEXT of SdpMedia.h
     * @return The list of the records.
     */
    virtual ImsList<QosStatusRecord*> GetRecords(IN IMS_SINT32 eSdpMediaType) const;

    /**
     * @brief Clears current records and sets initial records of the given media.
     *
     * | Status | Status Type | Direction tag | Strength tag |
     * | ------ | ----------- | ------------- | ------------ |
     * | curr   | local       | none          | (not used)   |
     * | curr   | remote      | none          | (not used)   |
     * | des    | local       | send          | mandatory    |
     * | des    | local       | recv          | mandatory    |
     * | des    | remote      | send          | optional     |
     * | des    | remote      | recv          | optional     |
     * | conf   | remote      | sendrecv      | (not used)   |
     *
     * @param eSdpMediaType TYPE_AUDIO, TYPE_VIDEO, TYPE_TEXT of SdpMedia.h
     */
    virtual void InitializeRecords(IN IMS_SINT32 eSdpMediaType);

    /**
     * @brief Removes records that are unused.
     *
     * @param eMediaTypes Media types that are currently using.
     *                    Flag combination of MEDIATYPE_AUDIO, MEDIATYPE_VIDEO, MEDIATYPE_TEXT.
     */
    virtual void RemoveUnusedRecords(IN IMS_UINT32 eMediaTypes);

    virtual void UpdateStatusTableWithRemoteSdp(IN const IMedia& objMedia);

    virtual void UpdateLocalCurrentStatus(
            IN IMS_SINT32 eSdpMediaType, IN IMS_BOOL bLocalQosEnabled);
    virtual void EnableRemoteCurrentStatus(IN IMS_SINT32 eSdpMediaType);
    virtual IMS_BOOL IsCurrentStatusEnabled(IN IMS_SINT32 eSdpMediaType, IN IMS_SINT32 eStatusType);

    virtual IMS_SINT32 GetDirectionTag(
            IN IMS_SINT32 eSdpMediaType, IN IMS_SINT32 eAttrType, IN IMS_SINT32 eStatusType);
    virtual void SetDirectionTag(IN IMS_SINT32 eSdpMediaType, IN IMS_SINT32 eAttrType,
            IN IMS_SINT32 eStatusType, IN IMS_SINT32 eDirTag);

    virtual IMS_SINT32 GetStrengthTag(
            IN IMS_SINT32 eSdpMediaType, IN IMS_SINT32 eStatusType, IN IMS_SINT32 eDirTag);
    virtual void SetStrengthTag(IN IMS_SINT32 eSdpMediaType, IN IMS_SINT32 eStatusType,
            IN IMS_SINT32 eDirTag, IN IMS_SINT32 eStrengthTag);

    virtual IMS_BOOL IsLocalResourceConfirmed(IN IMS_SINT32 eSdpMediaType);
    virtual void SetLocalResourceConfirmed(IN IMS_SINT32 eSdpMediaType, IN IMS_BOOL bConfirmed);

private:
    /**
     * @brief Clears the records that match the given media.
     *
     * @param eSdpMediaType TYPE_AUDIO, TYPE_VIDEO, TYPE_TEXT of SdpMedia.h
     */
    void ClearRecords(IN IMS_SINT32 eSdpMediaType);

    /**
     * @brief Gets the records that match the given parameter.
     *
     * @param eSdpMediaType TYPE_AUDIO, TYPE_VIDEO, TYPE_TEXT of SdpMedia.h
     * @param eAttrType CURR, DES, CONF of SdpAttribute.h
     * @param eStatusType STATUS_LOCAL, STATUS_REMOTE of SdpPrecondition.h
     * @param eDirTag DIRECTION_NONE, DIRECTION_SEND, DIRECTION_RECV,
     *                DIRECTION_SENDRECV of SdpPrecondition.h
     *                Don't care if DIRECTION_INVALID.
     * @return The list of the records.
     */
    ImsList<QosStatusRecord*> GetRecords(IN IMS_SINT32 eSdpMediaType, IN IMS_SINT32 eAttrType,
            IN IMS_SINT32 eStatusType,
            IN IMS_SINT32 eDirTag = SdpPrecondition::DIRECTION_INVALID) const;

    ImsList<QosStatusRecord*>* GetRecordsRef(IN IMS_SINT32 eSdpMediaType);

    void InitializeDesChecked(IN IMS_SINT32 eSdpMediaType);

    void UpdateCurrentStatus(
            IN const IMediaDescriptor* piMediaDescriptor, IN IMS_SINT32 eSdpMediaType);
    void UpdateDesiredStatus(
            IN const IMediaDescriptor* piMediaDescriptor, IN IMS_SINT32 eSdpMediaType);

private:
    ImsList<QosStatusRecord*> m_lstAudioRecords;
    ImsList<QosStatusRecord*> m_lstVideoRecords;
    ImsList<QosStatusRecord*> m_lstTextRecords;
};

#endif
