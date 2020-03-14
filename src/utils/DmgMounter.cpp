//
//  DmgMounter.cpp
//  sparkless
//
//  Created by Kyle King on 2020-03-14.
//  Copyright © 2020 Kyle King. All rights reserved.
//

#include "DmgMounter.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QProcess>

#pragma mark - Constructors -

#pragma mark Public

DmgMounter::DmgMounter(const QString& theImagePath, const QString& theMountPoint)
: imagePath(theImagePath), mountPoint(theMountPoint) {

}


#pragma mark - Accessors -

#pragma mark Private


#pragma mark - Mutators -

#pragma mark Private

#pragma mark Public

bool DmgMounter::Mount() {

  const QString hdiutilPath = "/usr/bin/hdiutil";
  const QStringList hdiutilArgs = {
    "attach",
    "-nobrowse",
    "-readonly",
    "-mountpoint", mountPoint,
    imagePath,
  };

  QProcess hdiutilProcess;
  hdiutilProcess.setProgram(hdiutilPath);
  hdiutilProcess.setArguments(hdiutilArgs);

  qDebug().noquote().nospace() << "Mount() executing '" << hdiutilProcess.program() << " " << hdiutilProcess.arguments().join(' ') << "'";
  hdiutilProcess.start();

  if (!hdiutilProcess.waitForStarted(-1)) {
    return false;
  }

  if (!hdiutilProcess.waitForFinished(-1)) {
    return false;
  }

  commandOutput = hdiutilProcess.readAllStandardOutput();

  if (hdiutilProcess.exitStatus() == QProcess::NormalExit) {
    mounted = true;
    return true;
  }
  else {
    return false;
  }
}

bool DmgMounter::Unmount() {

  const QString hdiutilPath = "/usr/bin/hdiutil";
  if (!QFileInfo::exists(hdiutilPath)) {
    commandOutput = "could not find hdiutil program: " + hdiutilPath.toUtf8();
    return false;
  }

  const QStringList hdiutilArgs = {
    "detach",
    mountPoint,
  };

  QProcess hdiutilProcess;
  hdiutilProcess.setProgram(hdiutilPath);
  hdiutilProcess.setArguments(hdiutilArgs);

  qDebug().noquote().nospace() << "Unmount() executing '" << hdiutilProcess.program() << " " << hdiutilProcess.arguments().join(' ') << "'";
  hdiutilProcess.start();

  if (!hdiutilProcess.waitForStarted(-1)) {
    return false;
  }

  if (!hdiutilProcess.waitForFinished(-1)) {
    return false;
  }

  commandOutput = hdiutilProcess.readAllStandardOutput();

  mounted = false;

  if (hdiutilProcess.exitStatus() == QProcess::NormalExit) {
    mounted = false;
    return true;
  }
  else {
    return false;
  }
}
