//
//  AppcastItem.hpp
//  sparkless
//
//  Created by Kyle King on 2020-03-14.
//  Copyright © 2020 Kyle King. All rights reserved.
//

#ifndef AppcastItem_hpp
#define AppcastItem_hpp

#include <QObject>
#include <QDomDocument>
#include <QUrl>
#include <QDateTime>

#include "Constants.hpp"

class ItemEnclosure;
class ItemDelta;

class AppcastItem : public QObject {
  Q_OBJECT

private:

  QDomElement itemElement;

  QString title;
  QString description;

  QUrl releaseNotesUrl;
  QDateTime publishedTimestamp;

  QString versionDescription;
  qlonglong versionBuild = -1;

  QList<ItemEnclosure*> enclosures;
  QList<ItemDelta*> deltas;

  QHash<qlonglong, QHash<EnclosurePlatform, ItemDelta*>> deltaHash;

private:

  AppcastItem(QObject* theParent = nullptr);
  AppcastItem(const QDomElement&, QObject* theParent = nullptr);

public:

  static AppcastItem* FromElement(const QDomElement&, QObject* theParent = nullptr);
  static AppcastItem* NewItem(const QString& theVersionDescription, const qlonglong theVersionBuild, QObject* theParent = nullptr);

  virtual ~AppcastItem() Q_DECL_OVERRIDE;

private:

  static QDateTime TimestampFromString(const QString&);
  static QString TimestampToString(const QDateTime&);

public:

  const QString Title() const { return title; }
  const QString Description() const { return description; }

  const QUrl& ReleaseNotesUrl() const { return releaseNotesUrl; }

  const QDateTime PublishedTimestamp() const { return publishedTimestamp; }
  const QString PublishedTimestampString() const { return TimestampToString(publishedTimestamp); }

  const QString& VersionDescription() const { return versionDescription; }
  qlonglong VersionBuild() const { return versionBuild; }

  const QList<ItemEnclosure*>& Enclosures() const { return enclosures; }
  ItemEnclosure* Enclosure(const EnclosurePlatform) const;
  bool HasEnclosure(const EnclosurePlatform) const;

  const QList<ItemDelta*>& Deltas() const { return deltas; }
  ItemDelta Delta(const EnclosurePlatform, const qlonglong) const;

  void Print() const;

private:

  bool ParseXml();

public:

  void SetTitle(const QString&);
  void SetDescription(const QString&);
  void SetReleaseNotesUrl(const QUrl&);

  ItemEnclosure* AddEnclosure(const qlonglong theLength, const QUrl& theUrl, const EnclosurePlatform thePlatform, const QByteArray& theSignature, const EnclosureSignatureType theSignatureType);
  ItemDelta* AddDelta(const qlonglong prevBuildVersion, const qlonglong theLength, const QUrl& theUrl, const EnclosurePlatform thePlatform, const QByteArray& theSignature, const EnclosureSignatureType theSignatureType);
};

#endif /* AppcastItem_hpp */
