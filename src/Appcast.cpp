//
//  Appcast.cpp
//  sparkless
//
//  Created by Kyle King on 2020-03-14.
//  Copyright © 2020 Kyle King. All rights reserved.
//

#include "Appcast.hpp"

#include <QDebug>

#include "AppcastItem.hpp"

#pragma mark - Constructors -

#pragma mark Private

Appcast::Appcast(const QDomDocument& theXmlDoc, QObject* theParent)
: QObject(theParent) {

  appcastDoc = theXmlDoc;
}

#pragma mark Public

Appcast* Appcast::FromDocument(const QDomDocument& theXmlDoc, QObject* theParent) {

  Appcast* appcast = new Appcast(theXmlDoc, theParent);

  if (!appcast->ParseXml()) {
    delete appcast;
    appcast = nullptr;
  }

  return appcast;
}

Appcast::~Appcast() {

  qDeleteAll(items);
}


#pragma mark - Accessors -

#pragma mark Private

#pragma mark Public

AppcastItem* Appcast::Item(const qlonglong theVersion) const {

  if (theVersion < 0) {
    return nullptr;
  }

  return itemHash.value(theVersion);
}

void Appcast::PrintItems() const {

  foreach (AppcastItem* currItem, items) {

    if (currItem != nullptr) {
      currItem->Print();
    }
  }
}

#pragma mark - Mutators -

#pragma mark Private

bool Appcast::ParseXml() {

  QDomElement rssElement = appcastDoc.firstChildElement("rss");
  QDomElement channelElement = rssElement.firstChildElement("channel");

  QDomElement itemElement = channelElement.firstChildElement("item");

  while (!itemElement.isNull()) {

    AppcastItem* item = AppcastItem::FromElement(itemElement, this);

    if (item != nullptr) {

      items.append(item);

      if (item->VersionBuild() >= 0) {
        itemHash.insert(item->VersionBuild(), item);
      }
    }

    itemElement = itemElement.nextSiblingElement("item");
  }

  return true;
}

#pragma mark Public
