# Smart Coaster
### Designed by Edward Coyle and Luke Wilkins

## What is it?
This program is to be used alongside the physical smart coaster system for the use in a
restaurant setting. The smart coaster alerts the staff when the drink on it has a liquid
level below a third of the cup size to allow for automatic refill notifications without
having to constantly return to each and every table.

## How does it work?
The drink level is determined by a force sensing resistor underneath the coaster and sending
that value through an MSP430FR2355 microcontroller through its ADC port. The weight values 
are calibrated for each restaurant's cup sizes. 

**There are three threshold values:

thresh_min - The lowest possible value the program will interpret as a "cup," so that the
program doesn't try to change states when nothing is on the coaster.

thresh_max - The weight up the cup at the point right before it is considered empty enough
to need a refill. When the ADC reads a value in between the thresh_min and thresh_max, it
contacts the kitchen for a refill.

big_thresh_max - The highest possible value the program will interpret as a "full cup." Any
higher and it means that it's likely that someone in just pressing down on the coaster. When
the ADC reads a value between the thresh_max and big_thresh_max, it knows that the cup on it
still has plenty liquid inside.

**The program has three states:

The read state - This is the state the program spends the most time in. Every three seconds,
it takes a weight reading of the coaster to determine the drink level. The LED on the coaster
will remain a solid color just to indiciate that there's power. If the ADC reads a value in 
between the thresh_min and thresh_max, it contacts the kitchen for a refill, sending the 
program to the send state.

The send state - This state is simple. The program sends out a signal to the kitchen terminal
showing that a patron needs a refill. Once the kitchen prepares the refill and confirms that
it is on its way, the program moves on to the recieve state. In the send state, the LED blinks
slowly, signifying that the coaster is aware that the patron needs a refill, but that the kitchen
has yet to confirm that they're sending one back.

The recieve state - In this state, the LED will blink quickly to show that the drink is on its way.
This state also records the coaster weight every three seconds, until it detects a value between
the thresh_max and big_thresh_max. Once it does, it moves back to the read state.
