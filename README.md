# RelaysControl

Proyecto de control de relés basado en ESP32 y PlatformIO.

## Descripción

Este repositorio contiene el firmware para un sistema de automatización de relés. El código está organizado en:

- `include/`: cabeceras del sistema, comandos y configuración de relés.
- `src/`: implementación del firmware, lógica de comandos y temporizadores.
- `lib/`: documentación adicional de librerías.
- `test/`: documentación para pruebas.

## Plataforma

- Placa: `esp32dev`
- Framework: `arduino`
- Velocidad de monitor serie: `115200`

## Cómo compilar

1. Instala PlatformIO en VS Code o usa la CLI de PlatformIO.
2. Abre este proyecto en VS Code.
3. Ejecuta el task de PlatformIO para compilar o usa:

```bash
platformio run
```

## Cómo subir al ESP32

```bash
platformio run --target upload
```

## Estructura del proyecto

- `platformio.ini`: configuración de PlatformIO.
- `src/main.cpp`: entrada principal del firmware.
- `include/`: definición de relés, comandos y límites del sistema.

## Notas

El repositorio ya está conectado con GitHub en `https://github.com/Agus-yanez/RelaysControl.git`.
