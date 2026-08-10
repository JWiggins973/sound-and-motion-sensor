# Alert Backend

A home alert system. An ESP32 watches a door and a microphone, alerts to
Discord and this backend, and the backend logs and displays it all.

## Why I built this
- Didn't want to buy a front door sensor
- Wanted to track how loud my apartment gets
- Decibel values aren't 100% accurate, but calibrated closely to AirPods Pro 2 Live Listen readings

## Tech Stack
- Arduino
- C#
- .NET 10 SDK
- SQLite
- Arduino IDE with ESP32 board support installed

## Hardware
- An ESP32 board (built and tested on an Arduino Nano ESP32 S3)

## Sensors
- MAX4466 electret microphone amplifier (sound level)
- HC-SR04 ultrasonic distance sensor (door detection)

## Project Structure
```
AlertBackend/
├── Arduino/
│   ├── sensor.ino
│   └── secrets.h.example
├── Data/
│   ├── AlertDbContext.cs
│   └── LiveStatusStore.cs
├── Dtos/
│   ├── AlertEventDto.cs
│   └── LiveStatusDto.cs
├── Model/
│   └── AlertEvent.cs
├── Pages/
│   ├── AlertLog.cshtml
│   └── Live.cshtml
├── Migrations/
├── Program.cs
└── alerts.http
```

## API
| Endpoint | Method | Description |
|---|---|---|
| `/alert` | POST | Save a new alert event to the database |
| `/alerts` | GET | Get all saved alert events |
| `/update-live-status` | POST | Update the current live sound/door reading |
| `/live-status` | GET | Get the current live sound/door reading |
| `/AlertLog` | GET | History page with filters and pagination |
| `/Live` | GET | Live sound/door status dashboard |

See `alerts.http` for example requests.

## How to run
1. Create `secrets.h` in `Arduino/` using `secrets.h.example` as a template
2. Start the backend:
   ```bash
   dotnet run
   ```
3. Flash `sensor.ino` to your Arduino Nano ESP32 S3

## Tests

- Playwright coming soon

