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
  src/util \
  src/api \
  src/api/common/attributes \
  src/api/node \
  src/api/wallet \
  src/api/node/foreign \
  src/api/wallet/foreign \
  src/api/wallet/attributes \
  src/api/node/attributes \
  src/api/node/owner \
  src/api/wallet/owner \
  src/telegrambot \
  src/grinwalletmanager \
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
  src/api/common/attributes/error.cpp \
  src/api/node/attributes/blindingfactor.cpp \
  src/api/node/attributes/blockheaderprintable.cpp \
  src/api/node/attributes/blocklisting.cpp \
  src/api/node/attributes/blockprintable.cpp \
  src/api/node/attributes/capabilities.cpp \
  src/api/node/attributes/difficulty.cpp \
  src/api/node/attributes/direction.cpp \
  src/api/node/attributes/input.cpp \
  src/api/node/attributes/locatedtxkernel.cpp \
  src/api/node/attributes/merkleproof.cpp \
  src/api/node/attributes/nodeversion.cpp \
  src/api/node/attributes/outputidentifier.cpp \
  src/api/node/attributes/outputlisting.cpp \
  src/api/node/attributes/outputprintable.cpp \
  src/api/node/attributes/peeraddr.cpp \
  src/api/node/attributes/peerdata.cpp \
  src/api/node/attributes/peerinfodisplay.cpp \
  src/api/node/attributes/poolentry.cpp \
  src/api/node/attributes/protocolversion.cpp \
  src/api/node/attributes/rangeproof.cpp \
  src/api/node/attributes/status.cpp \
  src/api/node/attributes/tip.cpp \
  src/api/node/attributes/transaction.cpp \
  src/api/node/attributes/transactionbody.cpp \
  src/api/node/attributes/txkernel.cpp \
  src/api/node/attributes/txkernelprintable.cpp \
  src/api/node/attributes/txsource.cpp \
  src/api/node/foreign/nodeforeignapi.cpp \
  src/api/node/owner/nodeownerapi.cpp \
  src/api/wallet/attributes/account.cpp \
  src/api/wallet/attributes/builtoutput.cpp \
  src/api/wallet/attributes/coinbase.cpp \
  src/api/wallet/attributes/com.cpp \
  src/api/wallet/attributes/commitment.cpp \
  src/api/wallet/attributes/config.cpp \
  src/api/wallet/attributes/inittxargs.cpp \
  src/api/wallet/attributes/inittxsendargs.cpp \
  src/api/wallet/attributes/kernel.cpp \
  src/api/wallet/attributes/loggingconfig.cpp \
  src/api/wallet/attributes/nodeheight.cpp \
  src/api/wallet/attributes/output.cpp \
  src/api/wallet/attributes/outputcommitmapping.cpp \
  src/api/wallet/attributes/outputdata.cpp \
  src/api/wallet/attributes/paymentproof.cpp \
  src/api/wallet/attributes/proof.cpp \
  src/api/wallet/attributes/query.cpp \
  src/api/wallet/attributes/rewindhash.cpp \
  src/api/wallet/attributes/signature.cpp \
  src/api/wallet/attributes/slatepack.cpp \
  src/api/wallet/attributes/torconfig.cpp \
  src/api/wallet/attributes/txlogentry.cpp \
  src/api/wallet/attributes/verifypaymentproofstatus.cpp \
  src/api/wallet/attributes/version.cpp \
  src/api/wallet/attributes/viewwallet.cpp \
  src/api/wallet/attributes/viewwalletentry.cpp \
  src/api/wallet/attributes/walletconfig.cpp \
  src/api/wallet/attributes/walletinfo.cpp \
  src/grinwalletmanager/grinwalletmanager.cpp \
  src/main.cpp \
  src/telegrambot/jsonhelper.cpp \
  src/telegrambot/modules/httpserver/httpserver.cpp \
  src/telegrambot/modules/sslserver/sslserver.cpp \
  src/telegrambot/telegrambot.cpp \
  src/util/jsonutil.cpp \
  src/worker/worker.cpp \
  src/api/wallet/foreign/walletforeignapi.cpp \
  src/api/wallet/owner/walletownerapi.cpp \
  src/api/wallet/attributes/slate.cpp \
  src/database/databasemanager.cpp \
  src/database/attributes/donate.cpp \
  src/database/attributes/faucet.cpp \
  src/util/debugutils.cpp

# .h
HEADERS += \
  src/api/common/attributes/error.h \
  src/api/common/attributes/result.h \
  src/api/node/attributes/blindingfactor.h \
  src/api/node/attributes/blockheaderprintable.h \
  src/api/node/attributes/blocklisting.h \
  src/api/node/attributes/blockprintable.h \
  src/api/node/attributes/capabilities.h \
  src/api/node/attributes/difficulty.h \
  src/api/node/attributes/direction.h \
  src/api/node/attributes/input.h \
  src/api/node/attributes/locatedtxkernel.h \
  src/api/node/attributes/merkleproof.h \
  src/api/node/attributes/nodeversion.h \
  src/api/node/attributes/outputfeatures.h \
  src/api/node/attributes/outputidentifier.h \
  src/api/node/attributes/outputlisting.h \
  src/api/node/attributes/outputprintable.h \
  src/api/node/attributes/peeraddr.h \
  src/api/node/attributes/peerdata.h \
  src/api/node/attributes/peerinfodisplay.h \
  src/api/node/attributes/poolentry.h \
  src/api/node/attributes/protocolversion.h \
  src/api/node/attributes/rangeproof.h \
  src/api/node/attributes/status.h \
  src/api/node/attributes/tip.h \
  src/api/node/attributes/transaction.h \
  src/api/node/attributes/transactionbody.h \
  src/api/node/attributes/txkernel.h \
  src/api/node/attributes/txkernelprintable.h \
  src/api/node/attributes/txsource.h \
  src/api/node/foreign/nodeforeignapi.h \
  src/api/node/owner/nodeownerapi.h \
  src/api/wallet/attributes/account.h \
  src/api/wallet/attributes/builtoutput.h \
  src/api/wallet/attributes/coinbase.h \
  src/api/wallet/attributes/com.h \
  src/api/wallet/attributes/commitment.h \
  src/api/wallet/attributes/config.h \
  src/api/wallet/attributes/inittxargs.h \
  src/api/wallet/attributes/inittxsendargs.h \
  src/api/wallet/attributes/kernel.h \
  src/api/wallet/attributes/loggingconfig.h \
  src/api/wallet/attributes/nodeheight.h \
  src/api/wallet/attributes/output.h \
  src/api/wallet/attributes/outputcommitmapping.h \
  src/api/wallet/attributes/outputdata.h \
  src/api/wallet/attributes/paymentproof.h \
  src/api/wallet/attributes/proof.h \
  src/api/wallet/attributes/query.h \
  src/api/wallet/attributes/rewindhash.h \
  src/api/wallet/attributes/signature.h \
  src/api/wallet/attributes/slatepack.h \
  src/api/wallet/attributes/torconfig.h \
  src/api/wallet/attributes/txlogentry.h \
  src/api/wallet/attributes/verifypaymentproofstatus.h \
  src/api/wallet/attributes/version.h \
  src/api/wallet/attributes/viewwallet.h \
  src/api/wallet/attributes/viewwalletentry.h \
  src/api/wallet/attributes/walletconfig.h \
  src/api/wallet/attributes/walletinfo.h \
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
  src/util/jsonutil.h \
  src/worker/worker.h \
  3rdParty/secp256k1/include/secp256k1.h \
  3rdParty/secp256k1/include/secp256k1_ecdh.h \
  3rdParty/openssl/openssl/evp.h \
  3rdParty/openssl/openssl/rand.h \
  src/api/wallet/attributes/slate.h \
  src/database/databasemanager.h \
  src/database/attributes/donate.h \
  src/database/attributes/faucet.h \
  src/util/debugutils.h

# secp256k1
unix:LIBS += -lsecp256k1
win32:LIBS += -L$$PWD/3rdParty/secp256k1/.libs -lsecp256k1
win64:LIBS += -L$$PWD/3rdParty/secp256k1/.libs -lsecp256k1
INCLUDEPATH += $$PWD/3rdParty/secp256k1/include

# openssl
unix:LIBS += -lcrypto
win32:LIBS += -L$$PWD/3rdParty/openssl/lib -llibcrypto
win64:LIBS += -L$$PWD/3rdParty/openssl/lib -llibcrypto

DEFINES += OPENSSL_NO_DEPRECATED_3_0
