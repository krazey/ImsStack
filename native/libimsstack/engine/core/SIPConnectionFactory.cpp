/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20140203  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ImsIdentity.h"
#include "ISipServerConnection.h"
#include "ISipDialog.h"
#include "SipStatusCode.h"
#include "SipDebug.h"
#include "ISIPConnectionFactoryListener.h"
#include "util/CancellableMethodManager.h"
#include "util/DialogMethodManager.h"
#include "Service.h"
#include "SIPConnectionFactory.h"

__IMS_TRACE_TAG_IMS_CORE__;



PUBLIC
SIPConnectionFactory::SIPConnectionFactory(IN Service *pService_)
    : pService(pService_)
    , piDialog(IMS_NULL)
    , piListener(IMS_NULL)
    , piInitialSSC(IMS_NULL)
    , piInviteSSC(IMS_NULL)
{
    DialogMethodManager::GetInstance()->AddMethod(GetName(), this);
}

PUBLIC
SIPConnectionFactory::SIPConnectionFactory(IN Service *pService_, IN ISIPServerConnection *piSSC)
    : pService(pService_)
    , piDialog(IMS_NULL)
    , piListener(IMS_NULL)
    , piInitialSSC(piSSC)
    , piInviteSSC(IMS_NULL)
{
    DialogMethodManager::GetInstance()->AddMethod(GetName(), this);
    CancellableMethodManager::GetInstance()->AddMethod(GetName(), this);
}

PUBLIC VIRTUAL
SIPConnectionFactory::~SIPConnectionFactory()
{
    if (piDialog != IMS_NULL)
    {
        piDialog->Destroy();
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_BOOL SIPConnectionFactory::DispatchMessage(IN IMSMSG &objMSG)
{
    //---------------------------------------------------------------------------------------------

    switch (objMSG.GetName())
    {
    case AMSG_SSC_FOR_MID_DIALOG_RECEIVED:
    {
        ISIPServerConnection *piSSC = reinterpret_cast<ISIPServerConnection*>(objMSG.nLparam);

        if (piListener == IMS_NULL)
        {
            if (pService != IMS_NULL)
            {
                pService->SendResponse(piSSC, SIPStatusCode::SC_480);
            }

            piSSC->Close();
            return IMS_TRUE;
        }

        piListener->ConnectionFactory_NotifyRequest(this, piSSC);
        return IMS_TRUE;
    }

    default:
        break;
    }

    return EngineActivity::DispatchMessage(objMSG);
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_BOOL SIPConnectionFactory::Dialog_Compare(IN ISIPServerConnection *piSSC) const
{
    //---------------------------------------------------------------------------------------------

    if (piDialog == IMS_NULL)
    {
        return IMS_FALSE;
    }

    const SIPMethod &objMethod = piSSC->GetMethod();

    if (objMethod.Equals(SIPMethod::REFER))
    {
        // If the server transaction has the same dialog identifier with a dialog of this session,
        // then it will be handled by this session.
        ISIPDialog *piReferDialog = piSSC->GetDialog();

        if (piReferDialog == IMS_NULL)
        {
            return IMS_FALSE;
        }

        AString strDialogId = piDialog->GetDialogID();
        AString strReferDialogId = piReferDialog->GetDialogID();

        if (strDialogId.Equals(strReferDialogId))
        {
            IMS_TRACE_D("SIPConnectionFactory :: Dialog (%s), Refer's Dialog (%s)",
                    SIPDebug::GetCharA1(strDialogId.GetStr(), 8, '@'),
                    SIPDebug::GetCharA2(strReferDialogId.GetStr(), 8, '@'), 0);
            return IMS_TRUE;
        }

        return IMS_FALSE;
    }

    if (!piDialog->IsSameDialog(piSSC))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_BOOL SIPConnectionFactory::Dialog_NotifyRequest(IN ISIPServerConnection *piSSC)
{
    //---------------------------------------------------------------------------------------------

    if (piListener == IMS_NULL)
    {
        if (pService != IMS_NULL)
        {
            pService->SendResponse(piSSC, SIPStatusCode::SC_480);
        }

        piSSC->Close();
        return IMS_TRUE;
    }

    PostMessage(AMSG_SSC_FOR_MID_DIALOG_RECEIVED, 0, reinterpret_cast<IMS_UINTP>(piSSC));

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_BOOL SIPConnectionFactory::Cancellable_Compare(IN ISIPServerConnection *piSSC_CANCEL) const
{
    //---------------------------------------------------------------------------------------------

    if (piInviteSSC == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return piSSC_CANCEL->IsSameTransaction(piInviteSSC);
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_BOOL SIPConnectionFactory::Cancellable_NotifyRequest(IN ISIPServerConnection *piSSC_CANCEL)
{
    //---------------------------------------------------------------------------------------------

    if (piListener != IMS_NULL)
    {
        piListener->ConnectionFactory_NotifyRequest(this, piSSC_CANCEL);
    }
    else
    {
        CreateResponse(piSSC_CANCEL, SIPStatusCode::SC_200);
        piSSC_CANCEL->Send();
        piSSC_CANCEL->Close();
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC VIRTUAL
void SIPConnectionFactory::Destroy()
{
    //---------------------------------------------------------------------------------------------

    DialogMethodManager::GetInstance()->RemoveMethod(GetName());
    CancellableMethodManager::GetInstance()->RemoveMethod(GetName());

    PostMessage(AMSG_DESTROY, 0, 0);
}

/*

Remarks
 SIP_MESSAGE_MEDIATOR
*/
PUBLIC VIRTUAL
void SIPConnectionFactory::SetMessageMediator(IN IMessageMediator * /* piMediator */)
{
    // no-op
}

/*

Remarks

*/
PUBLIC VIRTUAL
ISIPClientConnection* SIPConnectionFactory::CreateClientConnection(IN CONST SIPMethod &objMethod,
        IN CONST SIPAddress *pFrom, IN CONST SIPAddress *pTo)
{
    //---------------------------------------------------------------------------------------------

    if (pService == IMS_NULL)
    {
        IMS_TRACE_E(0, "Service is null", 0, 0, 0);
        return IMS_NULL;
    }

    const SIPAddress *pTempFrom;
    const SIPAddress *pTempTo;
    IMS_BOOL bFromDeleteRequired = IMS_FALSE;
    IMS_BOOL bToDeleteRequired = IMS_FALSE;

    if (pFrom == IMS_NULL)
    {
        bFromDeleteRequired = IMS_TRUE;
        pTempFrom = new SIPAddress(pService->GetAuthorizedUserId());
    }
    else
    {
        pTempFrom = pFrom;
    }

    if (pTo == IMS_NULL)
    {
        bToDeleteRequired = IMS_TRUE;
        pTempTo = new SIPAddress(ImsIdentity::GetAnonymousUserId());
    }
    else
    {
        pTempTo = pTo;
    }

    ISIPClientConnection *piSCC = pService->CreateConnection(pTempFrom, pTempTo, objMethod);

    if (bFromDeleteRequired)
    {
        delete pTempFrom;
    }

    if (bToDeleteRequired)
    {
        delete pTempTo;
    }

    return piSCC;
}

/*

Remarks

*/
PUBLIC VIRTUAL
ISIPClientConnection* SIPConnectionFactory::CreateClientConnection(IN ISIPDialog *piDialog,
        IN CONST SIPMethod &objMethod)
{
    //---------------------------------------------------------------------------------------------

    if (pService == IMS_NULL)
    {
        IMS_TRACE_E(0, "Service is null", 0, 0, 0);
        return IMS_NULL;
    }

    ISIPDialog *piTempDialog = (piDialog != IMS_NULL) ? piDialog : this->piDialog;

    if (piTempDialog == IMS_NULL)
    {
        IMS_TRACE_E(0, "ISIPDialog is null", 0, 0, 0);
        return IMS_NULL;
    }

    return pService->CreateConnection(piTempDialog, objMethod);
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_BOOL SIPConnectionFactory::CreateResponse(IN_OUT ISIPServerConnection *piSSC,
        IN IMS_SINT32 nStatusCode, IN CONST AString &strPhrase /* = AString::ConstNull() */)
{
    //---------------------------------------------------------------------------------------------

    if (pService == IMS_NULL)
    {
        IMS_TRACE_E(0, "Service is null", 0, 0, 0);
        return IMS_FALSE;
    }

    return pService->CreateResponse(piSSC, nStatusCode, strPhrase);
}

/*

Remarks

*/
PUBLIC VIRTUAL
ISIPServerConnection* SIPConnectionFactory::GetNewServerConnection()
{
    //---------------------------------------------------------------------------------------------

    return piInitialSSC;
}

/*

Remarks

*/
PUBLIC VIRTUAL
void SIPConnectionFactory::SetDialog(IN ISIPDialog *piDialog)
{
    //---------------------------------------------------------------------------------------------

    if (this->piDialog != IMS_NULL)
    {
        this->piDialog->Destroy();
        this->piDialog = IMS_NULL;
    }

    if (piDialog != IMS_NULL)
    {
        this->piDialog = piDialog->Clone();
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL
void SIPConnectionFactory::SetListener(IN ISIPConnectionFactoryListener *piListener)
{
    //---------------------------------------------------------------------------------------------

    this->piListener = piListener;
}

/*

Remarks

*/
PUBLIC VIRTUAL
void SIPConnectionFactory::SetSSCForCANCEL(IN ISIPServerConnection *piSSC)
{
    //---------------------------------------------------------------------------------------------

    piInviteSSC = piSSC;
}
