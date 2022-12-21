//
//  main.cpp
//  Sparkless
//
//  Created by Kyle King on 2018-5-20.
//  Copyright © 2018 Divide Software Inc. All rights reserved.
//

#include <QCoreApplication>
#include <QDir>

#include "Appcast.hpp"
#include "AppcastItem.hpp"
#include "ItemEnclosure.hpp"
#include "utils/DeltaGenerator.hpp"
#include "utils/DsaSignatureGenerator.hpp"
#include "utils/EdDsaSignatureGenerator.hpp"

#include <QCommandLineParser>
#include <QFile>
#include <QDomDocument>
#include <QDebug>

int main(int argc, char *argv[]) {

  QCoreApplication a(argc, argv);
  a.setApplicationName("Sparkless");
  a.setApplicationVersion("2.1.0");


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
  QCommandLineOption edDsaGeneratorPathOption("eddsa-generator-path", "The local file path to the executable used to generate the Ed25519 signature", "generator_path");
  QCommandLineOption dsaKeyFilePathOption("dsa-key-path", "The local file path to the dsa key used for signing [required for windows bundles]", "key_path");
  QCommandLineOption dsaGeneratorPathOption("dsa-generator-path", "The local file path to the executable used to generate the DSA signature [required for windows bundles]", "generator_path");

  QCommandLineOption s3RegionOption("s3-region", "The s3 region (used for url generation)", "region");
  QCommandLineOption s3BucketOption("s3-bucket", "The s3 bucket (used for url generation)", "bucket_name");
  QCommandLineOption s3BucketDirOption("s3-bucket-dir", "The diectory inside the s3 bucket (used for url generation)", "bucket_dir");
  QCommandLineOption s3MirrorPathOption("s3-mirror-path", "The file path to the local mirror of the s3 bucket dir [required for automatic delta generation, requires other s3 options to be set]", "num_deltas");

  QCommandLineOption urlPrefixOption("url-prefix", "The url (without the filename) to be used for the appcast URL generation", "url_without_filename");

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
      edDsaKeyOption, edDsaGeneratorPathOption, dsaKeyFilePathOption, dsaGeneratorPathOption,
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
      edDsaKeyOption, edDsaGeneratorPathOption, dsaKeyFilePathOption, dsaGeneratorPathOption,
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
    command = parser.positionalArguments().at(0);
  }

  /* ---- Print ---- */
  if (command == "print") {

    parser.addOption(appcastOption);
//    parser.process(a);

    if (!parser.isSet(appcastOption)) {
      qCritical().noquote().nospace() << "Missing required option '--appcast'";
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

    if (!parser.isSet(macBundleOption) && !parser.isSet(windowsBundleOption)) {
      qCritical().noquote().nospace() << "`sign` requires '--mac-bundle' and/or '--windows-bundle'";
      return 1;
    }

    if (!parser.isSet(edDsaKeyOption) && !parser.isSet(dsaKeyFilePathOption)) {
      qCritical().noquote().nospace() << "`sign` requires either '--eddsa-key' and/or '--dsa-key-path'";
      return 1;
    }

    if (parser.isSet(windowsBundleOption) && !parser.isSet(dsaKeyFilePathOption)) {
      qCritical().noquote().nospace() << "Windows bundles require a dsa signature. Please specify one with '--dsa-key-path'";
      return 1;
    }

    if (parser.isSet(dsaKeyFilePathOption) && !parser.isSet(dsaGeneratorPathOption)) {
      qCritical().noquote().nospace() << "`sign` option '--dsa-key-path' requires a path to a DSA signature generator. Please specify one with '--dsa-generator-path'";
      return -1;
    }

    if (parser.isSet(edDsaKeyOption) && !parser.isSet(edDsaGeneratorPathOption)) {
      qCritical().noquote().nospace() << "`sign` option '--eddsa-key' requires a path to an Ed25519 signature generator. Please specify one with '--eddsa-generator-path'";
      return -1;
    }

    const QString macBundlePath = parser.value(macBundleOption);
    const QString windowsBundlePath = parser.value(windowsBundleOption);
    const QByteArray edDsaKey = parser.isSet(edDsaKeyOption) ? parser.value(edDsaKeyOption).toUtf8() : QByteArray();
    const QString edDsaGeneratorPath = QDir::fromNativeSeparators(parser.value(edDsaGeneratorPathOption));
    const QString dsaKeyPath = QDir::fromNativeSeparators(parser.value(dsaKeyFilePathOption));
    const QString dsaGeneratorPath = QDir::fromNativeSeparators(parser.value(dsaGeneratorPathOption));

    if (parser.isSet(macBundleOption)) {

      if (parser.isSet(edDsaKeyOption)) {
        EdDsaSignatureGenerator sigGenerator(macBundlePath, edDsaKey, edDsaGeneratorPath);
        if (!sigGenerator.Success()) {
          return 1;
        }
        printf("\n%s [Ed25519]: %s\n", macBundlePath.section('/', -1).toUtf8().constData(), sigGenerator.Signature().constData());
      }
      else if (parser.isSet(dsaKeyFilePathOption)) {
        qDebug() << "dsa key path: " << dsaKeyPath;
        DsaSignatureGenerator sigGenerator(macBundlePath, dsaKeyPath, dsaGeneratorPath);
        if (!sigGenerator.Success()) {
          return 1;
        }
        printf("\n%s [DSA]: %s\n", macBundlePath.section('/', -1).toUtf8().constData(), sigGenerator.Signature().constData());
      }
    }

    if (parser.isSet(windowsBundleOption)) {
      DsaSignatureGenerator sigGenerator(windowsBundlePath, dsaKeyPath, dsaGeneratorPath);
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
      qCritical().noquote().nospace() << "`delta` requires '--mac-bundle'";
      return 1;
    }
    if (!parser.isSet(previousBundleOption)) {
      qCritical().noquote().nospace() << "`delta` requires '--prev-bundle'";
      return 1;
    }
    if (!parser.isSet(deltaPathOption)) {
      qCritical().noquote().nospace() << "`delta` requires '--delta-path'";
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

    if (!parser.isSet(macBundleOption) && !parser.isSet(windowsBundleOption)) {
      qCritical().noquote().nospace() << "`add` requires '--mac-bundle' and/or '--windows-bundle'";
      return 1;
    }

    if (!parser.isSet(edDsaKeyOption) && !parser.isSet(dsaKeyFilePathOption)) {
      qCritical().noquote().nospace() << "`add` requires either '--eddsa-key' and/or '--dsa-key-path'";
      return 1;
    }

    if (parser.isSet(windowsBundleOption) && !parser.isSet(dsaKeyFilePathOption)) {
      qCritical().noquote().nospace() << "Windows bundles require a DSA signature. Please specify one with '--dsa-key-path'";
      return 1;
    }

    if (parser.isSet(dsaKeyFilePathOption) && !parser.isSet(dsaGeneratorPathOption)) {
      qCritical().noquote().nospace() << "`add` option '--dsa-key-path' requires a path to a DSA signature generator. Please specify one with '--dsa-generator-path'";
      return -1;
    }

    if (parser.isSet(edDsaKeyOption) && !parser.isSet(edDsaGeneratorPathOption)) {
      qCritical().noquote().nospace() << "`add` option '--eddsa-key' requires a path to an Ed25519 signature generator. Please specify one with '--eddsa-generator-path'";
      return -1;
    }

    if (!parser.isSet(versionBuildOption)) {
      qCritical().noquote().nospace() << "`add` requires '--build'";
      return 1;
    }
    if (!parser.isSet(versionStringOption)) {
      qCritical().noquote().nospace() << "`add` requires '--version'";
      return 1;
    }
    if (!parser.isSet(urlPrefixOption) && (!parser.isSet(s3RegionOption) || !parser.isSet(s3BucketOption))) {
      qCritical().noquote().nospace() << "`add` requires either '--url-prefix' or '--s3-region' and '--s3-bucket'";
      return 1;
    }

    // validate delta requiremements
    if (parser.isSet(deltasOption)) {
      if (!parser.isSet(macBundleOption)) {
        qCritical().noquote().nospace() << "`add` option '--deltas' is only available for mac bundles'";
        return 1;
      }
      if (!parser.isSet(edDsaKeyOption)) {
        qCritical().noquote().nospace() << "`add` option '--deltas' requires an EdDsa signature. Please specify one with '--eddsa-key'";
        return 1;
      }
      if (!parser.isSet(s3MirrorPathOption)) {
        qCritical().noquote().nospace() << "`add` option '--deltas' requires a local s3 mirror path. Please specify one with '--s3-mirror-path'";
        return 1;
      }
    }

    const int deltasCount = parser.value(deltasOption).toInt();
    if (parser.isSet(deltasOption) && QString::number(deltasCount) != parser.value(deltasOption)) {
      qCritical().nospace().noquote() << "invalid value for option '--deltas'. Please specify a number > 0'";
      return 1;
    }
    const QString versionString = parser.value(versionStringOption);
    const qlonglong versionBuild = parser.value(versionBuildOption).toLongLong();

    const QString macBundlePath = QDir::fromNativeSeparators(parser.value(macBundleOption));
    const QString windowsBundlePath = QDir::fromNativeSeparators(parser.value(windowsBundleOption));

    const QByteArray edDsaKey = parser.isSet(edDsaKeyOption) ? parser.value(edDsaKeyOption).toUtf8() : QByteArray();
    const QString edDsaGeneratorPath = QDir::fromNativeSeparators(parser.value(edDsaGeneratorPathOption));
    const QString dsaKeyPath = QDir::fromNativeSeparators(parser.value(dsaKeyFilePathOption));
    const QString dsaGeneratorPath = QDir::fromNativeSeparators(parser.value(dsaGeneratorPathOption));

    const QString appcastPath = QDir::fromNativeSeparators(parser.value(appcastOption));
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

    if (parser.isSet(macBundleOption)) {
      if (parser.isSet(edDsaKeyOption)) {

        ItemEnclosure* newMacEnclosure = appcast->AddEnclosureToItem(newItem, macBundlePath, MacPlatform, edDsaKey, edDsaGeneratorPath);
        if (newMacEnclosure == nullptr) { qWarning().noquote().nospace() << "failed to add mac enclosure"; return 1; }

        if (deltasCount >= 1) {

          qInfo().noquote().nospace() << "\nGenerating deltas for build " << versionBuild << "...\n";

          int deltasCreated = 0;
          qlonglong currBuildNumber = newItem->VersionBuild() - 1;

          while (deltasCreated < deltasCount && currBuildNumber > 0) {

            ItemDelta* newDelta = appcast->CreateDeltaForBuild(currBuildNumber, macBundlePath, newItem, MacPlatform, edDsaKey, edDsaGeneratorPath);
            if (newDelta != nullptr) {
              deltasCreated++;
            }
            currBuildNumber--;
          }
        }
      }
      else if (parser.isSet(dsaKeyFilePathOption)) {
        ItemEnclosure* newMacEnclosure = appcast->AddEnclosureToItem(newItem, macBundlePath, MacPlatform, dsaKeyPath, dsaGeneratorPath);
        if (newMacEnclosure == nullptr) { qWarning().noquote().nospace() << "failed to add mac enclosure"; return 1; }
      }
    }
    if (parser.isSet(windowsBundleOption)) {
      if (parser.isSet(dsaKeyFilePathOption)) {
        ItemEnclosure* newWindowsEnclosure = appcast->AddEnclosureToItem(newItem, windowsBundlePath, WindowsPlatform, dsaKeyPath, dsaGeneratorPath);
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
