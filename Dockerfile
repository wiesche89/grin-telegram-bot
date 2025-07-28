# Basis-Image mit Qt 6.6.2 und Buildtools
FROM carlonluca/qt-dev:6.6.2

# Umgebungsvariablen
ENV DEBIAN_FRONTEND=noninteractive
ENV LD_LIBRARY_PATH=/usr/local/lib

# Abhängigkeiten installieren
RUN apt-get update && apt-get install -y \
    libssl-dev \
    libcurl4-openssl-dev \
    libjsoncpp-dev \
    autoconf \
    libtool \
    tor \
    git \
    unzip \
    ca-certificates \
    xz-utils \
    python3 \
    && apt-get clean

# libsecp256k1 aus Github bauen
RUN git clone https://github.com/bitcoin-core/secp256k1.git && \
    cd secp256k1 && \
    ./autogen.sh && \
    ./configure --enable-module-recovery --enable-experimental --enable-module-ecdh && \
    make -j$(nproc) && \
    make install && \
    echo "/usr/local/lib" > /etc/ld.so.conf.d/secp256k1.conf && ldconfig

# Projekt klonen: Grin-Telegram-Bot (Branch qt6.9)
RUN git clone --branch qt6.9 --single-branch https://github.com/wiesche89/grin-telegram-bot.git grin-telegram-bot

# Working directory setzen
WORKDIR /grin-telegram-bot

# Projekt bauen
RUN qmake grin-telegram-bot.pro && make -j$(nproc)

# Konfigurationsdateien kopieren
COPY etc /grin-telegram-bot/etc
COPY .wallet /grin-telegram-bot/.wallet/
COPY qt.conf /grin-telegram-bot/qt.conf
COPY qtlogging.ini /grin-telegram-bot/qtlogging.ini
COPY .grin /root/.grin/
COPY start.sh /grin-telegram-bot/start.sh
RUN chmod +x /grin-telegram-bot/start.sh

# Grin Wallet herunterladen
RUN wget https://github.com/mimblewimble/grin-wallet/releases/download/v5.4.0-alpha.1/grin-wallet-v5.4.0-alpha.1-linux-x86_64.tar.gz && \
    tar -xzf grin-wallet-v5.4.0-alpha.1-linux-x86_64.tar.gz && \
    chmod +x grin-wallet && \
    rm -f grin-wallet-v5.4.0-alpha.1-linux-x86_64.tar.gz

# Startkommando
CMD ["./start.sh"]
