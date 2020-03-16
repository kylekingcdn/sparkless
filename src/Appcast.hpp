//
//  Appcast.hpp
//  sparkless
//
//  Created by Kyle King on 2020-03-14.
//  Copyright © 2020 Kyle King. All rights reserved.
//

#ifndef Appcast_hpp
#define Appcast_hpp

#include <QObject>
#include <QDomDocument>
#include <QHash>

#include "Constants.hpp"

class ItemEnclosure;
class AppcastItem;

class Appcast : public QObject {
  Q_OBJECT

private:

  QDomDocument appcastDoc;

  QString title;

  QList<AppcastItem*> items;
  QHash<qlonglong, AppcastItem*> itemHash;

  QString s3Region;
  QString s3BucketName;
  QString s3BucketDir;
  QString s3LocalMirrorPath;

  QString urlPrefix;


#pragma mark - Constructors -

#pragma mark Private
private:

  Appcast(const QDomDocument&, QObject* theParent = nullptr);

#pragma mark Public
public:

  static Appcast* FromDocument(const QDomDocument&, QObject* theParent = nullptr);
  static Appcast* FromPath(const QString&, QObject* theParent = nullptr);

  virtual ~Appcast() Q_DECL_OVERRIDE;


#pragma mark - Accessors -

#pragma mark Private
private:

#pragma mark Public
public:

  const QList<AppcastItem*>& Items() const { return items; }

  const QString& Title() const { return title; }

  AppcastItem* Item(const qlonglong theBuildVersion) const;

  const QString& S3Region() const { return s3Region; }
  const QString& S3BucketName() const { return s3BucketName; }
  const QString& S3BucketDir() const { return s3BucketDir; }
  const QString& S3LocalMirrorPath() const { return s3LocalMirrorPath; }

  const QString& UrlPrefix() const { return urlPrefix; }

  bool Contains(const qlonglong theBuildVersion) const;
  bool ContainsEnclosure(const qlonglong theBuildVersion, const EnclosurePlatform) const;

  const QString UrlForRelease(const QString& theReleaseFileName, const EnclosurePlatform thePlatform) const;

  void PrintItems() const;


#pragma mark - Mutators -

#pragma mark Private
private:

  bool ParseXml();

  ItemEnclosure* AddEnclosureToItemWithSignature(AppcastItem* theItem, const QString& theFilePath, const EnclosurePlatform thePlatform, const QByteArray& theSignature, const EnclosureSignatureType theSignatureType);

#pragma mark Public
public:

  void SetS3Region(const QString&);
  void SetS3BucketName(const QString&);
  void SetS3BucketDir(const QString&);
  void SetS3LocalMirrorPath(const QString&);

  void SetUrlPrefix(const QString&);

  AppcastItem* CreateItem(const QString& theVersionDescription, const qlonglong theVersionBuild);
  ItemEnclosure* AddEnclosureToIem(AppcastItem* theItem, const QString& theFilePath, const EnclosurePlatform thePlatform, const QString& theDsaKeyPath);
  ItemEnclosure* AddEnclosureToIem(AppcastItem* theItem, const QString& theFilePath, const EnclosurePlatform thePlatform, const QByteArray& theEdDsaKey);

  bool Save(const QString& theFilePath);

  bool AddItem(AppcastItem*);


};

#endif /* Appcast_hpp */
