//
//  DsaSignatureGenerator.cpp
//  sparkless
//
//  Created by Kyle King on 2020-03-14.
//  Copyright © 2020 Kyle King. All rights reserved.
//

#include "utils/DsaSignatureGenerator.hpp"

#include <QCoreApplication>
#include <QDebug>
#include <QProcess>
#include <QFileInfo>

#pragma mark - Constructors -

#pragma mark Public

DsaSignatureGenerator::DsaSignatureGenerator() {

}

DsaSignatureGenerator::DsaSignatureGenerator(const QString& theBinaryPath, const QString& theDsaKeyPath) {

  SetBinaryPath(theBinaryPath);
  SetDsaKeyPath(theDsaKeyPath);

  GenerateSignature();
}


#pragma mark - Accessors -

#pragma mark Private

QString DsaSignatureGenerator::GenerateSignatureProgramPath() {

  return QString("%1/%2").arg(HelperScriptsDir(), "sign_update.bat");
}


#pragma mark - Mutators -

#pragma mark Public

void DsaSignatureGenerator::SetBinaryPath(const QString& thePath) {

  binaryPath = thePath;
}

void DsaSignatureGenerator::SetDsaKeyPath(const QString& thePath) {

  dsaKeyPath = thePath;
}

bool DsaSignatureGenerator::GenerateSignature() {

  // reset signature value
  signature = QByteArray();

  const QString generatePath = GenerateSignatureProgramPath();

  if (!QFileInfo::exists(generatePath)) {
    qFatal("Could not find dsa signature generator program at expected path: %s", generatePath.toLatin1().constData());
    return false;
  }
  if (!QFileInfo::exists(binaryPath)) {
    qWarning() << "Error generating dsa signature - binary doesn't exist: " << binaryPath;
    return false;
  }
  if (!QFileInfo::exists(dsaKeyPath)) {
    qWarning() << "Error generating dsa signature - DSA key file doesn't exist: " + dsaKeyPath;
    return false;
  }

  const QStringList generateArgs = {
    binaryPath,
    dsaKeyPath,
  };

  QProcess generateProcess;
  generateProcess.setProgram(generatePath);
  generateProcess.setArguments(generateArgs);
//  qDebug().noquote().nospace() << "DsaSignatureGenerator::GenerateSignature() executing '" << generateProcess.program() << " " << generateProcess.arguments().join(' ') << "'";
  generateProcess.start();

  if (!generateProcess.waitForStarted(-1)) {
    qWarning() << "waitForStarted() failed for: " << generatePath;
    success = false;
  }
  else if (!generateProcess.waitForFinished(-1)) {
    qWarning() << "waitForFinished() failed for: " << generatePath;
    success = false;
  }
  else {
    commandOutput = generateProcess.readAllStandardOutput();

    if (generateProcess.exitStatus() == QProcess::NormalExit) {

      signature = commandOutput.simplified();

      if (signature.isEmpty()) {
        qWarning() << "failed to parse DSA signature generator output: " << commandOutput;
        success = false;
      }
      else {
        success = true;
      }
    }
    else {
      qWarning() << "DSA signature generator had a non-zero exit code - output: " << commandOutput;
      success = false;
    }
  }

  if (!success) {
    qWarning() << "DSA signature generation failed";
  }

  return success;
}
