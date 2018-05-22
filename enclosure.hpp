//
//  enclosure.hpp
//  Crate
//
//  Created by Kyle King on 2018-5-21.
//  Copyright © 2016 Divide Software Inc. All rights reserved.
//

#ifndef ENCLOSURE_HPP
#define ENCLOSURE_HPP

#include <QtCore>
#include <QDomElement>

class Enclosure {

public:

  enum Platform { NullOS=0, Mac, Windows, Linux };

private:

  Platform os;
  QString filepath;
  QString signature;

  QString version;
  int buildNumber;
  QUrl bucketURL;


public:

  Enclosure(const Platform theOS, const QString& theFile, const QString& theSignature,
            const QString theVersion, const int theBuild, const QUrl& theBucket);

//  Enclosure();

  static QString PlatformString(const Platform);
  QString PlatformString() const { return PlatformString(os); }

  static QString SparklePlatformString(const Platform);
  QString SparklePlatformString() const { return SparklePlatformString(os); }

  Platform OS() const { return os; }
  const QString& Filepath() const { return filepath; }
  const QString& Signature() const { return signature; }

  const QString& Version() const { return version; }
  int BuildNumber() const { return buildNumber; }
  const QUrl BucketURL() const { return bucketURL; }

  QString Filename() const;
  qint64 Length() const;

  QUrl ReleaseURL() const;
  //QUrl ReleaseNotesURL() const;

  QDomElement EnclosureNode(QDomDocument& theDocument) const;

  bool IsValid() const { return Errors().isEmpty(); }
  QStringList Errors() const;

  void Print() const;

};

#endif // ENCLOSURE_HPP
