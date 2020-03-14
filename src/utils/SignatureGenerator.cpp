//
//  SignatureGenerator.cpp
//  sparkless
//
//  Created by Kyle King on 2020-03-14.
//  Copyright © 2020 Kyle King. All rights reserved.
//

#include "SignatureGenerator.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QProcess>

#pragma mark - Constructors -

#pragma mark Public

SignatureGenerator::SignatureGenerator(const QString& theBinaryPath, const QString& theDsaKeyPath)
: binaryPath(theBinaryPath), dsaKeyPath(theDsaKeyPath) {

  signatureType = DsaSignature;
  success = GenerateDsaSignature();
}

SignatureGenerator::SignatureGenerator(const QString& theBinaryPath, const QByteArray& theEdDsaKey)
: binaryPath(theBinaryPath), edDsaKey(theEdDsaKey) {

  signatureType = Ed25519Signature;
  success = GenerateEdDsaSignature();
}


#pragma mark - Accessors -

#pragma mark Private

QString SignatureGenerator::ScriptsDir() {

#ifdef DEBUG
  return QDir::currentPath();
#endif

  return qApp->applicationDirPath();
}

QString SignatureGenerator::GenerateSignatureScriptPath(const EnclosureSignatureType theSignatureType) {

  QString scriptFilename;
  QString scriptPath;

  switch (theSignatureType) {
    case Ed25519Signature: {
      scriptFilename = "sign_update_EdDSA";
      break;
    }
    case DsaSignature: {
      scriptFilename = "sign_update_DSA";
      break;
    }
    default: {
      break;
    }
  }

  if (!scriptFilename.isEmpty()) {
    scriptPath = QString("%1/%2").arg(ScriptsDir(), scriptFilename);
  }

  return scriptPath;
}


#pragma mark - Mutators -

#pragma mark Private

#pragma mark Public

bool SignatureGenerator::GenerateDsaSignature() {

  const QString generatePath = GenerateSignatureScriptPath(signatureType);

  if (generatePath.isEmpty()) {
    qWarning() << "GenerateDsaSignature() error - generation program path is empty";
    return false;
  }
  if (!QFileInfo::exists(generatePath)) {
    commandOutput = "Generate signature program doesn't exist: " + generatePath.toUtf8();
    return false;
  }
  if (!QFileInfo::exists(binaryPath)) {
    commandOutput = "Binary doesn't exist: " + generatePath.toUtf8();
    return false;
  }
  if (!QFileInfo::exists(dsaKeyPath)) {
    commandOutput = "DSA key file doesn't exist: " + generatePath.toUtf8();
    return false;
  }

  const QStringList generateArgs = {
    binaryPath,
    dsaKeyPath,
  };

  QProcess generateProcess;
  generateProcess.setProgram(generatePath);
  generateProcess.setArguments(generateArgs);

  qDebug().noquote().nospace() << "GenerateDsaSignature() executing '" << generateProcess.program() << " " << generateProcess.arguments().join(' ') << "'";
  generateProcess.start();

  if (!generateProcess.waitForStarted(-1)) {
    return false;
  }

  if (!generateProcess.waitForFinished(-1)) {
    return false;
  }

  commandOutput = generateProcess.readAllStandardOutput();
  if (generateProcess.exitStatus() == QProcess::NormalExit) {
    signature = commandOutput.simplified();
    return true;
  }
  else {
    signature = QByteArray();
    return false;
  }
}

bool SignatureGenerator::GenerateEdDsaSignature() {

  const QString generatePath = GenerateSignatureScriptPath(signatureType);

  if (generatePath.isEmpty()) {
    qWarning() << "GenerateEdDsaSignature() error - generation program path is empty";
    return false;
  }
  if (!QFileInfo::exists(generatePath)) {
    commandOutput = "Generate signature program doesn't exist: " + generatePath.toUtf8();
    return false;
  }
  if (!QFileInfo::exists(binaryPath)) {
    commandOutput = "Binary doesn't exist: " + generatePath.toUtf8();
    return false;
  }

  const QStringList generateArgs = {
    "-s", edDsaKey,
    binaryPath,
  };

  QProcess generateProcess;
  generateProcess.setProgram(generatePath);
  generateProcess.setArguments(generateArgs);

//  qDebug().noquote().nospace() << "GenerateEdDsaSignature() executing '" << generateProcess.program() << " " << generateProcess.arguments().join(' ') << "'";
  generateProcess.start();

  if (!generateProcess.waitForStarted(-1)) {
    return false;
  }

  if (!generateProcess.waitForFinished(-1)) {
    return false;
  }

  commandOutput = generateProcess.readAllStandardOutput();

  if (generateProcess.exitStatus() == QProcess::NormalExit) {

    const QList<QByteArray> signatureParts = commandOutput.simplified().split('"');

    if (signatureParts.count() == 5) {
      signature = signatureParts.at(1);
      return true;
    }
    else {
      qWarning() << "failed to parse EdDSA signature generator output";
    }
  }

  signature = QByteArray();
  return false;
}

