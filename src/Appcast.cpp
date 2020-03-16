//
//  Appcast.cpp
//  sparkless
//
//  Created by Kyle King on 2020-03-14.
//  Copyright © 2020 Kyle King. All rights reserved.
//

#include "Appcast.hpp"

#include <QDebug>
#include <QFile>
#include <QFileInfo>

#include "AppcastItem.hpp"
#include "ItemEnclosure.hpp"
#include "utils/DsaSignatureGenerator.hpp"
#include "utils/EdDsaSignatureGenerator.hpp"

#pragma mark - Constructors -

#pragma mark Private

Appcast::Appcast(const QDomDocument& theXmlDoc, QObject* theParent)
: QObject(theParent) {

  appcastDoc = theXmlDoc;
}


#pragma mark Public

Appcast* Appcast::FromDocument(const QDomDocument& theXmlDoc, QObject* theParent) {

  Appcast* appcast = new Appcast(theXmlDoc, theParent);

  if (!appcast->ParseXml()) {
    delete appcast;
    appcast = nullptr;
  }

  return appcast;
}

Appcast* Appcast::FromPath(const QString& theFilePath, QObject* theParent) {

  Appcast* appcast  = nullptr;
  QDomDocument appcastDoc;

  QFile appcastFile(theFilePath);

  if (!appcastFile.exists()) {
    qWarning().noquote().nospace() << "error - appcast file doesn't exist: " << theFilePath;
  }
  else if (!appcastFile.open(QIODevice::ReadOnly| QIODevice::Text)) {
    qWarning() << "error opening appcast file for reading: " << theFilePath;
  }
  else {
    if (!appcastDoc.setContent(&appcastFile)) {
      qWarning() << "error parsing appcast file xml: " << theFilePath;
    }

    appcastFile.close();
  }

  if (!appcastDoc.isNull()) {

    appcast = new Appcast(appcastDoc, theParent);

    if (!appcast->ParseXml()) {
      delete appcast;
      appcast = nullptr;
    }
  }

  return appcast;
}

Appcast::~Appcast() {

  qDeleteAll(items);
}


#pragma mark - Accessors -

#pragma mark Private

#pragma mark Public

AppcastItem* Appcast::Item(const qlonglong theBuildVersion) const {

  if (theBuildVersion < 0) {
    return nullptr;
  }

  return itemHash.value(theBuildVersion);
}

bool Appcast::Contains(const qlonglong theBuildVersion) const {

  return itemHash.value(theBuildVersion) != nullptr;
}

bool Appcast::ContainsEnclosure(const qlonglong theBuildVersion, const EnclosurePlatform thePlatform) const {

  AppcastItem* item = Item(theBuildVersion);
  if (item != nullptr) {

    return item->HasEnclosure(thePlatform);
  }

  return false;
}

const QString Appcast::UrlForRelease(const QString& theReleaseFileName, const EnclosurePlatform thePlatform) const {

  if (!urlPrefix.isEmpty()) {
    return QString("%1/%2").arg(urlPrefix, theReleaseFileName);
  }

  if (!s3Region.isEmpty() && !S3BucketName().isEmpty()) {


    const QString s3Endpoint = QString("https://s3-%1.amazonaws.com").arg(s3Region);

    QString path = "/" + s3BucketName;

    if (!s3BucketDir.isEmpty()) {
      path.append(s3BucketDir);
    }

    QString platformName = ItemEnclosure::PlatformToDescription(thePlatform);

    if (!platformName.isEmpty()) {
      path.append("/" + platformName);
    }

    return s3Endpoint + path + "/" + theReleaseFileName;
  }

  return QString();
}

void Appcast::PrintItems() const {

  foreach (AppcastItem* currItem, items) {

    if (currItem != nullptr) {
      currItem->Print();
    }
  }
}

#pragma mark - Mutators -

#pragma mark Private

bool Appcast::ParseXml() {

  if (appcastDoc.isNull()) {
    return false;
  }
  
  QDomElement rssElement = appcastDoc.firstChildElement("rss");
  QDomElement channelElement = rssElement.firstChildElement("channel");

  QDomElement titleElement = channelElement.firstChildElement("title");
  title = titleElement.firstChild().nodeValue();

  QDomElement itemElement = channelElement.firstChildElement("item");

  while (!itemElement.isNull()) {

    AppcastItem* item = AppcastItem::FromElement(itemElement, this);

    if (item != nullptr) {

      items.append(item);

      if (item->VersionBuild() >= 0) {
        itemHash.insert(item->VersionBuild(), item);
      }
    }

    itemElement = itemElement.nextSiblingElement("item");
  }

  return true;
}

ItemEnclosure* Appcast::AddEnclosureToItemWithSignature(AppcastItem* theItem, const QString& theFilePath, const EnclosurePlatform thePlatform, const QByteArray& theSignature, const EnclosureSignatureType theSignatureType) {

  qDebug() << "AddEnclosureToItemWithSignature("<<theFilePath<<")";

  if (theItem == nullptr) {
    qWarning().noquote().nospace() << "error adding enclosure to item - item is NULL";
    return nullptr;
  }

  QFileInfo fileInfo(theFilePath);

  const qlonglong fileLength = fileInfo.size();
  const QString fileName = fileInfo.fileName();
  const QUrl fileUrl = UrlForRelease(fileName, thePlatform);

  ItemEnclosure* enclosure = theItem->AddEnclosure(fileLength, fileUrl, thePlatform, theSignature, theSignatureType);
  
  return enclosure;
}

#pragma mark Public

void Appcast::SetS3Region(const QString& theS3Region) {

  s3Region = theS3Region;
}

void Appcast::SetS3BucketName(const QString& theS3BucketName) {

  s3BucketName = theS3BucketName;
}

void Appcast::SetS3BucketDir(const QString& theS3BucketDir) {

  s3BucketDir = theS3BucketDir;
}

void Appcast::SetS3LocalMirrorPath(const QString& theS3LocalMirrorPath) {

  s3LocalMirrorPath = theS3LocalMirrorPath;
}

void Appcast::SetUrlPrefix(const QString& theUrlPrefix) {

  urlPrefix = theUrlPrefix;
}

AppcastItem* Appcast::CreateItem(const QString& theVersionDescription, const qlonglong theVersionBuild) {

  AppcastItem* newItem = AppcastItem::NewItem(theVersionDescription, theVersionBuild, this);
  if (newItem != nullptr) {
    newItem->SetTitle(title);
  }

  return newItem;
}

ItemEnclosure* Appcast::AddEnclosureToIem(AppcastItem* theItem, const QString& theFilePath, const EnclosurePlatform thePlatform, const QString& theDsaKeyPath) {

  DsaSignatureGenerator signatureGenerator(theFilePath, theDsaKeyPath);
  if (!signatureGenerator.Success()) {
    qWarning().noquote().nospace() << "error adding enclosure to item - failed to generate DSA signature";
    return nullptr;
  }

  return AddEnclosureToItemWithSignature(theItem, theFilePath, thePlatform, signatureGenerator.Signature(), DsaSignature);
}

ItemEnclosure* Appcast::AddEnclosureToIem(AppcastItem* theItem, const QString& theFilePath, const EnclosurePlatform thePlatform, const QByteArray& theEdDsaKey) {

  EdDsaSignatureGenerator signatureGenerator(theFilePath, theEdDsaKey);
  if (!signatureGenerator.Success()) {
    qWarning().noquote().nospace() << "error adding enclosure to item - failed to generate EdDSA signature";
    return nullptr;
  }

  return AddEnclosureToItemWithSignature(theItem, theFilePath, thePlatform, signatureGenerator.Signature(), Ed25519Signature);
}



bool Appcast::Save(const QString& theFilePath) {

  if (appcastDoc.isNull()) {
    return false;
  }

  if (theFilePath.isEmpty()) {
    qWarning().noquote().nospace() << "error saving appcast - specified save path is empty in Save() method";
    return false;
  }

  QFile appcastFile(theFilePath);

  if (!appcastFile.open(QIODevice::ReadWrite| QIODevice::Truncate)) {
    qWarning() << "error opening appcast file for saving: " << theFilePath;
    return false;
  }

  QTextStream textStream(&appcastFile);
  appcastDoc.save(textStream, 0);
  appcastFile.close();

  qInfo().noquote().nospace() << "successfully saved appcast file: " << theFilePath;

  return true;
}

bool Appcast::AddItem(AppcastItem* theItem) {

  if (theItem == nullptr) { qWarning() << "Appcast::AddItem() failed - specified item is NULL"; return false; }
  if (theItem->Title().isEmpty()) { qWarning() << "Appcast::AddItem() failed - item's title is empty"; return false; }
  if (theItem->PublishedTimestamp().isNull()) { qWarning() << "Appcast::AddItem() failed - item's published timestamp is null"; return false; }


  QDomElement itemElement = appcastDoc.createElement("item");

  {
    QDomElement itemTitleElement = appcastDoc.createElement("title");
    QDomText itemTitleValue = appcastDoc.createTextNode(theItem->Title());
    itemTitleElement.appendChild(itemTitleValue);
    itemElement.appendChild(itemTitleElement);
  }

  {
    QDomElement itemPublishedDateElement = appcastDoc.createElement("pubDate");
    QDomText itemPublishedDateValue = appcastDoc.createTextNode(theItem->PublishedTimestampString());
    itemPublishedDateElement.appendChild(itemPublishedDateValue);
    itemElement.appendChild(itemPublishedDateElement);
  }

  if (!theItem->Description().isEmpty()) {
    QDomElement itemDescriptionElement = appcastDoc.createElement("description");
    QDomText itemDescriptionValue = appcastDoc.createTextNode(theItem->Description());
    itemDescriptionElement.appendChild(itemDescriptionValue);
    itemElement.appendChild(itemDescriptionElement);
  }

  if (!theItem->ReleaseNotesUrl().isEmpty()) {
    QDomElement itemReleaseNotesElement = appcastDoc.createElement("sparkle:releaseNotesLink");
    QDomText itemReleaseNotesValue = appcastDoc.createTextNode(theItem->ReleaseNotesUrl().toString());
    itemReleaseNotesElement.appendChild(itemReleaseNotesValue);
    itemElement.appendChild(itemReleaseNotesElement);
  }

  foreach (ItemEnclosure* currEnclosure, theItem->Enclosures()) {

    if (currEnclosure == nullptr) { qWarning() << "Appcast::AddItem() failed - the item has a null enclosure object"; return false; }

    QDomElement itemEnclosureElement = appcastDoc.createElement("enclosure");
    if (currEnclosure->Serialize(itemEnclosureElement)) {
      itemElement.appendChild(itemEnclosureElement);
    }
  }

  QDomElement rssElement = appcastDoc.firstChildElement("rss");
  QDomElement channelElement = rssElement.firstChildElement("channel");

  channelElement.insertAfter(itemElement, channelElement.firstChildElement("language"));

  return true;
}

#pragma mark Public
