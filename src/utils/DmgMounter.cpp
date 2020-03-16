//
//  DmgMounter.cpp
//  sparkless
//
//  Created by Kyle King on 2020-03-14.
//  Copyright © 2020 Kyle King. All rights reserved.
//

#include "utils/DmgMounter.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QProcess>

#pragma mark - Constructors -

#pragma mark Public

DmgMounter::DmgMounter() {

}

DmgMounter::DmgMounter(const QString& theImagePath, const QString& theMountPoint) {

  SetImagePath(theImagePath);
  SetMountPoint(theMountPoint);
}


#pragma mark - Accessors -

#pragma mark Private

QString DmgMounter::HdiutilPath() {

  return QString("/usr/bin/hdiutil");
}


#pragma mark - Mutators -

void DmgMounter::SetImagePath(const QString& thePath) {

  imagePath = thePath;
}

void DmgMounter::SetMountPoint(const QString& theMountPoint) {

  mountPoint = theMountPoint;
}

#pragma mark Public

bool DmgMounter::Mount() {

  const QString hdiutilPath = HdiutilPath();

  if (!QFileInfo::exists(hdiutilPath)) {
    qFatal("Could not find hdiutil program at expected path: %s", hdiutilPath.toLatin1().constData());
    return false;
  }
  if (!QFileInfo::exists(imagePath)) {
    qWarning() << "Error mounting dmg - image not found: " << imagePath;
    return false;
  }
  if (!QFileInfo::exists(mountPoint)) {
    qWarning() << "Error mounting dmg - mount point not found: " << mountPoint;
    return false;
  }

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
    success = false;
  }

  else if (!hdiutilProcess.waitForFinished(-1)) {
    success = false;
  }

  else {

    commandOutput = hdiutilProcess.readAllStandardOutput();

    if (hdiutilProcess.exitStatus() == QProcess::NormalExit) {
      mounted = true;
      success =  true;
    }
    else {
      success = false;
    }
  }

  return success;
}

bool DmgMounter::Unmount() {

   const QString hdiutilPath = HdiutilPath();

  if (!QFileInfo::exists(hdiutilPath)) {
    qFatal("Could not find hdiutil program at expected path: %s", hdiutilPath.toLatin1().constData());
    return false;
  }
  if (!QFileInfo::exists(mountPoint)) {
    qWarning() << "Error unmounting dmg - mount point not found: " << mountPoint;
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
    qWarning() << "waitForStarted() failed for hdiutil unmount";
    success = false;
  }

  else if (!hdiutilProcess.waitForFinished(-1)) {
    qWarning() << "waitForFinished() failed for hdiutil unmount";
    success = false;
  }

  else {

    commandOutput = hdiutilProcess.readAllStandardOutput();

    if (hdiutilProcess.exitStatus() == QProcess::NormalExit) {
      mounted = false;
      success =  true;
    }
    else {
      qWarning().noquote().nospace() << "hdiutil unmount had a non-zero exit code - output: " << commandOutput;
      success = false;
    }
  }

  if (!success) {
    qWarning() << "hdiutil unmount failed";
  }

  return success;
}
