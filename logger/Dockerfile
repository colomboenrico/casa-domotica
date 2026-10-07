# Stage 1: compilazione
FROM debian:bookworm AS build
RUN apt-get update && apt-get install -y --no-install-recommends g++ libmosquitto-dev libsqlite3-dev
WORKDIR /src
COPY logger.cpp .
RUN g++ -std=c++17 -O2 logger.cpp -o logger -lmosquitto -lsqlite3

# Stage 2: immagine finale
FROM debian:bookworm-slim
RUN apt-get update \
    && apt-get install -y --no-install-recommends libmosquitto1 libsqlite3-0 sqlite3 \
    && rm -rf /var/lib/apt/lists/*
COPY --from=build /src/logger /usr/local/bin/logger
ENV DB_PATH=/usr/local/bin/casa.db
ENTRYPOINT ["logger"]
