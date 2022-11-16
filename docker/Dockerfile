FROM ubuntu:latest

SHELL ["/bin/bash", "-c"]

###############################################################################
# Update apt
###############################################################################
RUN apt-get update -qq

###############################################################################
# Install prereqs
###############################################################################
RUN DEBIAN_FRONTEND=noninteractive apt-get -y install \
    git \
    curl \
    sudo \
    # Python
    python3 \
    python3-dev \
    python3-pip \
    build-essential \
    cmake \
    # For PPAs
    software-properties-common \
    apt-transport-https

###############################################################################
# Python/AWS CLI
###############################################################################
RUN pip3 install --upgrade pip \
    && pip3 install awscli \
    && aws --version

###############################################################################
# OpenSSL
###############################################################################
RUN set -ex apt install build-essential
RUN set -ex \
    && ([ -d /opt/openssl ] && rm -rf /opt/openssl) || true \
    && mkdir -p /tmp/build \
    && cd /tmp/build \
    && git clone https://github.com/openssl/openssl.git \
    && pushd openssl \
    && git checkout OpenSSL_1_1_1-stable \
    && ./config -fPIC \
    no-md2 no-rc5 no-rfc3779 no-sctp no-ssl-trace no-zlib no-hw no-mdc2 \
    no-seed no-idea no-camellia no-bf no-dsa no-ssl3 no-capieng \
    no-unit-test no-tests \
    -DSSL_FORBID_ENULL -DOPENSSL_NO_DTLS1 -DOPENSSL_NO_HEARTBEATS \
    --prefix=/opt/openssl --openssldir=/opt/openssl \
    && make -j \
    && make install_sw \
    && LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/openssl/lib /opt/openssl/bin/openssl version

RUN apt-get update \
    && apt-get install -y git \
    g++ \
    make \
    wget

RUN set -ex \
    && apt-get install -y libcurl4-openssl-dev libssl-dev uuid-dev zlib1g-dev libpulse-dev
RUN set -ex\
    && apt-get install -y curl zip unzip tar locate
RUN set -ex\
    && git clone https://github.com/microsoft/vcpkg
RUN apt-get install -y iputils-ping 
RUN apt-get install -y net-tools
RUN apt install iproute2 -y
RUN apt install gdb -y
RUN apt install clang-format -y 
RUN pip install conan
# RUN set -ex\
#     && ./vcpkg/bootstrap-vcpkg.sh\
#     && ./vcpkg/vcpkg install "aws-sdk-cpp[s3, sns,sqs]"

RUN set -ex\
    && git clone --recurse-submodules https://github.com/aws/aws-sdk-cpp
RUN set -ex\
    && mkdir sdk_build\
    && cd sdk_build\
    && cmake ../aws-sdk-cpp -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=/usr/local/ -DCMAKE_INSTALL_PREFIX=/usr/local/ -DBUILD_ONLY="sns"\
    && make\
    && make install

RUN set -ex \
    && mkdir work \
    && cd work \
    && git clone https://github.com/abhilashraju/Bongo.git \
    && cd Bongo\
    && conan create .

RUN set -ex \
    && cd work \
    && git clone https://github.com/abhilashraju/Bingo.git 

RUN set -ex\
    && echo ./work/Bingo/examples/build/aws_service/aws_sns_publisher /work/Bingo/examples/aws_service/resources 9003 > run.sh\
    && chmod 777 run.sh

RUN set -ex \
    && cd work/Bingo \
    && git pull \
    && ./configure

# CMD ["bash","./run.sh"]




###############################################################################
# Cleanup
###############################################################################
# RUN set -ex \
#     && apt-get update -qq \
#     && apt-get clean \
#     && rm -rf /tmp/*
