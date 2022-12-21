//
//  ItemDelta.cpp
//  sparkless
//
//  Created by Kyle King on 2020-03-14.
//  Copyright © 2020 Kyle King. All rights reserved.
//

#include "ItemDelta.hpp"

#include <QDebug>

ItemDelta::ItemDelta(const qlonglong theLength, const qlonglong thePrevBuild, const qlonglong theNewBuild, const QString& theVersion, const QUrl& theUrl, const EnclosurePlatform thePlatform, const QByteArray& theSignature, const EnclosureSignatureType theSignatureType, QObject* theParent)
: ItemEnclosure(theLength, theNewBuild, theVersion, theUrl, thePlatform, theSignature, theSignatureType, theParent) {

  initialVersionBuild = thePrevBuild;

}

ItemDelta::ItemDelta(const QDomElement& theEnclosureElement, QObject* theParent)
: ItemEnclosure(theEnclosureElement, theParent) {

}

ItemDelta* ItemDelta::FromElement(const QDomElement& theEnclosureElement, QObject* theParent) {

  ItemDelta* enclsoure = new ItemDelta(theEnclosureElement, theParent);

  if (!enclsoure->ParseXml()) {
    delete enclsoure;
    enclsoure = nullptr;
  }

  return enclsoure;
}

ItemDelta* ItemDelta::NewDelta(const qlonglong theLength, const qlonglong thePrevBuild, const qlonglong theNewBuild, const QString& theVersion, const QUrl& theUrl, const EnclosurePlatform thePlatform, const QByteArray& theSignature, const EnclosureSignatureType theSignatureType, QObject* theParent) {

  ItemDelta* delta = new ItemDelta(theLength, thePrevBuild, theNewBuild, theVersion, theUrl, thePlatform, theSignature, theSignatureType, theParent);

  return delta;
}

ItemDelta::~ItemDelta() {

}

void ItemDelta::Print() const {

  ItemEnclosure::Print();

  qInfo().noquote().nospace() << "Delta Version Intitial: " << initialVersionBuild;
}

bool ItemDelta::ParseXml() {

  if (!ItemEnclosure::ParseXml()) {
    return false;
  }

  initialVersionBuild = EnclosureElement().attribute("sparkle:deltaFrom").toLongLong();
  
  return true;
}

bool ItemDelta::Serialize(QDomElement& theDeltaElement) {

  if (!ItemEnclosure::Serialize(theDeltaElement)) {
    return false;
  }

  theDeltaElement.setAttribute("sparkle:deltaFrom", QString::number(initialVersionBuild));

  return true;
}
