//
//  main.cpp
//  Sparkless
//
//  Created by Kyle King on 2018-5-20.
//  Copyright © 2018 Divide Software Inc. All rights reserved.
//

#include <QCoreApplication>
//#include <unistd.h>

#include "Appcast.hpp"
#include "AppcastItem.hpp"
#include "ItemEnclosure.hpp"
#include "utils/DeltaGenerator.hpp"
#include "utils/DmgMounter.hpp"
#include "utils/DsaSignatureGenerator.hpp"
#include "utils/EdDsaSignatureGenerator.hpp"

#include <QCommandLineParser>
#include <QFile>
#include <QDomDocument>
#include <QDebug>

int main(int argc, char *argv[]) {

  QCoreApplication a(argc, argv);
  a.setApplicationName("Sparkless");
  a.setApplicationVersion("2.0.0");


  /* -------- Command Line Specs -------- */

  QCommandLineParser parser;
  parser.setApplicationDescription("Appcast generator for Sparkle");

  parser.addPositionalArgument("command", "the command to run", "add|print|sign|delta|help");
  parser.addHelpOption();

  /* ---- options used in multiple commands ---- */

  QCommandLineOption appcastOption("appcast", "The local file path to the appcast xml", "appcast_path");

  /* ---- add ---- */

  QCommandLineOption versionStringOption("version", "The descriptive (string) version for the new bundle", "version");
  QCommandLineOption versionBuildOption("build", "The build number/version for the new bundle", "build_number");

  QCommandLineOption macBundleOption("mac-bundle", "The local file path to the mac app/dmg/zip bundle", "bundle_path");
  QCommandLineOption windowsBundleOption("windows-bundle", "The local file path to the windows msi/exe/zip bundle", "bundle_path");

  QCommandLineOption deltasOption("deltas", "The number of delta updates to generate, without specifying this deltas will NOT be generated", "num_deltas");

  QCommandLineOption edDsaKeyOption("eddsa-key", "The Ed25519 key used for signing (the key is passed in-line, not by filepath) [required for macOS delta updates]", "key");
  QCommandLineOption dsaKeyFilePathOption("dsa-key-path", "The local file path to the dsa key used for signing [required for windows bundles]", "key_path");

  QCommandLineOption s3RegionOption("s3-region", "The s3 region (used for url generation)", "region");
  QCommandLineOption s3BucketOption("s3-bucket", "The s3 bucket (used for url generation)", "bucket_name");
  QCommandLineOption s3BucketDirOption("s3-bucket-dir", "The diectory inside the s3 bucket (used for url generation)", "bucket_dir");
  QCommandLineOption s3MirrorPathOption("s3-mirror-path", "The file path to the local mirror of the s3 bucket dir [required for automatic delta generation, requires other s3 options to be set]", "num_deltas");

  QCommandLineOption urlPrefixOption("url-prefix", "The url (without the filename) to be used for the appcast URL generation. This is an alternative ", "url_without_filename");

  /* ---- delta ---- */

  QCommandLineOption previousBundleOption("prev-bundle", "The local file path to the previous app/dmg/zip [required for delta command]", "bundle_path");
  QCommandLineOption deltaPathOption("delta-path", "The local file path for the output delta file [required for delta command]", "delta_path");


  // add options
  if (qApp->arguments().contains("add")) {
    parser.addOptions({
      appcastOption,
      versionStringOption, versionBuildOption,
      macBundleOption, windowsBundleOption,
      deltasOption,
      edDsaKeyOption, dsaKeyFilePathOption,
      s3RegionOption, s3BucketOption, s3BucketDirOption, s3MirrorPathOption,
      urlPrefixOption,
    });

  }
  // print optinos
  else if (qApp->arguments().contains("print")) {
    parser.addOption(appcastOption);
  }
  // sign options
  else if (qApp->arguments().contains("sign")) {
    parser.addOptions({
      macBundleOption, windowsBundleOption,
      edDsaKeyOption, dsaKeyFilePathOption,
    });
  }
  // delta options
  else if (qApp->arguments().contains("delta")) {
    parser.addOptions({
      macBundleOption,
      previousBundleOption,
      deltaPathOption
    });
  }

  parser.process(a);

  QString command;

  if (!parser.positionalArguments().isEmpty()) {
    command = parser.positionalArguments().first();
  }

  /* ---- Print ---- */
  if (command == "print") {

    parser.addOption(appcastOption);
//    parser.process(a);

    if (!parser.isSet(appcastOption)) {
      qCritical().noquote().nospace() << "Missing required option '--"<<appcastOption.names().first()<<"'.";
      return 1;
    }

    const QString appcastPath = parser.value(appcastOption);
    Appcast* appcast = Appcast::FromPath(appcastPath);
    if (appcast == nullptr) {
      return 1;
    }

    appcast->PrintItems();
    return 0;
  }

  /* ---- Sign ---- */
  else if (command == "sign") {

    bool hasMacBundle = parser.isSet(macBundleOption);
    bool hasWindowsBundle = parser.isSet(windowsBundleOption);
    bool hasEdDsaKey = parser.isSet(edDsaKeyOption);
    bool hasDsaKeyPath = parser.isSet(dsaKeyFilePathOption);

    if (!hasMacBundle && !hasWindowsBundle) {
      qCritical().noquote().nospace() << "`sign` requires '--"<<macBundleOption.names().first()<<"' and/or '--"<<windowsBundleOption.names().first()<<"'.";
      return 1;
    }

    if (!hasEdDsaKey && !hasDsaKeyPath) {
      qCritical().noquote().nospace() << "`sign` requires either '--"<<edDsaKeyOption.names().first()<<"' and/or '--"<<dsaKeyFilePathOption.names().first()<<"'.";
      return 1;
    }

    if (hasWindowsBundle && !hasDsaKeyPath) {
      qCritical().noquote().nospace() << "Windows bundles require a dsa signature. Please specify one with '--"<<dsaKeyFilePathOption.names().first()<<"'.";
      return 1;
    }

    const QString macBundlePath = hasMacBundle ? parser.value(macBundleOption) : QString();
    const QString windowsBundlePath = hasWindowsBundle ? parser.value(windowsBundleOption) : QString();
    const QByteArray edDsaKey = hasEdDsaKey ? parser.value(edDsaKeyOption).toUtf8() : QByteArray();
    const QString dsaKeyPath = hasDsaKeyPath ? parser.value(dsaKeyFilePathOption) : QString();

    if (hasMacBundle) {

      if (hasEdDsaKey) {
        EdDsaSignatureGenerator sigGenerator(macBundlePath, edDsaKey);
        if (!sigGenerator.Success()) {
          return 1;
        }
        printf("\n%s [Ed25519]: %s\n", macBundlePath.section('/', -1).toUtf8().constData(), sigGenerator.Signature().constData());
      }
      else if (hasDsaKeyPath) {
        qDebug() << "dsa key path: " << dsaKeyPath;
        DsaSignatureGenerator sigGenerator(macBundlePath, dsaKeyPath);
        if (!sigGenerator.Success()) {
          return 1;
        }
        printf("\n%s [DSA]: %s\n", macBundlePath.section('/', -1).toUtf8().constData(), sigGenerator.Signature().constData());
      }
    }

    if (hasWindowsBundle) {
      DsaSignatureGenerator sigGenerator(windowsBundlePath, dsaKeyPath);
      if (!sigGenerator.Success()) {
        return 1;
      }
      printf("\n%s [DSA]: %s\n", windowsBundlePath.section('/', -1).toUtf8().constData(), sigGenerator.Signature().constData());
    }

    return 0;
  }

  /* ---- delta ---- */
  else if (command == "delta") {

    if (!parser.isSet(macBundleOption)) {
      qCritical().noquote().nospace() << "`delta` requires '--"<<macBundleOption.names().first()<<"'.";
      return 1;
    }
    if (!parser.isSet(previousBundleOption)) {
      qCritical().noquote().nospace() << "`delta` requires '--"<<previousBundleOption.names().first()<<"'.";
      return 1;
    }
    if (!parser.isSet(deltaPathOption)) {
      qCritical().noquote().nospace() << "`delta` requires '--"<<deltaPathOption.names().first()<<"'.";
      return 1;
    }

    const QString macBundlePath = parser.value(macBundleOption);
    const QString previousBundlePath = parser.value(previousBundleOption);
    const QString deltaPath = parser.value(deltaPathOption);

    DeltaGenerator deltaGenerator(previousBundlePath, macBundlePath, deltaPath);
    if (!deltaGenerator.Success()) {
      return 1;
    }


    printf("\ndelta generated: %s\n", deltaPath.toUtf8().constData());
    return 0;
  }

  /* ---- Add ---- */
  else if (command == "add") {

//    parser.addPositionalArgument("command", "The command to run");

//    parser.process(a);

    bool hasMacBundle = parser.isSet(macBundleOption);
    bool hasWindowsBundle = parser.isSet(windowsBundleOption);
    bool hasEdDsaKey = parser.isSet(edDsaKeyOption);
    bool hasDsaKeyPath = parser.isSet(dsaKeyFilePathOption);

    if (!hasMacBundle && !hasWindowsBundle) {
      qCritical().noquote().nospace() << "`add` requires '--"<<macBundleOption.names().first()<<"' and/or '--"<<windowsBundleOption.names().first()<<"'.";
      return 1;
    }

    if (!hasEdDsaKey && !hasDsaKeyPath) {
      qCritical().noquote().nospace() << "`add` requires either '--"<<edDsaKeyOption.names().first()<<"' and/or '--"<<dsaKeyFilePathOption.names().first()<<"'.";
      return 1;
    }

    if (hasWindowsBundle && !hasDsaKeyPath) {
      qCritical().noquote().nospace() << "Windows bundles require a DSA signature. Please specify one with '--"<<dsaKeyFilePathOption.names().first()<<"'.";
      return 1;
    }

    if (!parser.isSet(versionBuildOption)) {
      qCritical().noquote().nospace() << "`add` requires '--"<<versionBuildOption.names().first()<<"'.";
      return 1;
    }
    if (!parser.isSet(versionStringOption)) {
      qCritical().noquote().nospace() << "`add` requires '--"<<versionStringOption.names().first()<<"'.";
      return 1;
    }
    if (!parser.isSet(urlPrefixOption) && (!parser.isSet(s3RegionOption) || !parser.isSet(s3BucketOption))) {
      qCritical().noquote().nospace() << "`add` requires either '--"<<urlPrefixOption.names().first()<<"' or '--"<<s3RegionOption.names().first()<<"' and '--"<<s3BucketOption.names().first()<<"'.";
      return 1;
    }

    // validate delta requiremements
    if (parser.isSet(deltasOption)) {
      if (!hasMacBundle) {
        qCritical().noquote().nospace() << "`add` option '--"<<deltasOption.names().first()<<"' is only available for mac bundles'";
        return 1;
      }
      if (!hasEdDsaKey) {
        qCritical().noquote().nospace() << "`add` option '--"<<deltasOption.names().first()<<"' requires a EdDsa signature. Please specify one with '--"<<edDsaKeyOption.names().first()<<"'.";
        return 1;
      }
      if (!parser.isSet(s3MirrorPathOption)) {
        qCritical().noquote().nospace() << "`add` option '--"<<deltasOption.names().first()<<"' requires a local s3 mirror path. Please specify one with '--"<<s3MirrorPathOption.names().first()<<"'.";
        return 1;
      }
    }

    const int deltasCount = parser.value(deltasOption).toInt();
    if (QString::number(deltasCount) != parser.value(deltasOption)) {
      qCritical().nospace().noquote() << "invalid value for option '--"<<deltasOption.names().first()<<"'. Please specify a number > 0'";
      return 1;
    }
    const QString versionString = parser.value(versionStringOption);
    const qlonglong versionBuild = parser.value(versionBuildOption).toLongLong();

    const QString macBundlePath = hasMacBundle ? parser.value(macBundleOption) : QString();
    const QString windowsBundlePath = hasWindowsBundle ? parser.value(windowsBundleOption) : QString();

    const QByteArray edDsaKey = hasEdDsaKey ? parser.value(edDsaKeyOption).toUtf8() : QByteArray();
    const QString dsaKeyPath = hasDsaKeyPath ? parser.value(dsaKeyFilePathOption) : QString();

    const QString appcastPath = parser.value(appcastOption);
    Appcast* appcast = Appcast::FromPath(appcastPath);
    if (appcast == nullptr) {
      return 1;
    }

    if (parser.isSet(urlPrefixOption)) {
      appcast->SetUrlPrefix(parser.value(urlPrefixOption));
    }
    else {
      appcast->SetS3Region(parser.value(s3RegionOption));
      appcast->SetS3BucketName(parser.value(s3BucketOption));
      if (parser.isSet(s3BucketDirOption)) {
        appcast->SetS3BucketDir(parser.value(s3BucketDirOption));
      }
      if (parser.isSet(s3MirrorPathOption)) {
        appcast->SetS3LocalMirrorPath(parser.value(s3MirrorPathOption));
      }
    }

    AppcastItem* newItem = appcast->CreateItem(versionString, versionBuild);

    if (hasMacBundle) {
      if (hasEdDsaKey) {

        ItemEnclosure* newMacEnclosure = appcast->AddEnclosureToIem(newItem, macBundlePath, MacPlatform, edDsaKey);
        if (newMacEnclosure == nullptr) { qWarning().noquote().nospace() << "failed to add mac enclosure"; return 1; }

        if (deltasCount >= 1) {

          qInfo().noquote().nospace() << "\nGenerating deltas for build " << versionBuild << "...\n";

          int deltasCreated = 0;
          qlonglong currBuildNumber = newItem->VersionBuild() - 1;

          while (deltasCreated < deltasCount && currBuildNumber > 0) {

            ItemDelta* newDelta = appcast->CreateDeltaForBuild(currBuildNumber, macBundlePath, newItem, MacPlatform, edDsaKey);
            if (newDelta != nullptr) {
              deltasCreated++;
            }
            currBuildNumber--;
          }
        }
      }
      else if (hasDsaKeyPath) {
        ItemEnclosure* newMacEnclosure = appcast->AddEnclosureToIem(newItem, macBundlePath, MacPlatform, dsaKeyPath);
        if (newMacEnclosure == nullptr) { qWarning().noquote().nospace() << "failed to add mac enclosure"; return 1; }
      }
    }
    if (hasWindowsBundle) {
      if (hasDsaKeyPath) {
        ItemEnclosure* newWindowsEnclosure = appcast->AddEnclosureToIem(newItem, windowsBundlePath, WindowsPlatform, dsaKeyPath);
        if (newWindowsEnclosure == nullptr) { qWarning().noquote().nospace() << "failed to add windows enclosure"; return 1; }
      }
    }

    qInfo().noquote().nospace() << "\nSaving updated appcast file...";
    if (!appcast->AddItem(newItem)) { qWarning().noquote().nospace() << "failed to add item to appcast xml"; return 1; }
    if (!appcast->Save(appcastPath)) { qWarning().noquote().nospace() << "failed to save appcast xml"; return 1; }

    newItem->Print();
    
    return 0;
  }

  else {

    bool unknownCommand = (!command.isEmpty() && command != "help");
    if (unknownCommand) {
      printf("Unknown command: %s\n", command.toUtf8().constData());
    }

    printf("\nTo print available options for a specific command, run `sparkless [command] -h`\n");
    printf("\nAvailable commands:\n");
    printf("  add         Add a bundle to an existing appcast file\n");
    printf("  sign        Generates a signature for a bundle\n");
    printf("  delta       Generates deltas for a bundle\n");
    printf("  print       Print the contents of an existing appcast file\n");
    printf("  help        Print usage\n");
    printf("\n");

    if (!unknownCommand) {
      parser.showHelp();
    }
    return 1;
  }

  return 1;
}
