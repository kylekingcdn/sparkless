//
//  enclosure.cpp
//  Crate
//
//  Created by Kyle King on 2018-5-21.
//  Copyright © 2016 Divide Software Inc. All rights reserved.
//

#include "enclosure.hpp"

#include <QFile>
#include <QFileInfo>
#include <QDebug>

Enclosure::Enclosure(const Platform theOS, const QString& theFile, const QString& theSignature, const QString theVersion, const int theBuild, const QUrl& theBucket)
  : os(theOS), filepath(theFile), signature(theSignature), version(theVersion), buildNumber(theBuild), bucketURL(theBucket) {

}

QString Enclosure::PlatformString(const Platform thePlatform) {

  switch (thePlatform) {
    case NullOS: { return ""; }
    case Mac: { return "mac"; }
    case Windows: { return "windows"; }
    case Linux: { return "linux"; }
  }
}

QString Enclosure::SparklePlatformString(const Platform thePlatform) {

  switch (thePlatform) {
    case NullOS: { return ""; }
    case Mac: { return "macos"; }
    case Windows: { return "windows"; }
    case Linux: { return "linux"; }
  }
}

QString Enclosure::Filename() const {

  return QFileInfo(filepath).fileName();
}

qint64 Enclosure::Length() const {

  return QFileInfo(filepath).size();
}

QUrl Enclosure::ReleaseURL() const {

  return QString("%1/releases/%2/%3").arg(bucketURL.toString()).arg(PlatformString()).arg(Filename());
}

//QUrl Enclosure::ReleaseNotesURL() const {

//  return QString("%1/release-notes/%2-%3.md").arg(bucketURL.toString()).arg(Version()).arg(BuildNumber());
//}

QDomElement Enclosure::EnclosureNode(QDomDocument& theDocument) const {

  if (!IsValid()) { return QDomElement(); }

  QDomElement node = theDocument.createElement("enclosure");
  node.setAttribute("sparkle:version", QString::number(buildNumber));
  node.setAttribute("sparkle:shortVersionString", version);
  node.setAttribute("sparkle:os", SparklePlatformString());
  node.setAttribute("url", ReleaseURL().toString());
  node.setAttribute("length", Length());
  node.setAttribute("sparkle:dsaSignature", signature);
  node.setAttribute("type", "application/octet-stream");

  return node;
}

QStringList Enclosure::Errors() const {

  QStringList list;

  if (os == NullOS) { list.append("Internal - platform variable is NULL"); }
  if (!QFileInfo(filepath).exists()) { list.append("Bundle does not exist at filepath"); }
  if (signature.isEmpty()) { list.append("Signature is empty"); }
  if (version.isEmpty()) { list.append("Version is empty"); }
  if (buildNumber <= 0) { list.append("Invalid Build Number: " + QString::number(buildNumber)); }
  if (!bucketURL.isValid()) { list.append("Invalid bucket URL"); }

  return list;
}

void Enclosure::Print() const {

  qDebug().noquote().nospace() << "\nAdding build to appcast:";
  qDebug().noquote().nospace() << "  OS: " << PlatformString();
  qDebug().noquote().nospace() << "  Local File: " << filepath;
  qDebug().noquote().nospace() << "  Signature: " << signature;
  qDebug().noquote().nospace() << "  Version: " << version;
  qDebug().noquote().nospace() << "  Build Number: " << buildNumber;
  qDebug().noquote().nospace() << "  Filename: " << Filename();
  qDebug().noquote().nospace() << "  Length: " << Length();
  qDebug().noquote().nospace() << "  Release URL: " << ReleaseURL().toString();
  //qDebug().noquote().nospace() << "  Release Notes URL: " << ReleaseNotesURL().toString();
  if (!IsValid()) { qDebug().noquote().nospace() << "Errors:\n  " << Errors().join("\n  "); }
  qDebug().noquote() << "";
}

