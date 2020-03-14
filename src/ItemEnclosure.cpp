//
//  ItemEnclosure.cpp
//  sparkless
//
//  Created by Kyle King on 2020-03-14.
//  Copyright © 2020 Kyle King. All rights reserved.
//

#include "ItemEnclosure.hpp"

#include <QDebug>

#pragma mark - Constructors -

#pragma mark Protected

ItemEnclosure::ItemEnclosure(const QDomElement& theEnclosureElement, QObject* theParent)
: QObject(theParent) {

  enclosureElement = theEnclosureElement;
}

#pragma mark Public

ItemEnclosure* ItemEnclosure::FromElement(const QDomElement& theEnclosureElement, QObject* theParent) {

  ItemEnclosure* enclsoure = new ItemEnclosure(theEnclosureElement, theParent);

  if (!enclsoure->ParseXml()) {
    delete enclsoure;
    enclsoure = nullptr;
  }

  return enclsoure;
}

ItemEnclosure::~ItemEnclosure() {

}


#pragma mark - Accessors -

#pragma mark Private

#pragma mark Public

EnclosurePlatform ItemEnclosure::PlatformFromString(const QString& theString) {

  EnclosurePlatform platform = NullPlatform;

  if (theString.toLower() == "macos") {
    platform = MacPlatform;
  }
  else if (theString.toLower() == "windows" || theString.toLower() == "windows-x86" || theString.toLower() == "windows-x64") {
    platform = WindowsPlatform;
  }

  return platform;
}

QString ItemEnclosure::PlatformToString(const EnclosurePlatform& thePlatform) {

  switch (thePlatform) {
    case MacPlatform: {
      return "macos";
    }
    case WindowsPlatform: {
      return "windows";
    }
    default: {
      return "";
    }
  }
}

QString ItemEnclosure::PlatformToDescription(const EnclosurePlatform& thePlatform) {

  switch (thePlatform) {
    case MacPlatform: {
      return "macOS";
    }
    case WindowsPlatform: {
      return "Windows";
    }
    default: {
      return "";
    }
  }
}

void ItemEnclosure::Print() const {

//  qDebug() << "ItemEnclosure::Print()";

  qDebug() << QString("File Url: %1").arg(fileUrl.toString(), 20);
  qDebug().noquote().nospace() << fileUrl.path().section('/', -1);
}



#pragma mark - Mutators -

#pragma mark Private

bool ItemEnclosure::ParseXml() {

  versionDescription = enclosureElement.attribute("sparkle:shortVersionString");
  versionBuild = enclosureElement.attribute("sparkle:version").toLongLong();

  fileUrl = QUrl(enclosureElement.attribute("url"));
  mimeType = enclosureElement.attribute("type");
  length = enclosureElement.hasAttribute("length") ? enclosureElement.attribute("length").toLongLong() : -1;
  if (enclosureElement.hasAttribute("sparkle:dsaSignature")) {
    signature = enclosureElement.attribute("sparkle:dsaSignature").toUtf8();
  }
  else if (enclosureElement.hasAttribute("sparkle:edSignature")) {
    signature = enclosureElement.attribute("sparkle:edSignature").toUtf8();
  }
  else {
    qWarning() << "ItemEnclosure::ParseXml() warning - enclosure is missing signature: " << enclosureElement.toDocument().toString();
  }

  platform = PlatformFromString(enclosureElement.attribute("sparkle:os"));

  return true;
}

#pragma mark Public
