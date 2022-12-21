//
//  DeltaGenerator.cpp
//  sparkless
//
//  Created by Kyle King on 2020-03-14.
//  Copyright © 2020 Kyle King. All rights reserved.
//

#include "utils/DeltaGenerator.hpp"
#include "Constants.hpp"

#include <QCoreApplication>
#include <QFileInfo>
#include <QDebug>
#include <QProcess>

DeltaGenerator::DeltaGenerator(const QString& theOldAppPath, const QString& theNewAppPath, const QString& theDeltaPath)
: oldAppPath(theOldAppPath), newAppPath(theNewAppPath), deltaPath(theDeltaPath) {

  success = GenerateDelta();
}

QString DeltaGenerator::GenerateDeltaProgramPath() {

  return QString("%1/%2").arg(HelperScriptsDir(), "BinaryDelta");
}

bool DeltaGenerator::GenerateDelta() {

  const QString generateDeltaPath = GenerateDeltaProgramPath();

    if (!QFileInfo::exists(generateDeltaPath)) {
    qFatal("Could not find delta generator program at expected path: %s", generateDeltaPath.toLatin1().constData());
    return false;
  }
  if (!QFileInfo::exists(oldAppPath)) {
    qWarning() << "Error generating delta - old app not found at path: " << oldAppPath;
    return false;
  }
  if (!QFileInfo::exists(newAppPath)) {
    qWarning() << "Error generating delta - new app not found at path: " << newAppPath;
    return false;
  }
  if (QFileInfo::exists(deltaPath)) {
    qWarning() << "Error generating delta - delta already exists at path: " << deltaPath;
    return false;
  }

  const QStringList generateArgs = {
    "create",
    "--verbose",
    oldAppPath,
    newAppPath,
    deltaPath,
  };


//  qInfo().noquote().nospace() << "Generating delta for bundles: '" << oldAppPath << "' and '" << newAppPath << "'";

  QProcess generateProcess;
  generateProcess.setProgram(generateDeltaPath);
  generateProcess.setArguments(generateArgs);
//  qDebug().nospace().noquote() << "GenerateDelta() executing: " << generateProcess.program() << " " << generateProcess.arguments().join(' ');
  generateProcess.start();

  if (!generateProcess.waitForStarted(-1)) {
    qWarning() << "waitForStarted() failed for:" << generateDeltaPath;
    success = false;
  }
  else if (!generateProcess.waitForFinished(-1)) {
    qWarning() << "waitForFinished() failed for:" << generateDeltaPath;
    success = false;
  }

  else {

    commandOutput = generateProcess.readAllStandardOutput();

    if (generateProcess.exitStatus() == QProcess::NormalExit) {
      success = true;
    }
    else {
      qWarning().noquote().nospace() << "BinaryDelta had a non-zero exit code - output: " << commandOutput;
      success = false;
    }
  }

  if (!success) {
    qWarning() << "generate delta program failed";
  }

  return success;
}
