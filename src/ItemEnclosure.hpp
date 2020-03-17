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

#include "Constants.hpp"

class ItemEnclosure : public QObject {
  Q_OBJECT

private:

  static QList<EnclosureSignatureType> VALID_SIGNATURE_TYPES;

  QDomElement enclosureElement;

  QString versionDescription;
  qlonglong versionBuild = -1;

  QUrl fileUrl;
  QString mimeType;
  qlonglong length = -1;

  QByteArray signature;
  EnclosureSignatureType signatureType = NullSignature;

  EnclosurePlatform platform = NullPlatform;
  QStringList installerArguments;


#pragma mark - Constructors -

#pragma mark Protected
protected:

  ItemEnclosure(QObject* theParent = nullptr);
  ItemEnclosure(const qlonglong theLength, const qlonglong theBuild, const QString& theVersion, const QUrl& theUrl, const EnclosurePlatform thePlatform, const QByteArray& theSignature, const EnclosureSignatureType theSignatureType, QObject* theParent = nullptr);
  ItemEnclosure(const QDomElement&, QObject* theParent = nullptr);

#pragma mark Public
public:

  static ItemEnclosure* FromElement(const QDomElement&, QObject* theParent = nullptr);

  static ItemEnclosure* NewEnclosure(const qlonglong theLength, const qlonglong theBuild, const QString& theVersion, const QUrl& theUrl, const EnclosurePlatform thePlatform, const QByteArray& theSignature, const EnclosureSignatureType theSignatureType, QObject* theParent = nullptr);

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
  EnclosureSignatureType SignatureType() const { return signatureType; }
  QString SignatureTypeXmlKey() const { return SignatureTypeToXmlKey(signatureType); }
  QString SignatureTypeDescription() const { return SignatureTypeToDescription(signatureType); }

  EnclosurePlatform Platform() const { return platform; }
  QString PlatformXmlValue() const { return PlatformToXmlValue(platform); }
  QString PlatformDescription() const { return PlatformToDescription(platform); }
  const QStringList& InstallerArguments() const { return installerArguments; }

  static EnclosureSignatureType SignatureTypeFromXmlKey(const QString&);
  static QString SignatureTypeToXmlKey(const EnclosureSignatureType);
  static QString SignatureTypeToDescription(const EnclosureSignatureType);

  static EnclosurePlatform PlatformFromXmlValue(const QString&);
  static QString PlatformToXmlValue(const EnclosurePlatform&);
  static QString PlatformToDescription(const EnclosurePlatform&);

  virtual void Print() const;


#pragma mark - Mutators -

#pragma mark Protected
protected:

  virtual bool ParseXml();

#pragma mark Public
public:

  virtual bool Serialize(QDomElement& theEnclosureElement);
  
};

#endif /* ItemEnclosure_hpp */
