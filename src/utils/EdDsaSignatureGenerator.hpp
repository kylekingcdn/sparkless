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

class EdDsaSignatureGenerator {

private:

  QString binaryPath;
  QByteArray edDsaKey;

  QByteArray signature;

  bool success = false;
  QByteArray commandOutput;

public:

  explicit EdDsaSignatureGenerator();
  explicit EdDsaSignatureGenerator(const QString& theBinaryPath, const QByteArray& EdDsaKey);

private:

static QString GenerateSignatureProgramPath();

public:

  const QString& BinaryPath() const { return binaryPath; }
  const QByteArray& EdDsaKey() const { return edDsaKey; }

  const QByteArray& Signature() const { return signature; }

  bool Success() const { return success; }
  const QByteArray& CommandOutput() const { return commandOutput; }

public:

  void SetBinaryPath(const QString&);
  void SetEdDsaKey(const QByteArray&);

  bool GenerateSignature();

};

#endif /* EdDsaSignatureGenerator_hpp */
