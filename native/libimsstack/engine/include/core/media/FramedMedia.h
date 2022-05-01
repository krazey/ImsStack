/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100503  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _FRAMED_MEDIA_H_
#define _FRAMED_MEDIA_H_

#include "IMSMap.h"
#include "AStringArray.h"
#include "ByteArray.h"
#include "media/Media.h"

class FramedMedia
    : public Media
{
public:
    FramedMedia(IN Service *pService_, IN ISDPOAState *piOAState_);
    virtual ~FramedMedia();

private:
    FramedMedia(IN CONST FramedMedia &objRHS);
    FramedMedia& operator=(IN CONST FramedMedia &objRHS);

public:
    // Media class
    virtual IMS_SINT32 GetType() const;

protected:
    // Media class
    virtual MediaProposal* CreateMediaProposal(IN ISDPOAState *piOAState);
};

#endif // _FRAMED_MEDIA_H_
