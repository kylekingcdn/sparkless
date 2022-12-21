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
#include <QDir>

#include "AppcastItem.hpp"
#include "ItemEnclosure.hpp"
#include "ItemDelta.hpp"
#include "utils/DsaSignatureGenerator.hpp"
#include "utils/EdDsaSignatureGenerator.hpp"
#include "utils/DeltaGenerator.hpp"
#include "utils/DmgMounter.hpp"

Appcast::Appcast(const QDomDocument& theXmlDoc, QObject* theParent)
: QObject(theParent) {

  appcastDoc = theXmlDoc;
}

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

QString Appcast::TemporaryMountDirForBuild(const qlonglong theBuildNumber) {

  return QString("/tmp/sparkless/%1").arg(theBuildNumber);
}

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

const QString Appcast::S3BaseUrl() const {

  QString s3Endpoint;

  if (!s3Region.isEmpty() && !S3BucketName().isEmpty()) {

    s3Endpoint = QString("https://s3-%1.amazonaws.com/%2").arg(s3Region).arg(s3BucketName);

    if (!s3BucketDir.isEmpty()) {
      s3Endpoint.append(s3BucketDir);
    }
  }

  return s3Endpoint;
}

const QString Appcast::UrlForRelease(const QString& theReleaseFileName, const EnclosurePlatform thePlatform) const {

  // FIXME: broken
  if (!urlPrefix.isEmpty()) {
    return QString("%1/%2").arg(urlPrefix, theReleaseFileName);
  }

  if (!s3Region.isEmpty() && !S3BucketName().isEmpty()) {

    const QString s3Endpoint = S3BaseUrl();
    const QString platformName = ItemEnclosure::PlatformToDescription(thePlatform);

    return QString("%1/%2/%3").arg(s3Endpoint).arg(platformName).arg(theReleaseFileName);
  }

  return QString();
}

const QString Appcast::UrlForDelta(const QString& theDeltaFileName, const qlonglong theNewBuildVersion, const EnclosurePlatform thePlatform) const {

  // FIXME: broken
  if (!urlPrefix.isEmpty()) {
    return QString("%1/%2").arg(urlPrefix, theDeltaFileName);
  }

  if (!s3Region.isEmpty() && !S3BucketName().isEmpty()) {

    const QString s3Endpoint = S3BaseUrl();
    const QString platformName = ItemEnclosure::PlatformToDescription(thePlatform);

    return QString("%1/%2/deltas/%3/%4").arg(s3Endpoint).arg(platformName).arg(theNewBuildVersion).arg(theDeltaFileName);
  }

  return QString();
}

const QString Appcast::MapRemoteUrlToLocalMirrorPath(const QString& theUrl) const {

  return QString(theUrl).replace(S3BaseUrl(), s3LocalMirrorPath);
}

const QString Appcast::MapLocalMirrorPathToRemoteUrl(const QString& theMirrorPath) const {

  return QString(theMirrorPath).replace(s3LocalMirrorPath, S3BaseUrl());
}

void Appcast::PrintItems() const {

  foreach (AppcastItem* currItem, items) {

    if (currItem != nullptr) {
      currItem->Print();
//      qDebug() << MapRemoteUrlToLocalMirrorPath(currItem->Enclosures().first()->FileUrl().toString());
    }
  }
}

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

//  qDebug() << "AddEnclosureToItemWithSignature("<<theFilePath<<")";

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

ItemDelta* Appcast::CreateDeltaForBuild(const qlonglong theOldBuildNumber, const QString theNewReleasePath, AppcastItem* theNewItem, const EnclosurePlatform thePlatform, const QByteArray& theEdDsaKey) {

  Q_ASSERT(theNewItem != nullptr);
  Q_ASSERT(!theNewReleasePath.isEmpty());
  Q_ASSERT(theOldBuildNumber >= 0);
  if (theNewItem != nullptr) {
    Q_ASSERT(theNewItem->VersionBuild() >= 0);
  }
  Q_ASSERT(thePlatform != NullPlatform);

  // temp. enforce .dmg only
  if (!theNewReleasePath.toLower().endsWith(".dmg")) {
    return nullptr;
  }

  // temp. enforce deltas on macOS only
  if (thePlatform != MacPlatform) {
    return nullptr;
  }

  ItemDelta* newDelta = nullptr;

  const qlonglong newBuildNumber = theNewItem->VersionBuild();

  const QString newReleaseMountPoint = TemporaryMountDirForBuild(newBuildNumber);
  QDir().mkpath(newReleaseMountPoint);

//  qDebug() << "mounting new release '" << theNewReleasePath << "' to '" << newReleaseMountPoint << "'.";

  DmgMounter newReleaseMounter(theNewReleasePath, newReleaseMountPoint);
  if (!newReleaseMounter.Mount()) {
    qWarning() << "failed to mount image for delta generation: " << newReleaseMounter.ImagePath();
    return nullptr;
  }

  AppcastItem* oldItem = Item(theOldBuildNumber);
  if (oldItem != nullptr) {

    ItemEnclosure* oldEnclosure = oldItem->Enclosure(thePlatform);
    if (oldEnclosure != nullptr && oldEnclosure->FileUrl().fileName().toLower().endsWith(".dmg")) {

      const QString oldReleasePath = MapRemoteUrlToLocalMirrorPath(oldEnclosure->FileUrl().toString());

      if (QFileInfo::exists(oldReleasePath)) {

        const QString oldReleaseMountPoint = TemporaryMountDirForBuild(theOldBuildNumber);
        QDir().mkpath(oldReleaseMountPoint);

//        qDebug() << "mounting old release '" << oldReleasePath << "' to '" << oldReleaseMountPoint << "'.";

        DmgMounter oldReleaseMounter(oldReleasePath, oldReleaseMountPoint);
        if (!oldReleaseMounter.Mount()) {
          qWarning() << "failed to mount image for delta generation: " << oldReleaseMounter.ImagePath();
          return nullptr;
        }

        qInfo().noquote().nospace() << "Generating delta for build " << theOldBuildNumber << " -> " << newBuildNumber << "...";

        const QString oldReleaseBundlePath = QString("%1/%2.app").arg(oldReleaseMountPoint, title);
        const QString newReleaseBundlePath = QString("%1/%2.app").arg(newReleaseMountPoint, title);
        const QString deltaDir = QString("%1/deltas/%2").arg(QFileInfo(theNewReleasePath).dir().absolutePath()).arg(newBuildNumber);
        const QString deltaFilename = QString("%1.%2.%3.delta").arg(title).arg(theOldBuildNumber).arg(newBuildNumber);
        const QString deltaPath = QString("%1/%2").arg(deltaDir).arg(deltaFilename);
//        qDebug() << "path for delta: " << deltaPath;

        QDir().mkpath(deltaDir);

        DeltaGenerator deltaGenerator(oldReleaseBundlePath, newReleaseBundlePath, deltaPath);
        if (!deltaGenerator.Success()) {
          qFatal("failed to make delta: %s", deltaPath.toUtf8().constData());
        }

        newReleaseMounter.Unmount();
        oldReleaseMounter.Unmount();

        newDelta = AddDeltaToIem(theNewItem, theOldBuildNumber, deltaPath, theEdDsaKey);
      }
    }
  }

  return newDelta;
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

ItemDelta* Appcast::AddDeltaToIem(AppcastItem* theItem, const qlonglong thePrevVersion, const QString& theFilePath, const QByteArray& theEdDsaKey) {

  EdDsaSignatureGenerator signatureGenerator(theFilePath, theEdDsaKey);
  if (!signatureGenerator.Success()) {
    qWarning().noquote().nospace() << "error adding enclosure to item - failed to generate EdDSA signature";
    return nullptr;
  }

  QFileInfo fileInfo(theFilePath);

  const qlonglong fileLength = fileInfo.size();
  const QString fileName = fileInfo.fileName();
  const QUrl fileUrl = UrlForDelta(fileName, theItem->VersionBuild(), MacPlatform);
//  qDebug() << "delta url: " << fileUrl.toString();

  ItemDelta* delta = theItem->AddDelta(thePrevVersion, fileLength, fileUrl, MacPlatform, signatureGenerator.Signature(), Ed25519Signature);
  return delta;
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

  if (!theItem->Deltas().isEmpty()) {

    QDomElement deltasElement = appcastDoc.createElement("sparkle:deltas");

    foreach (ItemDelta* currDelta, theItem->Deltas()) {

      if (currDelta == nullptr) { qWarning() << "Appcast::AddItem() failed - the item has a null delta object"; return false; }

      // add <enclosure> to <sparkle:deltas>
      QDomElement deltaEnclosureElement = appcastDoc.createElement("enclosure");
      if (currDelta->Serialize(deltaEnclosureElement)) {
        deltasElement.appendChild(deltaEnclosureElement);
      }
    }

    // add <sparkle:deltas> to <item>
    if (!deltasElement.firstChildElement("enclosure").isNull()) {
      itemElement.appendChild(deltasElement);
    }
  }

  QDomElement rssElement = appcastDoc.firstChildElement("rss");
  QDomElement channelElement = rssElement.firstChildElement("channel");

  channelElement.insertAfter(itemElement, channelElement.firstChildElement("language"));

  return true;
}
