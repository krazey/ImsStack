

#ifndef UCE_NOTIFY_BODY_PART_DATA_H_
#define UCE_NOTIFY_BODY_PART_DATA_H_

#include "AString.h"
#include "ByteArray.h"

class UceNotifyBodyPartData {
 public:
  UceNotifyBodyPartData(AString& strContentType, AString& strContentId,
                        ByteArray& objBody) {
    m_strContentType = strContentType;
    m_strContentId = strContentId;
    m_pBodyContent.Append(objBody);
  }
  virtual ~UceNotifyBodyPartData() {}

  AString& GetContentType() { return m_strContentType; }
  AString& GetContentId() { return m_strContentId; }
  ByteArray& GetBodyContent() { return m_pBodyContent; }

 private:
  AString m_strContentType;
  AString m_strContentId;
  ByteArray m_pBodyContent;
};

#endif /* UCE_NOTIFY_BODY_PART_DATA_H_ */
