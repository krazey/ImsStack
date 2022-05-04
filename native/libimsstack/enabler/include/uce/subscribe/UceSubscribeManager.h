/*
    Author
    <table>
    date      author                        description
            Saurabh Srivastava
    --------  --------------                ----------
    20120525  saurabh31.srivastava@           Created
    20121106  hyunho.shin@                   Re-Factorying
    20130820  jaesik.kong@                   Re-Factorying for one source
    </table>

    Description - UceSubscribeMngr.h

*/

#ifndef _UCE_SUBSCRIBE_MANAGER_H_
#define _UCE_SUBSCRIBE_MANAGER_H_

#include "IMSActivityEx.h"

class ICoreService;
class UceSubscribe;

class UceSubscribeManager : public IMSActivityEx {
  /* ------------------------------------------------------------------------------------------
      Constructor, Destructor, Operator Overloading
  ---------------------------------------------------------------------------------------------
*/
 public:
  UceSubscribeManager(IN CONST AString &strName, ICoreService *_piCoreService,
                      IN CONST AString &strAppName, IN IMS_SINT32 nSimSlot);
  virtual ~UceSubscribeManager();
  /* ------------------------------------------------------------------------------------------
      Methods
  ---------------------------------------------------------------------------------------------
*/
 public:
  IMS_BOOL QuerySingleCapability(IN AString strUser, IN IMS_UINT32 key);
  IMS_BOOL QueryMultiCapability(IN IMSList<AString> objUsers,
                                IN IMS_UINT32 key);
  IMS_BOOL AosConnected(IMS_UINT32 conectedService);
  IMS_BOOL AosDisConnected();
  void ClosedService();  // core service closed
 protected:
  virtual IMS_BOOL OnMessage(IN IMSMSG &objMsg);

 private:
  IMS_BOOL RemoveSubscribe(IN UceSubscribe *subscribe);
  IMS_BOOL ClearSubscribeList();

  /* ------------------------------------------------------------------------------------------
      Variable
  ---------------------------------------------------------------------------------------------
*/
 protected:
  IMSList<UceSubscribe *> m_objUceSubscribeList;  // Subscription List
  ICoreService *m_piCoreService;
  AString m_strAppName;
  IMS_SINT32 m_nSimSlot;
  IMS_UINT32 m_nConnectedServices;
};
#endif  // _UCE_SUBSCRIBE_MANAGER_H_
