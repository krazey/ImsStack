

#ifndef UCE_NOTIFY_MESSAGE_BODY_H_
#define UCE_NOTIFY_MESSAGE_BODY_H_

#include "AString.h"
#include "UceNotifyBodyPartData.h"

class UceNotifyMessageBody {
 public:
  UceNotifyMessageBody() { m_strContentType = AString::ConstNull(); }
  virtual ~UceNotifyMessageBody() {
    for (IMS_UINT32 i = 0; i < m_objNotifyBodyPartDatas.GetSize(); ++i) {
      UceNotifyBodyPartData* pData = m_objNotifyBodyPartDatas.GetAt(i);
      if (pData != IMS_NULL) {
        delete pData;
        pData = IMS_NULL;
      }
    }
    m_objNotifyBodyPartDatas.Clear();
  }

  void SetContentType(IN AString& strContentType) {
    m_strContentType = strContentType;
  }
  void SetNotifyBodyPartData(IN UceNotifyBodyPartData* pData) {
    m_objNotifyBodyPartDatas.Append(pData);
  }
  AString& GetContentType() { return m_strContentType; }
  IMSList<UceNotifyBodyPartData*> GetNotifyBodyPartDatas() {
    return m_objNotifyBodyPartDatas;
  }

 private:
  AString m_strContentType;
  IMSList<UceNotifyBodyPartData*> m_objNotifyBodyPartDatas;
};
#endif /* UCE_NOTIFY_MESSAGE_BODY_H_ */
