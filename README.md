# Dependencies

Right now there's quite a hodge podge of these as I experiment with different libraries.

 * ESP8266WiFi

 * ArduinoOTA

 * https://github.com/me-no-dev/ESPAsyncWebServer

 * https://github.com/FastLED/FastLED

 * https://github.com/kitesurfer1404/WS2812FX

 * https://github.com/gmag11/ESPNtpClient/

# Future Work

### TODOs

  1. Leverage palettes code to drive WS2812fx effects, for example Fireworks.
  2. Allow coupling speed to "time until off" or other events to ramp things down.

### Scene Descriptors

```
NODE/Strand/Bulbs

NODE := IP | HOST
Strand := [0..8] # Maybe allow locally named strands.
Bulbs := [0, 1, 2..10]
Bulbs := N % 2
```