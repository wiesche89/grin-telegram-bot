QT -= gui
QT += core websockets

CONFIG += console
CONFIG -= app_bundle
CONFIG += c++14

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += \
  src \
  src/worker \
  src/telegrambot \
  src/telegrambot/modules/httpserver \
  src/telegrambot/modules/sslserver

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
  src/main.cpp \
  src/telegrambot/jsonhelper.cpp \
  src/telegrambot/modules/httpserver/httpserver.cpp \
  src/telegrambot/modules/sslserver/sslserver.cpp \
  src/telegrambot/telegrambot.cpp \
  src/worker/worker.cpp

HEADERS += \
  src/telegrambot/jsonhelper.h \
  src/telegrambot/modules/httpserver/httpserver.h \
  src/telegrambot/modules/sslserver/sslserver.h \
  src/telegrambot/qdelegate.h \
  src/telegrambot/telegrambot.h \
  src/telegrambot/telegramdatainterface.h \
  src/telegrambot/telegramdatastructs.h \
  src/worker/worker.h
