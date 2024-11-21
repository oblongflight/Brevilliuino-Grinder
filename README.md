## This project adds several features to the Breville Barista Express:
- Adds stop-on-mass grinding via a relay connected to the grinder motor and a load cell
- Turns entire machine on or off using a relay
- Manages the front panel LEDs
- Interprets potentiometer input to select grind mass target

## Pinouts
**The LilyGo T-Display S3 is available in a plastic case with presoldered female headers. This is the version I used. 5V is supplied to the JST battery connector at the bottom of the PCB.**
![pinout](/T-displayS3.png)
![plastic case](/T-DisplayS3Case.png)

## Hardware
**I replaced the prefabricated front button PCBs with ones I soldered myself because I couldn't figure out the pinouts from the original PCBs. This was very easy using just typical prototyping boards with the originals as templates.**
**The front panel LEDs are controlled by a TLC5947. This seems very sensitive to small fluctuations in supply voltage or maybe EMI from solenoid operation elsewhere in the machine, hence the occasional call to resetLEDs() to equalize any LEDs that have dropped out.**

## SinricPro Integration
**I think it's cool to be able to ask Google to turn on my coffee machine, so I added a SinricPro integration. You must add a "credentials.h" file to the src directory with defines for SinricPro's variables: WIFI_SSID, WIFI_PASS, APP_KEY, APP_SECRET, and SWITCH_ID (the latter three values provided by SinricPro for your particular instance).
