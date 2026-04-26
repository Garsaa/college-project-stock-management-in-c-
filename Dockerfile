FROM debian:trixie-slim AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        cmake \
        libbrotli-dev \
        libdrogon-dev \
        libhiredis-dev \
        libjsoncpp-dev \
        libmariadb-dev \
        libmariadb-dev-compat \
        libpq-dev \
        libsqlite3-dev \
        libssl-dev \
        libyaml-cpp-dev \
        ninja-build \
        pkg-config \
        uuid-dev \
        zlib1g-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src

COPY CMakeLists.txt ./
COPY include ./include
COPY src ./src
COPY sql ./sql

RUN cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --config Release

FROM debian:trixie-slim AS runtime

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        ca-certificates \
        curl \
        libdrogon1t64 \
        libpq5 \
    && rm -rf /var/lib/apt/lists/* \
    && useradd --system --create-home --home-dir /app --shell /usr/sbin/nologin appuser

WORKDIR /app

COPY --from=builder /src/build/inventory_api /app/inventory_api
COPY --from=builder /src/sql /app/sql

RUN chown -R appuser:appuser /app

USER appuser

ENV APP_HOST=0.0.0.0
ENV APP_PORT=2020

EXPOSE 2020

HEALTHCHECK --interval=30s --timeout=5s --start-period=20s --retries=3 \
    CMD sh -c 'curl -fsS "http://127.0.0.1:${PORT:-${APP_PORT:-2020}}/health" >/dev/null || exit 1'

CMD ["./inventory_api"]
