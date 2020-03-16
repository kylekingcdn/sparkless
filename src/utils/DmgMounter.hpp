//
//  DmgMounter.hpp
//  sparkless
//
//  Created by Kyle King on 2020-03-14.
//  Copyright © 2020 Kyle King. All rights reserved.
//

#ifndef DmgMounter_hpp
#define DmgMounter_hpp

#include <QObject>

class DmgMounter {

private:

  QString imagePath;
  QString mountPoint;

  bool mounted = false;

  bool success = false;
  QByteArray commandOutput;


#pragma mark - Constructors -

#pragma mark Public
public:

  DmgMounter();
  DmgMounter(const QString& theImagePath, const QString& theMountPoint);


#pragma mark - Accessors -

#pragma mark Private
private:

  static QString HdiutilPath();


#pragma mark Public
public:

  const QString& ImagePath() const { return imagePath; }
  const QString& MountPoint() const { return mountPoint; }

  bool Mounted() const { return mounted; }

  bool Success() const { return success; }
  QByteArray CommandOutput() const { return commandOutput; }


#pragma mark - Mutators -

#pragma mark Public
public:

  void SetImagePath(const QString&);
  void SetMountPoint(const QString&);

  bool Mount();
  bool Unmount();

};

#endif /* DmgMounter_hpp */
