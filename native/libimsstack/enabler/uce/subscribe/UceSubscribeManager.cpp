/*
   Author
   <table>
   date      author                            description
   --------  --------------                ----------
   20111213  saurabh31.srivastava@              Created
   20121106  hyunho.shin@                       Re-Factorying
   20130820  jaesik.kong@                       Re-Factorying for one source
   </table>

   Description - EABSubscribeMngr.cpp

*/

#include "subscribe/UceSubscribeManager.h"

#include <stdlib.h>

#include "ICoreService.h"
#include "IMessage.h"
#include "IUUceService.h"
#include "ServiceTrace.h"
#include "subscribe/UceSubscribe.h"

__IMS_TRACE_TAG_USER_DECL__("UCE");
/* -------------------------------------------------------------------------------------------------
    Constructor, Destructor, Operator Overloading
-------------------------------------------------------------------------------------------------
*/
PUBLIC
UceSubscribeManager::UceSubscribeManager(IN CONST AString& strName,
                                         ICoreService* _piCoreService,
                                         IN CONST AString& strAppName,
                                         IN IMS_SINT32 nSimSlot)
    : IMSActivityEx(strName),
      m_piCoreService(_piCoreService),
      m_strAppName(strAppName),
      m_nSimSlot(nSimSlot),
      m_nConnectedServices(0) {
  IMS_TRACE_D("UCE_M : UceSubscribeManager = %" PFLS_u,
              sizeof(UceSubscribeManager), 0, 0);
  IMS_TRACE_I("UceSubscribeManager", 0, 0, 0);
}

PUBLIC VIRTUAL UceSubscribeManager::~UceSubscribeManager() {
  IMS_TRACE_D("UCE_F : UceSubscribeManager = %" PFLS_u,
              sizeof(UceSubscribeManager), 0, 0);
  IMS_TRACE_I("~UceSubscribeManager", 0, 0, 0);

  ClearSubscribeList();
}
/* -------------------------------------------------------------------------------------------------
    Methods
-------------------------------------------------------------------------------------------------
*/
PUBLIC
IMS_BOOL UceSubscribeManager::QuerySingleCapability(IN AString strUser,
                                                    IN IMS_UINT32 key) {
  UceSubscribe* pUceSubscribe =
      new UceSubscribe(m_piCoreService, m_strAppName, GetName(),
                       m_nConnectedServices, m_nSimSlot);
  if (pUceSubscribe == IMS_NULL) {
    IMS_TRACE_I("QuerySingleCapability:UceSubscribe create failed", 0, 0, 0);
    return IMS_FALSE;
  }
  m_objUceSubscribeList.Append(pUceSubscribe);
  pUceSubscribe->QuerySingleCapability(strUser, key);
  return IMS_TRUE;
}

IMS_BOOL UceSubscribeManager::QueryMultiCapability(IN IMSList<AString> objUsers,
                                                   IN IMS_UINT32 key) {
  UceSubscribe* pUceSubscribe =
      new UceSubscribe(m_piCoreService, m_strAppName, GetName(),
                       m_nConnectedServices, m_nSimSlot);
  if (pUceSubscribe == IMS_NULL) {
    IMS_TRACE_I("QuerySingleCapability:UceSubscribe create failed", 0, 0, 0);
    return IMS_FALSE;
  }
  m_objUceSubscribeList.Append(pUceSubscribe);
  pUceSubscribe->QueryMultiCapability(objUsers, key);
  return IMS_TRUE;
}

IMS_BOOL UceSubscribeManager::AosConnected(IMS_UINT32 conectedService) {
  IMS_TRACE_D("AosConnected", 0, 0, 0);
  m_nConnectedServices = conectedService;
  return IMS_TRUE;
}

IMS_BOOL UceSubscribeManager::AosDisConnected() {
  IMS_TRACE_D("AosDisConnected", 0, 0, 0);
  if (m_objUceSubscribeList.IsEmpty()) {
    return IMS_TRUE;
  }

  UceSubscribe* pUceSubscribe = IMS_NULL;
  for (IMS_UINT32 i = 0; i < m_objUceSubscribeList.GetSize(); i++) {
    pUceSubscribe = (UceSubscribe*)m_objUceSubscribeList.GetAt(i);
    if (pUceSubscribe != IMS_NULL) {
      pUceSubscribe->AosDisConnected();
      pUceSubscribe = IMS_NULL;
    }
  }
  return IMS_TRUE;
}

void UceSubscribeManager::ClosedService() {
  IMS_TRACE_D("ClosedService", 0, 0, 0);
  if (m_objUceSubscribeList.IsEmpty()) {
    return;
  }

  UceSubscribe* pUceSubscribe = IMS_NULL;
  for (IMS_UINT32 i = 0; i < m_objUceSubscribeList.GetSize(); i++) {
    pUceSubscribe = (UceSubscribe*)m_objUceSubscribeList.GetAt(i);
    if (pUceSubscribe != IMS_NULL) {
      pUceSubscribe->AosDisConnected();
      pUceSubscribe = IMS_NULL;
    }
  }
}

PROTECTED VIRTUAL IMS_BOOL UceSubscribeManager::OnMessage(IN IMSMSG& objMsg) {
  if (objMsg.nMSG == IUUceService::UCE_SUBSCRIBE_DELETED_IND) {
    IMS_TRACE_D("UCE_SUBSCRIBE_DELETED_IND", 0, 0, 0);
    UceSubscribe* pUceSubscribe =
        reinterpret_cast<UceSubscribe*>(objMsg.nLparam);
    RemoveSubscribe(pUceSubscribe);
  }
  return IMS_TRUE;
}

PRIVATE
IMS_BOOL UceSubscribeManager::RemoveSubscribe(IN UceSubscribe* subscribe) {
  IMS_TRACE_D("RemoveSubscribe", 0, 0, 0);
  if (subscribe == IMS_NULL) {
    return IMS_FALSE;
  }

  for (IMS_UINT32 i = 0; i < m_objUceSubscribeList.GetSize(); i++) {
    if (m_objUceSubscribeList.GetAt(i) == subscribe) {
      IMS_TRACE_D("RemoveSubscribe:Find[%d]", i, 0, 0);
      delete subscribe;
      subscribe = IMS_NULL;
      m_objUceSubscribeList.RemoveAt(i);
      return IMS_TRUE;
    }
  }
  return IMS_TRUE;
}

PRIVATE
IMS_BOOL UceSubscribeManager::ClearSubscribeList() {
  IMS_TRACE_D("AosDisConnected", 0, 0, 0);
  if (m_objUceSubscribeList.IsEmpty()) {
    return IMS_TRUE;
  }

  UceSubscribe* pUceSubscribe = IMS_NULL;
  for (IMS_UINT32 i = 0; i < m_objUceSubscribeList.GetSize(); i++) {
    pUceSubscribe = (UceSubscribe*)m_objUceSubscribeList.GetAt(i);
    if (pUceSubscribe != IMS_NULL) {
      delete pUceSubscribe;
      pUceSubscribe = IMS_NULL;
    }
  }
  m_objUceSubscribeList.Clear();
  return IMS_TRUE;
}
