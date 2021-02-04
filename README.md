# Microprocessor
These are solutions for all projects of the course Microprocessor in NCTU 107 academic year.
The controller we used is STM32L476RG.

The final project was designed to maintain the temperature of a mother board. A thermometer is used to detect the temperature and then feedback the data to our microprocessor. The motor of the fan would be activated once the temperature higer than threshold. The fan has 6 different speeds, and the speed of fan depends on how high the temperature is. If the temperature is much more higher than threshold, the fan would be turned to a powerful mode, which has a high speed and can cool down the motherboard in a shorter time.

The threshold can be tuned by user. It depends on the temperature that would affect the efficiency of the mother board.
