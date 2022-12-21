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

AppcastItem::AppcastItem(QObject* theParent)
: QObject(theParent) {

}

AppcastItem::AppcastItem(const QDomElement& theItemElement, QObject* theParent)
: QObject(theParent) {

  itemElement = theItemElement;
}

AppcastItem* AppcastItem::FromElement(const QDomElement& theItemElement, QObject* theParent) {

  AppcastItem* item = new AppcastItem(theItemElement, theParent);

  if (!item->ParseXml()) {
    delete item;
    item = nullptr;
  }

  return item;
}

AppcastItem* AppcastItem::NewItem(const QString& theVersionDescription, const qlonglong theVersionBuild, QObject* theParent) {

  AppcastItem* item = new AppcastItem(theParent);
  item->versionDescription = theVersionDescription;
  item->versionBuild = theVersionBuild;
  item->publishedTimestamp = QDateTime::currentDateTimeUtc();
  
  return item;
}

AppcastItem::~AppcastItem() {

}

QDateTime AppcastItem::TimestampFromString(const QString& theString) {

  QDateTime timestamp = QDateTime::fromString(theString, "ddd, dd MMM yyyy HH:mm:ss +0000");
  timestamp.setTimeSpec(Qt::TimeSpec::UTC);

  return timestamp;
}

QString AppcastItem::TimestampToString(const QDateTime& theTimestamp) {

  return theTimestamp.toString("ddd, dd MMM yyyy HH:mm:ss +0000");
}

ItemEnclosure* AppcastItem::Enclosure(const EnclosurePlatform thePlatform) const {

  foreach (ItemEnclosure* currEnclosure, enclosures) {

    if (currEnclosure != nullptr && currEnclosure->Platform() == thePlatform) {
      return currEnclosure;
    }
  }

  return nullptr;
}

bool AppcastItem::HasEnclosure(const EnclosurePlatform thePlatform) const {

  return Enclosure(thePlatform) != nullptr;
}

void AppcastItem::Print() const {

  qInfo();
  qInfo().noquote().nospace() << QString("%1 %2 (%3)").arg(title).arg(versionDescription).arg(versionBuild);
  qInfo().noquote().nospace() << "  Published: " << publishedTimestamp.toString();
  qInfo().noquote().nospace() << "  Enclosures";
  foreach (ItemEnclosure* currEnclosure, enclosures) {
    if (currEnclosure != nullptr) {

      qInfo().noquote().nospace() << "     "
        << QString("%1:  %2").arg(currEnclosure->PlatformDescription(), 7).arg(currEnclosure->FileUrl().toString())
        << QString(" [%1]").arg(currEnclosure->SignatureTypeDescription())
        << (!currEnclosure->InstallerArguments().isEmpty() ? QString(" (%1)").arg(currEnclosure->InstallerArguments().join(" ")) : "");
    }
  }
}

bool AppcastItem::ParseXml() {

  title = itemElement.firstChildElement("title").firstChild().nodeValue();
  description = itemElement.firstChildElement("description").firstChild().nodeValue();
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

      ItemDelta* delta = ItemDelta::FromElement(deltaEnclosureElement, this);
      if (delta != nullptr) {
        enclosures.append(delta);

        deltaHash[delta->VersionBuild()][delta->Platform()] = delta;
      }

      deltaEnclosureElement = deltaEnclosureElement.nextSiblingElement("enclosure");
    }
  }

  return true;
}

void AppcastItem::SetTitle(const QString& theTitle) {

  title = theTitle;
}

void AppcastItem::SetDescription(const QString& theDescription) {

  description = theDescription;
}

void AppcastItem::SetReleaseNotesUrl(const QUrl& theUrl) {

  releaseNotesUrl = theUrl;
}

ItemEnclosure* AppcastItem::AddEnclosure(const qlonglong theLength, const QUrl &theUrl, const EnclosurePlatform thePlatform, const QByteArray &theSignature, const EnclosureSignatureType theSignatureType) {

  ItemEnclosure* enclosure = ItemEnclosure::NewEnclosure(theLength, versionBuild, versionDescription, theUrl, thePlatform, theSignature, theSignatureType);

  if (enclosure != nullptr) {
    enclosures.append(enclosure);
  }

  return enclosure;
}

ItemDelta* AppcastItem::AddDelta(const qlonglong prevBuildVersion, const qlonglong theLength, const QUrl& theUrl, const EnclosurePlatform thePlatform, const QByteArray& theSignature, const EnclosureSignatureType theSignatureType) {

  ItemDelta* delta = ItemDelta::NewDelta(theLength, prevBuildVersion, versionBuild, versionDescription, theUrl, thePlatform, theSignature, theSignatureType);

  if (delta != nullptr) {
    deltas.append(delta);
  }

  return delta;
}
