# Amplifire

## Introduction

“Amplifire” is to a Fire Performer what an electric amplifier is to a musician.  

One or more propane flame effects are sited within a fire perimeter.  These effects are supervised/armed by fire safeties at the perimeter, and lighting choices indicate the armed/disarmed status of the flame effect, and provide a visual cue on what effect could be triggered (single vs multiple releases, tempo, volume).  The flame effects are triggered by infrared sensors that “look” upwards and above the flame effect and can only “see” objects in the portion of the infrared spectrum that fire tools radiate.  Thus, Fire Performers simply pass their tool above the flame effect to trigger it, and their tools also serve as the ignition source for the effect (although, there will be a secondary pilot present).  Yes, this is an insanely good idea: now a Fire performer with a pair of poi can chuck fireballs as part of their routine.  

Flame Effects feels like it’s split into two camps: propane and handheld tools.  This project seeks to to put (analog) fire performers with hand-held tools in charge of (mechanical) large-scale propane effects.  Both groups benefit: the fire performers add a natural, human element and the propane effects supply a larger scale.  

## Usage

Amplifire serves up it's own WiFi Access Point (AP), with SSID "Amplifire".  The AP is password protected, with the intention of providing only a single client access at at one moment.  This is an important safety feature: Arm and Enable is provided to a Operator at a single point of contact.

Once connected, Amplifire directs the Operator to connect to a web page at http://192.168.1.1/

From there, the Operator can access operator of Amplifire, and provides the following interface:

**Arming**

ARMED/Disarmed.  *arming. Amplifire signals armed status with red color and disarmed status with blue color.*

**Trigger**

Trigger Sensitivity [0-90]: *Amplifire calibrates the IR level after each Operator input. This setting expresses the percentage change from the calibration reading required to trigger the Effect.  e.g. 90% is very sensitive, and 10% is very insensitive.* 

Retrigger After [50-3000] ms: *pause for this amount of time after completing a firing sequence, so that it does not self-trigger.*

**Timing**

Open Time [50-3000] ms: *open the solenoid for this duration every cycle.*

Close Time [50-3000] ms: *close the solenoid for this duration every cycle.*

Number of open-close cycles: *perform this number of open/close cycles, then pause for the retrigger time before firing again.*

## Design

### Control 

![P&ID Diagram](https://github.com/the-magister/afterburn/raw/master/drawings/diagram.png)

### Physical Diagram

![Physical Diagram](https://github.com/the-magister/afterburn/raw/master/drawings/diagram.png)

## Fire Safety Plan

