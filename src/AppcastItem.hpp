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
  QUrl releaseNotesUrl;
  QDateTime publishedTimestamp;

  QString versionDescription;
  qlonglong versionBuild = -1;

  QList<ItemEnclosure*> enclosures;
  QList<ItemDelta*> deltas;


#pragma mark - Constructors -

#pragma mark Private
private:

  AppcastItem(const QDomElement&, QObject* theParent = nullptr);

#pragma mark Public
public:

  static AppcastItem* FromElement(const QDomElement&, QObject* theParent = nullptr);

  virtual ~AppcastItem() Q_DECL_OVERRIDE;


#pragma mark - Accessors -

#pragma mark Private
private:

  static QDateTime TimestampFromString(const QString&);
  static QString TimestampToString(const QDateTime&);

#pragma mark Public
public:

  const QString Title() const { return title; }
  const QUrl& ReleaseNotesUrl() const { return releaseNotesUrl; }

  const QDateTime PublishedTimestamp() const { return publishedTimestamp; }
  const QString PublishedTimestampString() const { return TimestampToString(publishedTimestamp); }

  const QString& VersionDescription() const { return versionDescription; }
  qlonglong VersionBuild() const { return versionBuild; }

  const QList<ItemEnclosure*>& Enclosures() const { return enclosures; }
  const QList<ItemDelta*>& Deltas() const { return deltas; }

  void Print() const;


#pragma mark - Mutators -

#pragma mark Private
private:

  bool ParseXml();

#pragma mark Public
public:


};

#endif /* AppcastItem_hpp */
