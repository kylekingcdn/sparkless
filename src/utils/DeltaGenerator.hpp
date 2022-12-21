//
//  DeltaGenerator.hpp
//  sparkless
//
//  Created by Kyle King on 2020-03-14.
//  Copyright © 2020 Kyle King. All rights reserved.
//

#ifndef DeltaGenerator_hpp
#define DeltaGenerator_hpp

#include <QObject>

class DeltaGenerator {

private:

  QString newAppPath;
  QString oldAppPath;
  QString deltaPath;

  bool success = false;
  QByteArray commandOutput;

public:

  DeltaGenerator(const QString& theOldAppPath, const QString& theNewAppPath, const QString& theDeltaPath);

private:

  static QString GenerateDeltaProgramPath();

public:

  const QString& OldAppPath() const { return oldAppPath; }
  const QString& NewAppPath() const { return oldAppPath; }

  const QString& DeltaPath() const { return deltaPath; }

  QByteArray CommandOutput() const { return commandOutput; }
  bool Success() const { return success; }

public:

  bool GenerateDelta();

};

#endif /* DeltaGenerator_hpp */
