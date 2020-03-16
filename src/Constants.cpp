//
//  Constants.cpp
//  sparkless
//
//  Created by Kyle King on 2020-03-14.
//  Copyright © 2020 Kyle King. All rights reserved.
//

#include "Constants.hpp"

#include <QDir>

QString HelperScriptsDir() {

#ifdef DEBUG
  return QDir::currentPath() + "/scripts";
#else
  return qApp->applicationDirPath();
#endif
}
