//
//  Appcast.hpp
//  sparkless
//
//  Created by Kyle King on 2020-03-14.
//  Copyright © 2020 Kyle King. All rights reserved.
//

#ifndef Appcast_hpp
#define Appcast_hpp

#include <QObject>
#include <QDomDocument>
#include <QHash>

class AppcastItem;

class Appcast : public QObject {
  Q_OBJECT

private:

  QDomDocument appcastDoc;

  QList<AppcastItem*> items;
  QHash<qlonglong, AppcastItem*> itemHash;


#pragma mark - Constructors -

#pragma mark Private
private:

  Appcast(const QDomDocument&, QObject* theParent = nullptr);

#pragma mark Public
public:

  static Appcast* FromDocument(const QDomDocument&, QObject* theParent = nullptr);

  virtual ~Appcast() Q_DECL_OVERRIDE;


#pragma mark - Accessors -

#pragma mark Private
private:

#pragma mark Public
public:

  const QList<AppcastItem*>& Items() const { return items; }

  AppcastItem* Item(const qlonglong theVersion) const;

  void PrintItems() const;


#pragma mark - Mutators -

#pragma mark Private
private:

  bool ParseXml();

#pragma mark Public
public:


};

#endif /* Appcast_hpp */
