# Casa domotica

Sistema IoT di esempio: sensori simulati in C++ pubblicano temperatura e umidità via MQTT,
un logger C++ salva le letture in SQLite, una dashboard React le mostra in tempo reale.
Tutto gira in Docker con un solo comando.

## Architettura

```
[3 sensori C++] ──MQTT──▶ [Mosquitto] ──MQTT──▶ [Logger C++] ──▶ [SQLite]
                              │
                              └──WebSocket──▶ [Dashboard React]
```

## Tecnologie

- **C++17** con libmosquitto e SQLite
- **MQTT** con broker Eclipse Mosquitto (QoS, retain, wildcard)
- **SQLite** con prepared statement
- **React** + Vite, client MQTT su WebSocket
- **Docker** con build multi-stage e Docker Compose

## Avvio

```bash
docker compose up -d --build
```

| Servizio | Indirizzo |
|---|---|
| Dashboard | http://localhost:8085 |
| Broker MQTT | localhost:1884 |
| Broker WebSocket | ws://localhost:9001 |

## Struttura

| Cartella | Contenuto |
|---|---|
| `sensore/` | sensore simulato: temperatura, umidità, allarme con cambio di stato |
| `logger/` | subscriber che salva le letture nel database |
| `dashboard/` | interfaccia React |
| `mosquitto/` | configurazione del broker |

## Interrogare il database

```bash
docker compose exec logger sqlite3 /data/casa.db "SELECT sensore, COUNT(*) FROM letture GROUP BY sensore;"
```