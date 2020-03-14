//
//  AppcastItem.cpp
//  sparkless
//
//  Created by Kyle King on 2020-03-14.
//  Copyright © 2020 Kyle King. All rights reserved.
//

#include "AppcastItem.hpp"

#include <QDebug>

#include "ItemEnclosure.hpp"
#include "ItemDelta.hpp"

#pragma mark - Constructors -

#pragma mark Private

AppcastItem::AppcastItem(const QDomElement& theItemElement, QObject* theParent)
: QObject(theParent) {

  itemElement = theItemElement;
}

#pragma mark Public

AppcastItem* AppcastItem::FromElement(const QDomElement& theItemElement, QObject* theParent) {

  AppcastItem* item = new AppcastItem(theItemElement, theParent);

  if (!item->ParseXml()) {
    delete item;
    item = nullptr;
  }

  return item;
}

AppcastItem::~AppcastItem() {

}


#pragma mark - Accessors -

#pragma mark Private

QDateTime AppcastItem::TimestampFromString(const QString& theString) {

  QDateTime timestamp = QDateTime::fromString(theString, "ddd, dd MMM yyyy HH:mm:ss +0000");
  timestamp.setTimeSpec(Qt::TimeSpec::UTC);

  return timestamp;
}

QString AppcastItem::TimestampToString(const QDateTime& theTimestamp) {

  return theTimestamp.toString("ddd, dd MMM yyyy HH:mm:ss +0000");
}

#pragma mark Public

void AppcastItem::Print() const {

  qDebug();
  qDebug().noquote().nospace() << QString("%1 %2 (%3)").arg(title).arg(versionDescription).arg(versionBuild);
  qDebug().noquote().nospace() << "  Published: " << publishedTimestamp.toString();
  qDebug().noquote().nospace() << "  Enclosures";
  foreach (ItemEnclosure* currEnclosure, enclosures) {
    if (currEnclosure != nullptr) {

      qDebug().noquote().nospace() << "     " << QString("%1:  %2").arg(currEnclosure->PlatformDescription(), 7).arg(currEnclosure->FileUrl().toString());
    }
  }
}



#pragma mark - Mutators -

#pragma mark Private

bool AppcastItem::ParseXml() {

  title = itemElement.firstChildElement("title").firstChild().nodeValue();
  releaseNotesUrl = itemElement.firstChildElement("sparkle:releaseNotesLink").firstChild().nodeValue();

  const QString publishedTimestampStr = itemElement.firstChildElement("pubDate").firstChild().nodeValue();
  publishedTimestamp = TimestampFromString(publishedTimestampStr);

  QDomElement enclosureElement = itemElement.firstChildElement("enclosure");

  while (!enclosureElement.isNull()) {

    ItemEnclosure* enclosure = ItemEnclosure::FromElement(enclosureElement, this);
    if (enclosure != nullptr) {
      enclosures.append(enclosure);

      // assign item version to enclosure version if it is unset
      if (versionBuild < 0 && enclosure->VersionBuild() >= 0) {
        versionDescription = enclosure->VersionDescription();
        versionBuild = enclosure->VersionBuild();
      }
    }

    enclosureElement = enclosureElement.nextSiblingElement("enclosure");
  }

  QDomElement deltasElement = itemElement.firstChildElement("sparkle:deltas");
  if (!deltasElement.isNull()) {

    QDomElement deltaEnclosureElement = deltasElement.firstChildElement("enclosure");

    while (!deltaEnclosureElement.isNull()) {

      ItemDelta* enclosure = ItemDelta::FromElement(deltaEnclosureElement, this);
      if (enclosure != nullptr) {
        enclosures.append(enclosure);
      }

      deltaEnclosureElement = deltaEnclosureElement.nextSiblingElement("enclosure");
    }
  }

  return true;
}

#pragma mark Public

