# veloGen
Minimalist bike computer and hub dynamo power manager. 

A typical hub dynamo can deliver approx. 6 - 10 V at 0.5 A. Output current is inductively limited and stays relatively stable over speed. However voltage will go up. A custom made buck converter is used to extract maximum power and charge a 18650 liion cell. The front and back bike lights will be powered by this cell.

Speed and distance is tracked by measuring the zero crossings of the hub dynamo AC output. Data is collected in the CPU until read out by a mobile device over bluetooth and possibly published to a cloud service.

# Brainstorming

 * Will use ESP-WROOM-32 and make good use of the sleep modes
 * AC input zero crossing --> falling edge interrupt. Wake up CPU and increment counting reg.
 * Every second, keep track of:
   * Battery voltage. --> ADC + voltage divider to 3 V
   * Generator RMS(or peak) voltage. --> ADC + voltage divider to 3 V
   * Generator RMS current. --> 5 mOhm shunt + TS1101 + ADC 1 V
   * Battery current in and out --> 5 mOhm shunt + TS1101 + ADC 1 V
   * Realtime clock timestamp
* Store these datasets in a circular buffer in memory, until read out over BLE.
* Manage the Buck converter PWM to charge the battery with maximum current


## PCB

 * 3.3 V LDO
 * WROOM-32
 * Buck inductor
 * Buck Schottky
 * Buck High side switch (p ch?)
 * 4 x schottky diode for bridge rectifier
 * Smoothing cap (10 uF?)
 * 2 x current sensing shunt (Igenerator, Ibattery) + 2 x TS1101
 * IN system programming pins
 
## 3D printed case

 * Mounts with zip ties / verlcro straps on the frame between 2 tubes angeled at 120 deg
 * 2 Shellswhich clip / screw together
 * 5 mm wall thickness with a groove to place an O-ring between the shells
 * AC input cable (2 pin)
 * DC output + I2C cable (4 pin)
 
