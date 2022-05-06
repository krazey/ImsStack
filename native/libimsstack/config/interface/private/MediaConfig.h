/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20091026  toastops@                 Created
    </table>

    Description

*/

#ifndef _MEDIA_CONFIG_H_
#define _MEDIA_CONFIG_H_

#include "private/ConfigBase.h"
#include "IMediaConfig.h"

class MediaConfigPrivate;

class MediaConfig : public ConfigBase, public IMediaConfig
{
public:
    explicit MediaConfig(IN IMS_SINT32 nSlotId);
    virtual ~MediaConfig();

private:
    MediaConfig(IN const MediaConfig& objRHS);
    MediaConfig& operator=(IN const MediaConfig& objRHS);

public:
    // IMediaConfig interface
    virtual const AStringArray& GetMediaCapabilities(IN IMS_SINT32 nMediaType) const;
    virtual const AStringArray& GetMediaProfile(
            IN const AString& strName, IN IMS_SINT32 nMediaType) const;

    // ConfigBase class
    virtual void Refresh();

    IMS_BOOL CreateMediaProfile(IN const AString& strName);
    void DestroyMediaProfile(IN const AString& strName);

protected:
    // ConfigBase class
    virtual IMS_BOOL ReadFrom();
    virtual IMS_BOOL WriteTo();

    IMS_BOOL ReadMediaProfile(IN const AString& strMProfName, IN const AString& strConfName);
    IMS_BOOL WriteMediaProfile(IN const AString& strMProfName, IN const AString& strConfName);

private:
    static const IMS_CHAR SECTION_PROFILES[];
    static const IMS_CHAR KEY_IDS[];

    MediaConfigPrivate* pMediaConfigP;
};

#endif  // _MEDIA_CONFIG_H_
