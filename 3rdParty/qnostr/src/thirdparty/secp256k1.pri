
!contains(DEFINES, SECP256K1_ENABLED) {
    SECP256K1_ROOT = $$PWD/secp256k1
    exists($$SECP256K1_ROOT/include/secp256k1.h) {
        message(Using secp256k1 bundle from $$SECP256K1_ROOT)
    } else {
        SECP256K1_ROOT = $$PWD/../../../../secp256k1
        exists($$SECP256K1_ROOT/include/secp256k1.h) {
            message(Using external secp256k1 from $$SECP256K1_ROOT)
        } else {
            SECP256K1_ROOT = $$PWD/../../../../3rdParty/secp256k1
            exists($$SECP256K1_ROOT/include/secp256k1.h) {
                message(Using git submodule secp256k1 from $$SECP256K1_ROOT)
            } else {
                error("secp256k1 headers not found – please clone them next to the project")
            }
        }
    }

    INCLUDEPATH += \
        $$SECP256K1_ROOT/include/

    DEFINES += \
        SECP256K1_ENABLED \
        ENABLE_MODULE_ECDH \
        ENABLE_MODULE_RECOVERY \
        ENABLE_MODULE_EXTRAKEYS \
        ENABLE_MODULE_SCHNORRSIG \
        ENABLE_MODULE_ELLSWIFT

    SECP_SOURCES = \
        $$SECP256K1_ROOT/src/secp256k1.c \
        $$SECP256K1_ROOT/src/precomputed_ecmult_gen.c \
        $$SECP256K1_ROOT/src/precomputed_ecmult.c

    if (exists($$SECP256K1_ROOT/src/secp256k1.c)) {
        SOURCES += $$SECP_SOURCES
    } else {
        SECP_LIB_DIR = $$SECP256K1_ROOT/.libs
        exists($$SECP_LIB_DIR/libsecp256k1.dll.a) {
            message(Linking against prebuilt secp256k1 libs in $$SECP_LIB_DIR)
        } else {
            SECP_LIB_DIR = $$SECP256K1_ROOT/lib
        }

        exists($$SECP_LIB_DIR/libsecp256k1.dll.a) {
            LIBS += -L$$SECP_LIB_DIR -lsecp256k1
        } else {
            error("secp256k1 libs not found in $$SECP_LIB_DIR")
        }
    }
}
