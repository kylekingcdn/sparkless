//
//  DsaSignatureGenerator.hpp
//  sparkless
//
//  Created by Kyle King on 2020-03-14.
//  Copyright © 2020 Kyle King. All rights reserved.
//

#ifndef DsaSignatureGenerator_hpp
#define DsaSignatureGenerator_hpp

#include <QObject>

#include "Constants.hpp"

class DsaSignatureGenerator {

private:

  QString binaryPath;
  QString dsaKeyPath;

  QByteArray signature;

  bool success = false;
  QByteArray commandOutput;


#pragma mark - Constructors -

#pragma mark Public
public:

  explicit DsaSignatureGenerator();
  explicit DsaSignatureGenerator(const QString& theBinaryPath, const QString& DsaKeyPath);


#pragma mark - Accessors -

#pragma mark Private
private:

static QString GenerateSignatureProgramPath();

#pragma mark Public
public:

  const QString& BinaryPath() const { return binaryPath; }
  const QString& DsaKeyPath() const { return dsaKeyPath; }

  bool Success() const { return success; }

  const QByteArray& CommandOutput() const { return commandOutput; }
  const QByteArray& Signature() const { return signature; }


#pragma mark - Mutators -

#pragma mark Public
public:

  bool GenerateSignature();

  void SetBinaryPath(const QString&);
  void SetDsaKeyPath(const QString&);

};

#endif /* DsaSignatureGenerator_hpp */
