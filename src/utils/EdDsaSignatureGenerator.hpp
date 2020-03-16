//
//  EdDsaSignatureGenerator.hpp
//  sparkless
//
//  Created by Kyle King on 2020-03-14.
//  Copyright © 2020 Kyle King. All rights reserved.
//

#ifndef EdDsaSignatureGenerator_hpp
#define EdDsaSignatureGenerator_hpp

#include <QObject>

#include "Constants.hpp"

class EdDsaSignatureGenerator {

private:

  QString binaryPath;
  QByteArray edDsaKey;

  QByteArray signature;

  bool success = false;
  QByteArray commandOutput;


#pragma mark - Constructors -

#pragma mark Public
public:

  explicit EdDsaSignatureGenerator();
  explicit EdDsaSignatureGenerator(const QString& theBinaryPath, const QByteArray& EdDsaKey);


#pragma mark - Accessors -

#pragma mark Private
private:

static QString GenerateSignatureProgramPath();

#pragma mark Public
public:

  const QString& BinaryPath() const { return binaryPath; }
  const QByteArray& EdDsaKey() const { return edDsaKey; }

  const QByteArray& Signature() const { return signature; }

  bool Success() const { return success; }
  const QByteArray& CommandOutput() const { return commandOutput; }


#pragma mark - Mutators -

#pragma mark Public
public:

  void SetBinaryPath(const QString&);
  void SetEdDsaKey(const QByteArray&);

  bool GenerateSignature();

};

#endif /* EdDsaSignatureGenerator_hpp */
