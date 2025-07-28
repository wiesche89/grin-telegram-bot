FROM ubuntu:22.04

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive
ENV QT_VERSION=6.2
ENV LD_LIBRARY_PATH=/usr/local/lib
ENV QT_QPA_PLATFORM_PLUGIN_PATH=/usr/lib/x86_64-linux-gnu/qt6/plugins/platforms
ENV PATH=/usr/lib/qt6/bin:$PATH
ENV QMAKE=/usr/lib/qt6/bin/qmake

# Install system dependencies
RUN apt-get update && \
    apt-get install -y software-properties-common && \
    add-apt-repository ppa:savoury1/qt-6-2 && \
    apt-get update && \
    apt-get install -y \
    build-essential \
    git \
    wget \
    curl \
    unzip \
    ca-certificates \
    qt6-base-dev \
    qt6-tools-dev \
    qt6-base-dev-tools \
    qt6-l10n-tools \
    qt6-tools-dev-tools \
    qt6-svg-dev \
    qt6-multimedia-dev \
    qt6-connectivity-dev \
    qt6-websockets-dev \
    qt6-networkauth-dev \
    libssl-dev \
    libssl3 \
    pkg-config \
    zlib1g-dev \
    libcurl4-openssl-dev \
    libjsoncpp-dev \
    autoconf \
    libtool \
    tor \
    xz-utils && \
    apt-get clean

# Set working directory
WORKDIR /opt

# Build and install libsecp256k1
RUN git clone https://github.com/bitcoin-core/secp256k1.git && \
    cd secp256k1 && \
    ./autogen.sh && \
    ./configure --enable-module-recovery --enable-experimental --enable-module-ecdh && \
    make -j$(nproc) && \
    make install && \
    echo "/usr/local/lib" > /etc/ld.so.conf.d/secp256k1.conf && ldconfig

# Clone grin-telegram-bot repository
RUN git clone --branch qt6.9 --single-branch https://github.com/wiesche89/grin-telegram-bot.git

# Copy project configuration and assets
COPY etc /opt/grin-telegram-bot/etc
COPY .wallet /opt/grin-telegram-bot/.wallet/
COPY qt.conf /opt/grin-telegram-bot/qt.conf
COPY qtlogging.ini /opt/grin-telegram-bot/qtlogging.ini
COPY .grin /root/.grin/
COPY start.sh /opt/grin-telegram-bot/start.sh
RUN chmod +x /opt/grin-telegram-bot/start.sh

# Set working directory to project root
WORKDIR /opt/grin-telegram-bot

# Download and extract grin-wallet
RUN wget https://github.com/mimblewimble/grin-wallet/releases/download/v5.4.0-alpha.1/grin-wallet-v5.4.0-alpha.1-linux-x86_64.tar.gz && \
    tar -xzf grin-wallet-v5.4.0-alpha.1-linux-x86_64.tar.gz && \
    chmod +x grin-wallet && \
    rm -f grin-wallet-v5.4.0-alpha.1-linux-x86_64.tar.gz

# Build the application using Qt 6.2
RUN $QMAKE grin-telegram-bot.pro && make -j$(nproc)

# Start command
CMD ["./start.sh"]
