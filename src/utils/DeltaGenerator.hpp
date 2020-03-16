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


#pragma mark - Constructors -

#pragma mark Public
public:

  DeltaGenerator(const QString& theOldAppPath, const QString& theNewAppPath, const QString& theDeltaPath);


#pragma mark - Accessors -

#pragma mark Private
private:

  static QString GenerateDeltaProgramPath();

#pragma mark Public
public:

  const QString& OldAppPath() const { return oldAppPath; }
  const QString& NewAppPath() const { return oldAppPath; }

  const QString& DeltaPath() const { return deltaPath; }

  QByteArray CommandOutput() const { return commandOutput; }
  bool Success() const { return success; }


#pragma mark - Mutators -

#pragma mark Private
private:

#pragma mark Public
public:

  bool GenerateDelta();

};

#endif /* DeltaGenerator_hpp */
