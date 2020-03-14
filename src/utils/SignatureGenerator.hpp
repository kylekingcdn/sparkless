//
//  SignatureGenerator.hpp
//  sparkless
//
//  Created by Kyle King on 2020-03-14.
//  Copyright © 2020 Kyle King. All rights reserved.
//

#ifndef SignatureGenerator_hpp
#define SignatureGenerator_hpp

#include <QObject>

#include "Constants.hpp"

class SignatureGenerator {

private:

  QString binaryPath;

  QString dsaKeyPath;
  QByteArray edDsaKey;
  EnclosureSignatureType signatureType;

  bool success = false;

  QByteArray commandOutput;
  QByteArray signature;


#pragma mark - Constructors -

#pragma mark Public
public:

  explicit SignatureGenerator(const QString& theBinaryPath, const QString& DsaKeyPath);
  explicit SignatureGenerator(const QString& theBinaryPath, const QByteArray& EdDsaKey);


#pragma mark - Accessors -

#pragma mark Private
private:

static QString ScriptsDir();
static QString GenerateSignatureScriptPath(const EnclosureSignatureType);

#pragma mark Public
public:

  const QString& BinaryPath() const { return binaryPath; }
  EnclosureSignatureType SignatureType() const { return signatureType; }

  bool Success() const { return success; }

  const QByteArray& CommandOutput() const { return commandOutput; }
  const QByteArray& Signature() const { return signature; }


#pragma mark - Mutators -

#pragma mark Private
private:

#pragma mark Public
public:

  bool GenerateDsaSignature();
  bool GenerateEdDsaSignature();

};

#endif /* SignatureGenerator_hpp */
