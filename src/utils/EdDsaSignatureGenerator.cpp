//
//  EdDsaSignatureGenerator.cpp
//  sparkless
//
//  Created by Kyle King on 2020-03-14.
//  Copyright © 2020 Kyle King. All rights reserved.
//

#include "EdDsaSignatureGenerator.hpp"

#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QProcess>

#pragma mark - Constructors -

#pragma mark Public

EdDsaSignatureGenerator::EdDsaSignatureGenerator() {

}

EdDsaSignatureGenerator::EdDsaSignatureGenerator(const QString& theBinaryPath, const QByteArray& theEdDsaKey) {

  SetBinaryPath(theBinaryPath);
  SetEdDsaKey(theEdDsaKey);

  GenerateSignature();
}


#pragma mark - Accessors -

#pragma mark Private

QString EdDsaSignatureGenerator::GenerateSignatureProgramPath() {

  return QString("%1/%2").arg(HelperScriptsDir(), "sign_update_EdDSA");
}


#pragma mark - Mutators -

#pragma mark Public

void EdDsaSignatureGenerator::SetBinaryPath(const QString& thePath) {

  binaryPath = thePath;
}

void EdDsaSignatureGenerator::SetEdDsaKey(const QByteArray& theKey) {

  edDsaKey = theKey;
}

bool EdDsaSignatureGenerator::GenerateSignature() {

  // reset signature value
  signature = QByteArray();

  const QString generatePath = GenerateSignatureProgramPath();

  if (!QFileInfo::exists(generatePath)) {
    qFatal("Could not find Ed25519 signature generator program at expected path: %s", generatePath.toLatin1().constData());
    return false;
  }
  if (!QFileInfo::exists(binaryPath)) {
    qWarning() << "Error generating Ed25519 signature - binary doesn't exist: " << binaryPath;
    return false;
  }
  if (edDsaKey.isEmpty()) {
    qWarning() << "Error generating Ed25519 signature - specified key is empty";
    return false;
  }

  const QStringList generateArgs = {
    "-s", edDsaKey,
    binaryPath,
  };

  QProcess generateProcess;
  generateProcess.setProgram(generatePath);
  generateProcess.setArguments(generateArgs);
//  qDebug().noquote().nospace() << "EdDsaSignatureGenerator::GenerateSignature() executing '" << generateProcess.program() << " " << generateProcess.arguments().join(' ') << "'";
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

      const QList<QByteArray> signatureParts = commandOutput.simplified().split('"');

      if (signatureParts.count() == 5) {
        signature = signatureParts.at(1);
      }

      if (signature.isEmpty()) {
        qWarning() << "failed to parse EdDSA signature generator output: " << commandOutput;
        success = false;
      }
      else {
        success = true;
      }
    }
    else {
      qWarning() << "EdDSA signature generator had a non-zero exit code - output: " << commandOutput;
      success = false;
    }
  }

  if (!success) {
    qWarning() << "EdDSA signature generation failed";
  }

  return false;
}

