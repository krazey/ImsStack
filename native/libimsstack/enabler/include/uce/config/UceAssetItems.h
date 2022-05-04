#ifndef UCE_ASSET_ITEMS_H_
#define UCE_ASSET_ITEMS_H_

#include "AString.h"
#include "IMSTypeDef.h"
#include "IMSVector.h"

class UceAssetItems {
 public:
  UceAssetItems()
      : m_nExpireValuePublish(0),
        m_nExtendedExpireValuePublish(0),
        m_nPublishRefreshRatio(80),
        m_nExpireValueListSubscribe(60),
        m_strRlsUri("pres-list=dummy-rfc5367"),
        m_bSubscribeIndepedentOfPublish(IMS_FALSE),
        m_nAnonymousFetchMethod(2),
        m_bEncodePublishBody(IMS_FALSE),
        m_bEncodeSubscribeBody(IMS_FALSE),
        m_bUseContactHeaderInPublish(IMS_FALSE),
        m_bUseContactHeaderInSubscribe(IMS_FALSE),
        m_bAddVideoTagContactHeaderInPublish(IMS_FALSE),
        m_objImmediatelyRetryPublishResponse(IMSVector<IMS_SINT32>()),
        m_nImmediatelyRetryPublishResponseMaxCount(0),
        m_objRetryPublishResponse(IMSVector<IMS_SINT32>()),
        m_nRetryPublishResponseMaxCount(0),
        m_nRetryPublishResponseTimeSec(0),
        m_objExponentialRetryPublishResponse(IMSVector<IMS_SINT32>()),
        m_nExponentialRetryPublishResponseMaxCount(0),
        m_objExponentialRetryPublishResponseTimeSec(IMSVector<IMS_SINT32>()),
        m_objReAttemptRegistrationPublishResponse(IMSVector<IMS_SINT32>()),
        m_objReAttemptRegistrationSubscribeResponse(IMSVector<IMS_SINT32>()) {
    m_objImmediatelyRetryPublishResponse.Push(412);

    m_objRetryPublishResponse.Push(0);

    m_objExponentialRetryPublishResponse.Push(0);

    m_objExponentialRetryPublishResponseTimeSec.Push(0);

    m_objReAttemptRegistrationPublishResponse.Push(403);

    m_objReAttemptRegistrationSubscribeResponse.Push(403);
  }

 public:
  IMS_UINT32 m_nExpireValuePublish;
  IMS_UINT32 m_nExtendedExpireValuePublish;
  IMS_UINT32 m_nPublishRefreshRatio;
  IMS_UINT32 m_nExpireValueListSubscribe;
  AString m_strRlsUri;
  IMS_BOOL m_bSubscribeIndepedentOfPublish;
  IMS_UINT32 m_nAnonymousFetchMethod;
  IMS_BOOL m_bEncodePublishBody;
  IMS_BOOL m_bEncodeSubscribeBody;
  IMS_BOOL m_bUseContactHeaderInPublish;
  IMS_BOOL m_bUseContactHeaderInSubscribe;
  IMS_BOOL m_bAddVideoTagContactHeaderInPublish;
  IMSVector<IMS_SINT32> m_objImmediatelyRetryPublishResponse;
  IMS_UINT32 m_nImmediatelyRetryPublishResponseMaxCount;
  IMSVector<IMS_SINT32> m_objRetryPublishResponse;
  IMS_UINT32 m_nRetryPublishResponseMaxCount;
  IMS_UINT32 m_nRetryPublishResponseTimeSec;
  IMSVector<IMS_SINT32> m_objExponentialRetryPublishResponse;
  IMS_UINT32 m_nExponentialRetryPublishResponseMaxCount;
  IMSVector<IMS_SINT32> m_objExponentialRetryPublishResponseTimeSec;
  IMSVector<IMS_SINT32> m_objReAttemptRegistrationPublishResponse;
  IMSVector<IMS_SINT32> m_objReAttemptRegistrationSubscribeResponse;
};
#endif
