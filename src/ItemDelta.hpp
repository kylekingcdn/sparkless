//
//  ItemDelta.hpp
//  sparkless
//
//  Created by Kyle King on 2020-03-14.
//  Copyright © 2020 Kyle King. All rights reserved.
//

#ifndef ItemDelta_hpp
#define ItemDelta_hpp

#include "ItemEnclosure.hpp"

class ItemDelta : public ItemEnclosure {
  Q_OBJECT

private:

  qlonglong initialVersionBuild = -1;

protected:

  ItemDelta(QObject* theParent = nullptr);
  ItemDelta(const qlonglong theLength, const qlonglong thePrevBuild, const qlonglong theNewBuild, const QString& theVersion, const QUrl& theUrl, const EnclosurePlatform thePlatform, const QByteArray& theSignature, const EnclosureSignatureType theSignatureType, QObject* theParent = nullptr);
  ItemDelta(const QDomElement&, QObject* theParent = nullptr);

public:

  static ItemDelta* FromElement(const QDomElement&, QObject* theParent = nullptr);

  static ItemDelta* NewDelta(const qlonglong theLength, const qlonglong thePrevBuild, const qlonglong theNewBuild, const QString& theVersion, const QUrl& theUrl, const EnclosurePlatform thePlatform, const QByteArray& theSignature, const EnclosureSignatureType theSignatureType, QObject* theParent = nullptr);

  virtual ~ItemDelta() Q_DECL_OVERRIDE;

public:

  qlonglong InitialVersionBuild() const { return initialVersionBuild; }

  virtual void Print() const Q_DECL_OVERRIDE;

protected:

  virtual bool ParseXml() Q_DECL_OVERRIDE;

public:

  virtual bool Serialize(QDomElement& theDeltaElement) Q_DECL_OVERRIDE;
};

#endif /* ItemDelta_hpp */
