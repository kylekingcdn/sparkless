//
//  main.cpp
//  Sparkless
//
//  Created by Kyle King on 2018-5-20.
//  Copyright © 2018 Divide Software Inc. All rights reserved.
//

#include <QCoreApplication>

#include "enclosure.hpp"

#include <QCommandLineParser>
#include <QFile>
#include <QDomDocument>

QDomElement ItemNodeForBuildNumber(const int theVersion, QDomElement channelNode) {

  QDomElement itemNode = channelNode.firstChildElement("item");

  while (!itemNode.isNull()) {

    QDomElement enclosureNode = itemNode.firstChildElement("enclosure");
    if (!enclosureNode.isNull() && enclosureNode.attribute("sparkle:version").toInt() == theVersion) { return itemNode; }

    itemNode = itemNode.nextSiblingElement("item");
  }

  return QDomElement();
}

QDomElement EnclosureConflict(const Enclosure theEnclosure, const QDomElement channelNode) {

  QDomElement itemNode = channelNode.firstChildElement("item");

  while (!itemNode.isNull()) {

    QDomElement enclosureNode = itemNode.firstChildElement("enclosure");

    while (!enclosureNode.isNull()) {

      const int enclosureBuild = enclosureNode.attribute("sparkle:version").toInt();
      const QString enclosureOS = enclosureNode.attribute("sparkle:os");

      if (enclosureBuild == theEnclosure.BuildNumber() && enclosureOS == theEnclosure.SparklePlatformString()) { return enclosureNode; }

      enclosureNode = enclosureNode.nextSiblingElement("enclosure");
    }

    itemNode = itemNode.nextSiblingElement("item");
  }

  return QDomElement();
}

int main(int argc, char *argv[]) {

  QCoreApplication a(argc, argv);
  a.setApplicationName("Sparkless");
  a.setApplicationVersion("1.0.0");


  /* -------- Command Line Specs -------- */

  QCommandLineParser parser;
  parser.setApplicationDescription("Sparkle Appcast Maintainer");

  parser.addPositionalArgument("appcast", "local sparkle appcast.xml", "filepath");
  parser.addPositionalArgument("bucket", "S3 Bucket URL.", "url");
  parser.addPositionalArgument("version", "long version string");
  parser.addPositionalArgument("build", "build number");

  // optional
  QCommandLineOption macFileOption(QStringList{"mF","macFile"}, "The local path to the mac dmg file and the signature generated with dsa_priv.pem and the local bundle", "file");
  macFileOption.setValueName("bundlepath");
  parser.addOption(macFileOption);

  QCommandLineOption macSignatureOption(QStringList{"mS","macSig"}, "The signature generated generated with the mac dmg and dsa_priv.pem", "signature");
  macSignatureOption.setValueName("signature");
  parser.addOption(macSignatureOption);

  QCommandLineOption winFileOption(QStringList{"wF","winFile"}, "The local path to the windows setup exe file", "file");
  winFileOption.setValueName("bundlepath");
  parser.addOption(winFileOption);

  QCommandLineOption winSignatureOption(QStringList{"wS","winSig"}, "The signature generated generated with the windows setup exe and dsa_priv.pem", "signature");
  winSignatureOption.setValueName("signature");
  parser.addOption(winSignatureOption);

  QCommandLineOption forceOption(QStringList{"f","force"});
  parser.addOption(forceOption);

  parser.process(a);

  if (parser.positionalArguments().count() != 4) { qWarning() << "\nUsage:    sparkless appcast bucket version build [options]"; return 1; }


  /* -------- Initialize Variables -------- */

  const QString appcastPath = parser.positionalArguments().at(0);
  const QUrl bucket = parser.positionalArguments().at(1);
  const QString version = parser.positionalArguments().at(2);
  const int buildNumber = parser.positionalArguments().at(3).toInt();

  if (!bucket.isValid()) { qWarning("\nError - Malformed remote URL"); return 1; }
  if (QString::number(buildNumber) != parser.positionalArguments().at(3)) { qWarning("\nError parsing build number"); return 1; }

  const bool hasMac = parser.isSet(macFileOption) || parser.isSet(macSignatureOption);
  const bool hasWindows = parser.isSet(winFileOption) || parser.isSet(winSignatureOption);
  if (!hasMac && !hasWindows) { qWarning("\nError - either a mac or win bundle must be specified (or both)"); return 1; }

  const QString macPath = parser.value(macFileOption);
  const QString macSignature = parser.value(macSignatureOption);
  if (macPath.isEmpty() && !macSignature.isEmpty()) { qWarning("\nError - missing macFile"); return 1; }
  if (!macPath.isEmpty() && macSignature.isEmpty()) { qWarning("\nError - missing macSignature"); return 1; }

  const QString winPath = parser.value(winFileOption);
  const QString winSignature = parser.value(winSignatureOption);
  if (winPath.isEmpty() && !winSignature.isEmpty()) { qWarning("\nError - missing winFile"); return 1; }
  if (!winPath.isEmpty() && winSignature.isEmpty()) { qWarning("\nError - missing winSignature"); return 1; }

  const bool forceOverwrite = parser.isSet(forceOption);


  /* -------- Generate Appcast -------- */

  QFile appcastFile(appcastPath);
  if (!appcastFile.exists()) { qWarning() << "\nError - invalid appcast path:" << appcastPath; return 1; }
  if (!appcastFile.open(QIODevice::ReadOnly| QIODevice::Text)) { qWarning() << "\nError opening appcast file for read"; return 1; }

  QDomDocument appcastDoc;
  if (!appcastDoc.setContent(&appcastFile)) { qWarning() << "\nError parsing appcast xml"; appcastFile.close(); return 1; }
  appcastFile.close();

  QDomElement rssNode = appcastDoc.firstChildElement("rss");
  QDomElement channelNode = rssNode.firstChildElement("channel");

  const QString bundleName = channelNode.firstChildElement("title").firstChild().nodeValue();

  QList<Enclosure> enclosures;

  if (hasMac) { enclosures += Enclosure(Enclosure::Mac, macPath, macSignature, version, buildNumber, bucket); }
  if (hasWindows) { enclosures += Enclosure(Enclosure::Windows, winPath, winSignature, version, buildNumber, bucket); }

  // Create item node if it doesn't exist
  QDomElement itemNode = ItemNodeForBuildNumber(buildNumber, channelNode);

  if (itemNode.isNull()) {

    itemNode = appcastDoc.createElement("item");
    channelNode.insertAfter(itemNode, channelNode.firstChildElement("language"));

    QDomElement titleNode = appcastDoc.createElement("title");
    itemNode.appendChild(titleNode);
    titleNode.appendChild(appcastDoc.createTextNode(bundleName));

    // Todo: description support
//    QDomElement descriptionNode = appcastDoc.createElement("description");
//    itemNode.appendChild(descriptionNode);
//    descriptionNode.appendChild(appcastDoc.createTextNode("INSERT DESCRIPTION HERE"));

    const QUrl releaseNotesURL = QString("%1/release-notes/%2-%3.md").arg(bucket.toString()).arg(version).arg(buildNumber);
    QDomElement releaseNotesNode = appcastDoc.createElement("sparkle:releaseNotesLink");
    itemNode.appendChild(releaseNotesNode);
    releaseNotesNode.appendChild(appcastDoc.createTextNode(releaseNotesURL.toString()));

    QDomElement pubDateNode = appcastDoc.createElement("pubDate");
    itemNode.appendChild(pubDateNode);
    pubDateNode.appendChild(appcastDoc.createTextNode(QDateTime::currentDateTime().toString("ddd, dd MMM yy hh:mm:ss +0000")));
  }

  // check for duplicate build
  for (int i =0; i<enclosures.count(); i++) {

    Enclosure currEnclosure = enclosures[i];
    if (!currEnclosure.IsValid()) { qDebug() << "Errors:\n  " << currEnclosure.Errors().join("  \n"); return 1; }

    QDomElement newNode = currEnclosure.EnclosureNode(appcastDoc);
    QDomElement existingNode = EnclosureConflict(currEnclosure, channelNode);

    if (existingNode.isNull()) { itemNode.appendChild(newNode); }
    else if (forceOverwrite) { itemNode.replaceChild(newNode, existingNode); }
    else { qWarning().noquote() << "\nBuild" << buildNumber << "already exists for" << currEnclosure.PlatformString() << "\nTo supress this message and overwrite the current enclosure use option -f"; return 1; }

    currEnclosure.Print();
  }

  if (!appcastFile.open(QIODevice::ReadWrite| QIODevice::Truncate)) { qWarning() << "\nError opening appcast file for writing"; return 1; }

  QTextStream textStream(&appcastFile);
  appcastDoc.save(textStream, 0);
  appcastFile.close();

  return 0;
}
