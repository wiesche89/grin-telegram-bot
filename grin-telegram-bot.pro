# Modules
QT -= gui
QT += core websockets sql

# Config
CONFIG += console
CONFIG -= app_bundle
CONFIG += c++14

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# Include-Folder
INCLUDEPATH += \
  src \
  src/worker \
  src/api \
  src/api/node \
  src/api/wallet \
  src/api/node/foreign \
  src/api/wallet/foreign \
  src/api/wallet/attributes \
  src/api/node/owner \
  src/api/wallet/owner \
  src/telegrambot \
  src/database \
  src/database/attributes \
  src/telegrambot/modules/httpserver \
  src/telegrambot/modules/sslserver \
  3rdParty \
  3rdParty/secp256k1 \
  3rdParty/secp256k1/include \
  3rdParty/openssl \
  3rdParty/openssl/openssl

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# .cpp
SOURCES += \
  src/api/node/foreign/nodeforeignapi.cpp \
  src/api/node/owner/nodeownerapi.cpp \
    src/api/wallet/attributes/transaction.cpp \
  src/grinwalletmanager/grinwalletmanager.cpp \
  src/main.cpp \
  src/telegrambot/jsonhelper.cpp \
  src/telegrambot/modules/httpserver/httpserver.cpp \
  src/telegrambot/modules/sslserver/sslserver.cpp \
  src/telegrambot/telegrambot.cpp \
  src/worker/worker.cpp \
  src/api/wallet/foreign/walletforeignapi.cpp \
  src/api/wallet/owner/walletownerapi.cpp \
  src/api/wallet/attributes/slate.cpp \
  src/api/wallet/attributes/summaryinfo.cpp \
  src/database/databasemanager.cpp \
    src/database/attributes/donate.cpp \
    src/database/attributes/faucet.cpp

# .h
HEADERS += \
  src/api/node/foreign/nodeforeignapi.h \
  src/api/node/owner/nodeownerapi.h \
    src/api/wallet/attributes/transaction.h \
  src/api/wallet/foreign/walletforeignapi.h \
  src/api/wallet/owner/walletownerapi.h \
  src/grinwalletmanager/grinwalletmanager.h \
  src/telegrambot/jsonhelper.h \
  src/telegrambot/modules/httpserver/httpserver.h \
  src/telegrambot/modules/sslserver/sslserver.h \
  src/telegrambot/qdelegate.h \
  src/telegrambot/telegrambot.h \
  src/telegrambot/telegramdatainterface.h \
  src/telegrambot/telegramdatastructs.h \
  src/worker/worker.h \
  3rdParty/secp256k1/include/secp256k1.h \
  3rdParty/secp256k1/include/secp256k1_ecdh.h \
  3rdParty/openssl/openssl/evp.h \
  3rdParty/openssl/openssl/rand.h \
  src/api/wallet/attributes/slate.h \
  src/api/wallet/attributes/summaryinfo.h \
  src/database/databasemanager.h \
    src/database/attributes/donate.h \
    src/database/attributes/faucet.h

# secp256k1
LIBS += -L$$PWD/3rdParty/secp256k1/.libs -lsecp256k1
INCLUDEPATH += $$PWD/3rdParty/secp256k1/include

# openssl
unix:LIBS += -lcrypto
win32:LIBS += -L$$PWD/3rdParty/openssl/lib -llibcrypto

DEFINES += OPENSSL_NO_DEPRECATED_3_0
