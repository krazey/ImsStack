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
#include "ISipConnectionFactoryListener.h"
#include "util/CancellableMethodManager.h"
#include "util/DialogMethodManager.h"
#include "Service.h"
#include "SipConnectionFactory.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
SipConnectionFactory::SipConnectionFactory(IN Service* pService_) :
        pService(pService_),
        piDialog(IMS_NULL),
        piListener(IMS_NULL),
        piInitialSSC(IMS_NULL),
        piInviteSSC(IMS_NULL)
{
    DialogMethodManager::GetInstance()->AddMethod(GetName(), this);
}

PUBLIC
SipConnectionFactory::SipConnectionFactory(IN Service* pService_, IN ISipServerConnection* piSSC) :
        pService(pService_),
        piDialog(IMS_NULL),
        piListener(IMS_NULL),
        piInitialSSC(piSSC),
        piInviteSSC(IMS_NULL)
{
    DialogMethodManager::GetInstance()->AddMethod(GetName(), this);
    CancellableMethodManager::GetInstance()->AddMethod(GetName(), this);
}

PUBLIC VIRTUAL SipConnectionFactory::~SipConnectionFactory()
{
    if (piDialog != IMS_NULL)
    {
        piDialog->Destroy();
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL SipConnectionFactory::DispatchMessage(IN IMSMSG& objMSG)
{
    //---------------------------------------------------------------------------------------------

    switch (objMSG.GetName())
    {
        case AMSG_SSC_FOR_MID_DIALOG_RECEIVED:
        {
            ISipServerConnection* piSSC = reinterpret_cast<ISipServerConnection*>(objMSG.nLparam);

            if (piListener == IMS_NULL)
            {
                if (pService != IMS_NULL)
                {
                    pService->SendResponse(piSSC, SipStatusCode::SC_480);
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
PUBLIC VIRTUAL IMS_BOOL SipConnectionFactory::Dialog_Compare(IN ISipServerConnection* piSSC) const
{
    //---------------------------------------------------------------------------------------------

    if (piDialog == IMS_NULL)
    {
        return IMS_FALSE;
    }

    const SipMethod& objMethod = piSSC->GetMethod();

    if (objMethod.Equals(SipMethod::REFER))
    {
        // If the server transaction has the same dialog identifier with a dialog of this session,
        // then it will be handled by this session.
        ISipDialog* piReferDialog = piSSC->GetDialog();

        if (piReferDialog == IMS_NULL)
        {
            return IMS_FALSE;
        }

        AString strDialogId = piDialog->GetDialogId();
        AString strReferDialogId = piReferDialog->GetDialogId();

        if (strDialogId.Equals(strReferDialogId))
        {
            IMS_TRACE_D("SipConnectionFactory :: Dialog (%s), Refer's Dialog (%s)",
                    SipDebug::GetCharA1(strDialogId.GetStr(), 8, '@'),
                    SipDebug::GetCharA2(strReferDialogId.GetStr(), 8, '@'), 0);
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
PUBLIC VIRTUAL IMS_BOOL SipConnectionFactory::Dialog_NotifyRequest(IN ISipServerConnection* piSSC)
{
    //---------------------------------------------------------------------------------------------

    if (piListener == IMS_NULL)
    {
        if (pService != IMS_NULL)
        {
            pService->SendResponse(piSSC, SipStatusCode::SC_480);
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
PUBLIC VIRTUAL IMS_BOOL SipConnectionFactory::Cancellable_Compare(
        IN ISipServerConnection* piSSC_CANCEL) const
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
PUBLIC VIRTUAL IMS_BOOL SipConnectionFactory::Cancellable_NotifyRequest(
        IN ISipServerConnection* piSSC_CANCEL)
{
    //---------------------------------------------------------------------------------------------

    if (piListener != IMS_NULL)
    {
        piListener->ConnectionFactory_NotifyRequest(this, piSSC_CANCEL);
    }
    else
    {
        CreateResponse(piSSC_CANCEL, SipStatusCode::SC_200);
        piSSC_CANCEL->Send();
        piSSC_CANCEL->Close();
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC VIRTUAL void SipConnectionFactory::Destroy()
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
PUBLIC VIRTUAL void SipConnectionFactory::SetMessageMediator(IN IMessageMediator* /* piMediator */)
{
    // no-op
}

/*

Remarks

*/
PUBLIC VIRTUAL ISipClientConnection* SipConnectionFactory::CreateClientConnection(
        IN CONST SipMethod& objMethod, IN CONST SipAddress* pFrom, IN CONST SipAddress* pTo)
{
    //---------------------------------------------------------------------------------------------

    if (pService == IMS_NULL)
    {
        IMS_TRACE_E(0, "Service is null", 0, 0, 0);
        return IMS_NULL;
    }

    const SipAddress* pTempFrom;
    const SipAddress* pTempTo;
    IMS_BOOL bFromDeleteRequired = IMS_FALSE;
    IMS_BOOL bToDeleteRequired = IMS_FALSE;

    if (pFrom == IMS_NULL)
    {
        bFromDeleteRequired = IMS_TRUE;
        pTempFrom = new SipAddress(pService->GetAuthorizedUserId());
    }
    else
    {
        pTempFrom = pFrom;
    }

    if (pTo == IMS_NULL)
    {
        bToDeleteRequired = IMS_TRUE;
        pTempTo = new SipAddress(ImsIdentity::GetAnonymousUserId());
    }
    else
    {
        pTempTo = pTo;
    }

    ISipClientConnection* piSCC = pService->CreateConnection(pTempFrom, pTempTo, objMethod);

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
PUBLIC VIRTUAL ISipClientConnection* SipConnectionFactory::CreateClientConnection(
        IN ISipDialog* piDialog, IN CONST SipMethod& objMethod)
{
    //---------------------------------------------------------------------------------------------

    if (pService == IMS_NULL)
    {
        IMS_TRACE_E(0, "Service is null", 0, 0, 0);
        return IMS_NULL;
    }

    ISipDialog* piTempDialog = (piDialog != IMS_NULL) ? piDialog : this->piDialog;

    if (piTempDialog == IMS_NULL)
    {
        IMS_TRACE_E(0, "ISipDialog is null", 0, 0, 0);
        return IMS_NULL;
    }

    return pService->CreateConnection(piTempDialog, objMethod);
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL SipConnectionFactory::CreateResponse(IN_OUT ISipServerConnection* piSSC,
        IN IMS_SINT32 nStatusCode, IN CONST AString& strPhrase /* = AString::ConstNull() */)
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
PUBLIC VIRTUAL ISipServerConnection* SipConnectionFactory::GetNewServerConnection()
{
    //---------------------------------------------------------------------------------------------

    return piInitialSSC;
}

/*

Remarks

*/
PUBLIC VIRTUAL void SipConnectionFactory::SetDialog(IN ISipDialog* piDialog)
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
PUBLIC VIRTUAL void SipConnectionFactory::SetListener(IN ISipConnectionFactoryListener* piListener)
{
    //---------------------------------------------------------------------------------------------

    this->piListener = piListener;
}

/*

Remarks

*/
PUBLIC VIRTUAL void SipConnectionFactory::SetSSCForCANCEL(IN ISipServerConnection* piSSC)
{
    //---------------------------------------------------------------------------------------------

    piInviteSSC = piSSC;
}
