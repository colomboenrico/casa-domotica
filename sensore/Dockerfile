# Stage 1: compilazione 
FROM debian:bookworm AS build
RUN apt-get update && apt-get install -y --no-install-recommends g++ libmosquitto-dev
WORKDIR /src
COPY sensore.cpp .
RUN g++ -std=c++17 -O2 sensore.cpp -o sensore -lmosquitto

# Stage 2: immagine finale
FROM debian:bookworm-slim
RUN apt-get update \
    && apt-get install -y --no-install-recommends libmosquitto1 \
    && rm -rf /var/lib/apt/lists/*
COPY --from=build /src/sensore /usr/local/bin/sensore
ENTRYPOINT ["sensore"]
CMD ["soggiorno"]