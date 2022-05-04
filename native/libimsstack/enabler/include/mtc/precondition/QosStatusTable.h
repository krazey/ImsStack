#ifndef QOS_STATUS_TABLE_H_
#define QOS_STATUS_TABLE_H_

#include "SdpMedia.h"
#include "SdpAttribute.h"
#include "offeranswer/SdpPrecondition.h"

class QosStatusRecord
{
public:
    inline QosStatusRecord() :
            eSdpMediaType(SdpMedia::TYPE_INVALID),
            eAttrType(SdpAttribute::ATTRIBUTE_INVALID),
            eStatusType(SdpPrecondition::STATUS_INVALID),
            eDirTag(SdpPrecondition::DIRECTION_NONE),
            eStrengthTag(SdpPrecondition::STRENGTH_NOTUSED),
            bDesiredCheck(IMS_FALSE)
    {
    }

    inline QosStatusRecord(IN IMS_SINT32 _eSdpMediaType, IN IMS_SINT32 _eAttrType,
            IN IMS_SINT32 _eStatusType, IN IMS_SINT32 _eDirTag, IN IMS_SINT32 _eStrengthTag) :
            eSdpMediaType(_eSdpMediaType),
            eAttrType(_eAttrType),
            eStatusType(_eStatusType),
            eDirTag(_eDirTag),
            eStrengthTag(_eStrengthTag),
            bDesiredCheck(IMS_FALSE)
    {
    }

    inline ~QosStatusRecord() {}

public:
    inline QosStatusRecord& operator=(IN const QosStatusRecord& objRHS)
    {
        if (this != &objRHS)
        {
            eSdpMediaType = objRHS.eSdpMediaType;
            eAttrType = objRHS.eAttrType;
            eStatusType = objRHS.eStatusType;
            eDirTag = objRHS.eDirTag;
            eStrengthTag = objRHS.eStrengthTag;
            bDesiredCheck = objRHS.bDesiredCheck;
        }
        return (*this);
    }

public:
    /* Definition of SDPMedia.h
        TYPE_INVALID = (-1),
        TYPE_AUDIO = 0,
        TYPE_VIDEO,
        TYPE_TEXT,
    */
    IMS_SINT32 eSdpMediaType;
    /*/ Type of attribute - Definition of SDPAttribute.h
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
    // whether if Desired Status is checked by remote view or not.
    IMS_BOOL bDesiredCheck;
};

class QosStatusTable
{
public:
    QosStatusTable();
    ~QosStatusTable();

private:
    QosStatusTable(IN CONST QosStatusTable& objRHS);
    QosStatusTable& operator=(IN CONST QosStatusTable& objRHS);

public:
    void UpdateStatusTableWithRemoteSdp(IN IMedia* piMedia);

    void UpdateLocalCurrentStatus(IN IMS_SINT32 eSdpMediaType, IN IMS_BOOL bLocalQoSEnabled);
    void EnableRemoteCurrentStatus(IN IMS_SINT32 eSdpMediaType);
    IMS_BOOL IsCurrentStatusEnabled(IN IMS_SINT32 eSdpMediaType, IN IMS_SINT32 eStatusType);

    IMS_SINT32 GetDirectionTag(
            IN IMS_SINT32 eSdpMediaType, IN IMS_SINT32 eAttrType, IN IMS_SINT32 eStatusType);
    IMS_SINT32 GetStrengthTag(
            IN IMS_SINT32 eSdpMediaType, IN IMS_SINT32 eStatusType, IN IMS_SINT32 eDirTag);
    void SetDirectionTag(IN IMS_SINT32 eSdpMediaType, IN IMS_SINT32 eAttrType,
            IN IMS_SINT32 eStatusType, IN IMS_SINT32 eDirTag);
    void SetStrengthTag(IN IMS_SINT32 eSdpMediaType, IN IMS_SINT32 eStatusType,
            IN IMS_SINT32 eDirTag, IN IMS_SINT32 eStrengthTag);

    void CreateStatusRecords(IN IMS_SINT32 eSdpMediaType);
    IMS_BOOL IsStatusRecordsListEmpty(IN IMS_SINT32 eSdpMediaType);
    void RemoveUnusedStatusRecords(IN IMS_UINT32 eMediaTypes);

private:
    void AddStatusRecord(IN IMS_SINT32 eSdpMediaType);
    void InitializeDesChecked(IN IMS_SINT32 eSdpMediaType);

    void UpdateCurrentStatus(IN IMediaDescriptor* piMediaDescriptor, IN IMS_SINT32 eSdpMediaType);
    void UpdateDesiredStatus(IN IMediaDescriptor* piMediaDescriptor, IN IMS_SINT32 eSdpMediaType);

    IMSList<QosStatusRecord*>& GetStatusRecords(IN IMS_SINT32 eSdpMediaType);
    IMSList<QosStatusRecord*> GetStatusRecords(IN IMS_SINT32 eSdpMediaType, IN IMS_SINT32 eAttrType,
            IN IMS_SINT32 eStatusType, IN IMS_SINT32 eDirTag = SdpPrecondition::DIRECTION_NONE);
    void ClearStatusRecords(IN IMSList<QosStatusRecord*>& lstRecords);

private:
    IMSList<QosStatusRecord*> m_lstAudioRecords;
    IMSList<QosStatusRecord*> m_lstVideoRecords;
    IMSList<QosStatusRecord*> m_lstTextRecords;
};
#endif
