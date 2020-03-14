//
//  ItemEnclosure.hpp
//  sparkless
//
//  Created by Kyle King on 2020-03-14.
//  Copyright © 2020 Kyle King. All rights reserved.
//

#ifndef ItemEnclosure_hpp
#define ItemEnclosure_hpp

#include <QObject>
#include <QDomDocument>
#include <QUrl>
#include <QDateTime>

class ItemEnclosure : public QObject {
  Q_OBJECT

public:

  enum EnclosurePlatform {
    NullPlatform = 0,
    MacPlatform,
    WindowsPlatform,
  };
  Q_ENUM(EnclosurePlatform);

private:

  QDomElement enclosureElement;

  QString versionDescription;
  qlonglong versionBuild = -1;

  QUrl fileUrl;
  QString mimeType;
  qlonglong length = -1;
  QByteArray signature;

  EnclosurePlatform platform = NullPlatform;


#pragma mark - Constructors -

#pragma mark Protected
protected:

  ItemEnclosure(const QDomElement&, QObject* theParent = nullptr);

#pragma mark Public
public:

  static ItemEnclosure* FromElement(const QDomElement&, QObject* theParent = nullptr);

  virtual ~ItemEnclosure() Q_DECL_OVERRIDE;


#pragma mark - Accessors -

#pragma mark Private
private:

#pragma mark Protected
protected:

  const QDomElement& EnclosureElement() const { return enclosureElement; }

#pragma mark Public
public:

  const QString& VersionDescription() const { return versionDescription; }
  qlonglong VersionBuild() const { return versionBuild; }

  const QUrl& FileUrl() const { return fileUrl; }
  const QString& MimeType() const { return mimeType; }
  qlonglong Length() const { return length; }
  const QByteArray& Signature() const { return signature; }

  EnclosurePlatform Platform() const { return platform; }
  QString PlatformString() const { return PlatformToString(platform); }
  QString PlatformDescription() const { return PlatformToDescription(platform); }
  
  static EnclosurePlatform PlatformFromString(const QString&);
  static QString PlatformToString(const EnclosurePlatform&);
  static QString PlatformToDescription(const EnclosurePlatform&);

  virtual void Print() const;


#pragma mark - Mutators -

#pragma mark Protected
protected:

  virtual bool ParseXml();

#pragma mark Public
public:


};

#endif /* ItemEnclosure_hpp */
