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
  QString edDsaGeneratorPath;

  QByteArray signature;

  bool success = false;
  QByteArray commandOutput;

public:

  explicit EdDsaSignatureGenerator();
  explicit EdDsaSignatureGenerator(const QString& theBinaryPath, const QByteArray& EdDsaKey, const QString& generatorPath);

public:

  const QString& BinaryPath() const { return binaryPath; }
  const QByteArray& EdDsaKey() const { return edDsaKey; }
  const QString& EdDsaGeneratorPath() const { return edDsaGeneratorPath; }

  const QByteArray& Signature() const { return signature; }

  bool Success() const { return success; }
  const QByteArray& CommandOutput() const { return commandOutput; }

public:

  void SetBinaryPath(const QString&);
  void SetEdDsaKey(const QByteArray&);
  void SetEdDsaGeneratorPath(const QString&);

  bool GenerateSignature();

};

#endif /* EdDsaSignatureGenerator_hpp */
