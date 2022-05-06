/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20091208  toastops@                 Created
    </table>

    Description

*/

#ifndef _MEDIA_FACTORY_H_
#define _MEDIA_FACTORY_H_

#include "media/Media.h"

class Service;
class ISDPOAState;

class MediaFactory
{
private:
    MediaFactory();

public:
    static Media* CreateOutgoingMedia(IN CONST AString& strMType, IN IMS_SINT32 nDirection,
            IN Service* pService, IN ISDPOAState* piOAState, IN IMS_SINT32 nCountOfDescriptor);

    static Media* CreateIncomingMedia(IN IMS_SINT32 nTransportProtocol, IN Service* pService,
            IN ISDPOAState* piOAState, IN CONST IMSList<IMS_SINT32>& objMids);

    static void DestroyMedia(IN Media*& pMedia);
};

#endif  // _MEDIA_FACTORY_H_
