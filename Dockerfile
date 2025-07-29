# Base image: Ubuntu 24.04 with system-wide Qt 6.6
FROM ubuntu:24.04

# Environment variables
ENV DEBIAN_FRONTEND=noninteractive
ENV LD_LIBRARY_PATH=/usr/local/lib
ENV DATA_DIR=/opt/grin-telegram-bot/data
ENV QT_QPA_PLATFORM=offscreen

# Install Qt and system dependencies
RUN apt-get update && apt-get install -y \
    qt6-base-dev \
    qt6-base-dev-tools \
    qt6-tools-dev \
    qt6-tools-dev-tools \
    qt6-websockets-dev \
    libqt6sql6 \
    libqt6sql6-sqlite \
    libssl-dev \
    libcurl4-openssl-dev \
    libjsoncpp-dev \
    build-essential \
    autoconf \
    libtool \
    tor \
    git \
    unzip \
    ca-certificates \
    wget \
    python3 \
    xz-utils \
    strace \
    && apt-get clean

# Build and install libsecp256k1 from GitHub
RUN git clone https://github.com/bitcoin-core/secp256k1.git && \
    cd secp256k1 && \
    ./autogen.sh && \
    ./configure --disable-dependency-tracking \
                --enable-module-recovery \
                --enable-experimental \
                --enable-module-ecdh && \
    make -j$(nproc) && \
    make install && \
    echo "/usr/local/lib" > /etc/ld.so.conf.d/secp256k1.conf && ldconfig

# Clone the Grin Telegram Bot project (branch qt6.9)
RUN git clone --branch 0.2 --single-branch https://github.com/wiesche89/grin-telegram-bot.git /grin-telegram-bot

# Set working directory
WORKDIR /grin-telegram-bot

# Build the project
RUN qmake6 grin-telegram-bot.pro && make -j$(nproc)

# Download and extract the Grin Wallet binary
RUN wget https://github.com/mimblewimble/grin-wallet/releases/download/v5.4.0-alpha.1/grin-wallet-v5.4.0-alpha.1-linux-x86_64.tar.gz && \
    tar -xzf grin-wallet-v5.4.0-alpha.1-linux-x86_64.tar.gz && \
    chmod +x grin-wallet && \
    rm -f grin-wallet-v5.4.0-alpha.1-linux-x86_64.tar.gz

# Copy startup script into the image
COPY start.sh /grin-telegram-bot/start.sh
RUN chmod +x /grin-telegram-bot/start.sh

# Set default command
CMD ["./start.sh"]
