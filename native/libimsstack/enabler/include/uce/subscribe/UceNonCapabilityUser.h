

#ifndef UCE_NON_CAPABILITY_USER_H_
#define UCE_NON_CAPABILITY_USER_H_

#include "AString.h"

class UceNonCapabilityUser {
 public:
  UceNonCapabilityUser(AString& strId, AString& strReason) {
    m_strId = strId;
    m_strReason = strReason;
  }
  virtual ~UceNonCapabilityUser(){};

  AString& GetId() { return m_strId; }
  AString& GetReason() { return m_strReason; }

 private:
  AString m_strId;
  AString m_strReason;
};

#endif /* UCE_NON_CAPABILITY_USER_H_ */
