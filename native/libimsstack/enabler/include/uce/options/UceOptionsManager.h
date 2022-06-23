/*



*/

#ifndef _UCE_OPTIONS_MANAGER_H_
#define _UCE_OPTIONS_MANAGER_H_

#include "ImsActivityEx.h"
#include "ImsMap.h"

class UceOptions;
class ICoreService;
class ICapabilities;

class UceOptionsManager : public ImsActivityEx
{
public:
    UceOptionsManager(
            IN CONST AString& strName, IN ICoreService* piCoreService, IN IMS_SINT32 simSlotId);
    virtual ~UceOptionsManager();

    IMS_BOOL SendOptionsRequest(
            IN IMS_UINT32 nKey, IN AString strRemoteURI, IN IMS_UINT32 ownCapabilities);
    IMS_BOOL SendOptionsResponse(IN IMS_UINT32 nKey, IN IMS_UINT32 nResponse, IN AString reason,
            IN IMS_UINT32 ownCapabilities);
    IMS_BOOL ReceivedOptions(IN ICoreService* piCoreService, IN ICapabilities* piCapabilities);

    void AoSConnected();
    void AoSDisconnected();
    void ClosedService();  // core service closed
protected:
    virtual IMS_BOOL OnMessage(IN IMSMSG& objMsg);

private:
    IMS_UINT32 getReceivedKey();
    void SendOptionsReceivedInd(IN IMS_UINT32 nKey, IN AString from, IN IMS_UINT32 capabilities);
    void SendOptionsCommandError(IN IMS_UINT32 nKey, IN IMS_UINT32 code);

private:
    IMS_SINT32 m_nSimSlot;
    ICoreService* m_piCoreService;
    IMS_BOOL m_bAoSConnected;
    IMSMap<IMS_UINT32, UceOptions*> m_objSentUceOptionsMap;
    IMSMap<IMS_UINT32, UceOptions*> m_objReceivedUceOptionsMap;
    IMS_UINT32 m_nReceivedOptionKey;
};
#endif  // _UCE_OPTIONS_MANAGER_H_
