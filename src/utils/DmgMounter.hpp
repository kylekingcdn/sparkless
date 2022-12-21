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

public:

  DmgMounter();
  DmgMounter(const QString& theImagePath, const QString& theMountPoint);

private:

  static QString HdiutilPath();

public:

  const QString& ImagePath() const { return imagePath; }
  const QString& MountPoint() const { return mountPoint; }

  bool Mounted() const { return mounted; }

  bool Success() const { return success; }
  QByteArray CommandOutput() const { return commandOutput; }

public:

  void SetImagePath(const QString&);
  void SetMountPoint(const QString&);

  bool Mount();
  bool Unmount();

};

#endif /* DmgMounter_hpp */
