# Smart Pill Dispenser

![Project Banner](docs/images/circuit-diagram.png)

An Arduino-based smart pill dispenser that automatically dispenses medication at scheduled times and reminds patients to take their pills.

## Features

- Automated pill dispensing at scheduled times
- Visual and audible reminders (LED + buzzer)
- 5-minute window to take medication before automatic retrieval
- Simple one-button interface
- Simulated clock for easy testing (1 second = 1 minute)

## Hardware Requirements

- Arduino Uno
- Servo motor (SG90)
- Piezo buzzer
- LED with resistor
- Push button
- LCD I2C display (16x2)
- Breadboard and jumper wires

## Installation

1. Clone this repository:
   ```bash
   git clone https://github.com/yourusername/smart-pill-dispenser.git