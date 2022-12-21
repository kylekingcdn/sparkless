QT -= gui
QT += xml

CONFIG += c++17 console
CONFIG -= app_bundle

HEADERS += \
  src/Constants.hpp \
  src/utils/DmgMounter.hpp \
  src/utils/DsaSignatureGenerator.hpp \
  src/utils/EdDsaSignatureGenerator.hpp \
  src/utils/DeltaGenerator.hpp \
  src/ItemEnclosure.hpp \
  src/ItemDelta.hpp \
  src/AppcastItem.hpp \
  src/Appcast.hpp

SOURCES += \
  src/Constants.cpp \
  src/utils/DmgMounter.cpp \
  src/utils/DsaSignatureGenerator.cpp \
  src/utils/EdDsaSignatureGenerator.cpp \
  src/utils/DeltaGenerator.cpp \
  src/ItemEnclosure.cpp \
  src/ItemDelta.cpp \
  src/AppcastItem.cpp \
  src/Appcast.cpp \
  src/main.cpp

INCLUDEPATH += src
