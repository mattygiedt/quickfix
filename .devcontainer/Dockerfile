FROM gcc:11.2

WORKDIR /

RUN echo "Installing development tools ..."; \
    set -ex; \
    apt-get update && export DEBIAN_FRONTEND=noninteractive; \
    apt-get install -y --no-install-recommends \
        build-essential \
        vim \
        gdb \
        htop \
        cmake \
        python-dev \
        autotools-dev \
        libicu-dev \
        libbz2-dev \
        libgoogle-perftools-dev \
        clang-tidy \
        clang-format;

RUN echo "Installing spdlog ..."; \
    set -ex; \
    git clone https://github.com/gabime/spdlog.git; \
    cd spdlog; \
    mkdir build; \
    cd build; \
    cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=RelWithDebInfo ..; \
    make -j4 install; \
    rm -rf /spdlog;

RUN echo "Installing quickfix ..."; \
    set -ex; \
    git clone https://github.com/quickfix/quickfix.git; \
    cd quickfix; \
    mkdir build; \
    cd build; \
    cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=RelWithDebInfo ..; \
    make -j4 install; \
    rm -rf /quickfix;

RUN echo "Installing nlohmann/json ..."; \
    set -ex; \
    git clone https://github.com/nlohmann/json.git; \
    cd json; \
    mkdir build; \
    cd build; \
    cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DJSON_BuildTests=OFF ..; \
    make -j4 install; \
    rm -rf /json;


RUN echo "export LD_LIBRARY_PATH ...";
ENV LD_LIBRARY_PATH=/usr/local/lib;

