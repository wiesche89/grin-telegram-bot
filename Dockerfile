FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
ENV LD_LIBRARY_PATH=/usr/local/lib
ENV QT_QPA_PLATFORM_PLUGIN_PATH=/usr/lib/x86_64-linux-gnu/qt5/plugins/platforms

# Install dependencies
RUN apt-get update && \
    apt-get install -y software-properties-common && \
    add-apt-repository universe && \
    apt-get update && \
    apt-get install -y \
    build-essential \
    git \
    wget \
    curl \
    unzip \
    ca-certificates \
    qt5-qmake \
    qtbase5-dev \
    libqt5websockets5-dev \
    qtbase5-dev-tools \
    qttools5-dev-tools \
    qtchooser \
    libssl-dev \
    pkg-config \
    zlib1g-dev \
    libcurl4-openssl-dev \
    libjsoncpp-dev && \
    apt-get clean

# Create app directory
WORKDIR /opt

RUN apt-get install -y autoconf libtool

# Installiere libsecp256k1
RUN git clone https://github.com/bitcoin-core/secp256k1.git && \
    cd secp256k1 && \
    ./autogen.sh && \
    ./configure --enable-module-recovery --enable-experimental --enable-module-ecdh && \
    make -j$(nproc) && \
    make install && \
    echo "/usr/local/lib" > /etc/ld.so.conf.d/secp256k1.conf && ldconfig

# Clone grin-telegram-bot
RUN git clone https://github.com/wiesche89/grin-telegram-bot.git


# Dummy build arg to invalidate cache
ARG CACHEBUST=7

# WICHTIG: etc-Ordner aus dem deploy-Verzeichnis mit rein kopieren
COPY etc /opt/grin-telegram-bot/etc
COPY qt.conf /opt/grin-telegram-bot/qt.conf
COPY qtlogging.ini /opt/grin-telegram-bot/qtlogging.ini
COPY .grin /root/.grin/

WORKDIR /opt/grin-telegram-bot

RUN wget https://github.com/mimblewimble/grin-wallet/releases/download/v5.4.0-alpha.1/grin-wallet-v5.4.0-alpha.1-linux-x86_64.tar.gz

RUN tar -xzf grin-wallet-v5.4.0-alpha.1-linux-x86_64.tar.gz && \
    chmod +x grin-wallet && \
    rm -f grin-wallet-v5.4.0-alpha.1-linux-x86_64.tar.gz
	
RUN apt-get update && apt-get install -y libssl3	
	
# Build project
RUN qmake grin-telegram-bot.pro && make -j$(nproc)

CMD ["./grin-telegram-bot"]
