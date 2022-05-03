/*
    Author
    <table>
    date          author                        description
    --------      --------------                ----------
    20150415    il.won@                   Created
    </table>

    Description

*/

#ifndef UC_LOCATION_OBJECT_H_
#define UC_LOCATION_OBJECT_H_

#include "call/IMtcCall.h"

class IMessage;
class ISipMessage;

class UCLocationObject
{
public:
    UCLocationObject();
    virtual ~UCLocationObject();

public:
    virtual IMS_BOOL IsGeolocationInfoRequired(IN IMtcCall* pSession);
    void SetLocation(IN IMtcCall* pSession, IN_OUT IMessage* piMessage,
            IN IMS_BOOL bGeolocationRouting = IMS_FALSE);
    void SetLocation(IN IMtcCall* pSession, IN_OUT ISipMessage* piSIPMessage,
            IN IMS_BOOL bGeolocationRouting = IMS_FALSE);

private:
    IMS_BOOL IsGeolocationPidfSupported(
            IN IMS_SINT32 nSlotId, IN IMS_SINT32 nGeolocationPidfType) const;

private:
    static const IMS_CHAR STR_APPLICATION_PIDF_XML[];
    static const IMS_CHAR STR_GEOLOCATION[];
    static const IMS_CHAR STR_GEOLOCATION_ROUTING[];
    static const IMS_CHAR STR_NO[];
    static const IMS_CHAR STR_YES[];
};

#endif  // UC_LOCATION_OBJECT_H_
