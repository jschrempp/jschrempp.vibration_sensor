# vibration sensor

This project will announce to your smart phone when your clothes dryer stops running. This is useful when your clothes dryer is in a location (such as a gargage) when you are unable to hear the buzzer alarm built into the dryer.

This project uses the Hiletgo SW-420 vibration sensor module (http://hiletgo.com/ProductDetail/2163677.html)  The module has a potentiometer for adjusting sensitivity. At the highest sensitivity the sensor will report even slight vibrations. The sensor returns a binary is/isnot vibrating. According to the documentation it will change states at approximately 5 FPS (200 msec) but in practice we believe it will change state faster.

This code runs on a Particle.io Argon but should be easily ported to other Arduino compatible microprocessors.

The code has three filter stages.

1. Small vibrations will trigger "vibrating" that continues until no vibrations are detected for a period of 1.5 seconds. The sensitivity of the SW-420 module should be fairly sensitive; enough that a slap on the table causes "vibrating" but residual vibration does not keep "vibrating" true for longer than 2 seconds. 

2. "Long Duration Vibration" is triggered when "vibrating" is true for 5 seconds. The state continues until "vibrating" is false for 10 seconds. This makes sure that intermittent calm periods are bridged. Once Long Duration Vibration is true, it takes a real calm period to end the state.

3. "Dryer Running" is triggered by 60 seconds of the "Long Duration Vibration" state. Dryer Running perists until Long Duration Vibration is false for 30 seconds.
