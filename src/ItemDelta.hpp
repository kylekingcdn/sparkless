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


#pragma mark - Constructors -

#pragma mark Protected
protected:

  ItemDelta(QObject* theParent = nullptr);
  ItemDelta(const qlonglong theLength, const qlonglong thePrevBuild, const qlonglong theNewBuild, const QString& theVersion, const QUrl& theUrl, const EnclosurePlatform thePlatform, const QByteArray& theSignature, const EnclosureSignatureType theSignatureType, QObject* theParent = nullptr);
  ItemDelta(const QDomElement&, QObject* theParent = nullptr);

#pragma mark Public
public:

  static ItemDelta* FromElement(const QDomElement&, QObject* theParent = nullptr);

  static ItemDelta* NewDelta(const qlonglong theLength, const qlonglong thePrevBuild, const qlonglong theNewBuild, const QString& theVersion, const QUrl& theUrl, const EnclosurePlatform thePlatform, const QByteArray& theSignature, const EnclosureSignatureType theSignatureType, QObject* theParent = nullptr);

  virtual ~ItemDelta() Q_DECL_OVERRIDE;


#pragma mark - Accessors -

#pragma mark Private
private:

#pragma mark Public
public:

  qlonglong InitialVersionBuild() const { return initialVersionBuild; }

  virtual void Print() const Q_DECL_OVERRIDE;


#pragma mark - Mutators -

#pragma mark Protected
protected:

  virtual bool ParseXml() Q_DECL_OVERRIDE;

#pragma mark Public
public:

  virtual bool Serialize(QDomElement& theDeltaElement) Q_DECL_OVERRIDE;


};

#endif /* ItemDelta_hpp */
