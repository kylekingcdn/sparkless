//
//  Constants.hpp
//  sparkless
//
//  Created by Kyle King on 2020-03-14.
//  Copyright © 2020 Kyle King. All rights reserved.
//

#ifndef Constants_hpp
#define Constants_hpp

#include <QMetaType>

enum EnclosurePlatform {
  NullPlatform = 0,
  MacPlatform,
  WindowsPlatform,
};

Q_DECLARE_METATYPE(EnclosurePlatform);

enum EnclosureSignatureType {
  NullSignature = 0,
  DsaSignature,
  Ed25519Signature,
};

Q_DECLARE_METATYPE(EnclosureSignatureType);

#endif /* Constants_hpp */
