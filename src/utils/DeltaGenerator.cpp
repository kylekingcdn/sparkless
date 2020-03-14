//
//  DeltaGenerator.cpp
//  sparkless
//
//  Created by Kyle King on 2020-03-14.
//  Copyright © 2020 Kyle King. All rights reserved.
//

#include "DeltaGenerator.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QProcess>

#pragma mark - Constructors -

#pragma mark Public

DeltaGenerator::DeltaGenerator(const QString& theOldAppPath, const QString& theNewAppPath, const QString& theDeltaPath)
: oldAppPath(theOldAppPath), newAppPath(theNewAppPath), deltaPath(theDeltaPath) {

  success = GenerateDelta();
}


#pragma mark - Accessors -

#pragma mark Private

QString DeltaGenerator::ScriptsDir() {

#ifdef DEBUG
  return QDir::currentPath();
#endif

  return qApp->applicationDirPath();
}

QString DeltaGenerator::GenerateDeltaScriptPath() {

  return QString("%1/%2").arg(ScriptsDir(), "BinaryDelta");
}

#pragma mark - Mutators -

#pragma mark Private

#pragma mark Public

bool DeltaGenerator::GenerateDelta() {

  const QString generateProcessPath = GenerateDeltaScriptPath();
  const QStringList generateArgs = {
    "create",
    "--verbose",
    oldAppPath,
    newAppPath,
    deltaPath,
  };

  if (!QFileInfo::exists(generateProcessPath)) {
    commandOutput = "could not find BinaryDelta program: " + generateProcessPath.toUtf8();
    return false;
  }

  QProcess generateProcess;
  generateProcess.setProgram(generateProcessPath);
  generateProcess.setArguments(generateArgs);

  qDebug() << "GenerateDelta() executing '" << generateProcess.program() << " " << generateProcess.arguments().join(' ') << "'";
  generateProcess.start();

  if (!generateProcess.waitForStarted(-1)) {

    return false;
  }

  if (!generateProcess.waitForFinished(-1)) {
    return false;
  }

  commandOutput = generateProcess.readAllStandardOutput();

  return generateProcess.exitStatus() == QProcess::NormalExit;
}
