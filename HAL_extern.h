/**
*
* @brief Some definitions for the pin change interrupt setup
*
* <pcint_vector_byte> array holds current information per interrupt vector
* <pcint_table> array stores the pin change interrupt specific information and a call back address
* size information a stored in the hardware.h file in user sketch area
*
*/
volatile s_pcint_vector_byte pcint_vector_byte[PCINT_PCIE_SIZE];
#ifdef PCINT_CALLBACK
extern void pci_callback(uint8_t vec, uint8_t pin, uint8_t flag);
#endif



//- cc1100 hardware functions ---------------------------------------------------------------------------------------------
/**
* @brief Initialisze the hardware setup of the cc1101 communication modul. No interrupt is used any more, while 
* detecting a falling edge by constantly checking the GDO0 pin for a falling edge
*/
void    ccInitHw(void) {
	SET_PIN_OUTPUT(CC_CS);																		// set chip select as output
	SET_PIN_OUTPUT(CC_MOSI);																	// set MOSI as output
	SET_PIN_INPUT(CC_MISO);																		// set MISO as input
	SET_PIN_OUTPUT(CC_SCLK);																	// set SCK as output
	SET_PIN_INPUT(CC_GDO0);																		// set GDO0 as input

	SPCR = _BV(SPE) | _BV(MSTR);																// SPI enable, master, speed = CLK/4
}

/**
* @brief With this function we are polling the GDO0 pin to detect a falling edge, while it signals that
* there are some data in the receive buffer
*/
uint8_t ccGetGDO0(void) {
	static uint8_t prev;
	uint8_t curr = GET_PIN_STATUS(CC_GDO0);														// get the current pin status

	if (prev == curr) return 0;																	// if nothing changed since last request then return a 0

	prev = curr;																				// if we are here, a change was detected, so remember for next time
	if (curr) return 0;																			// change was detected, but not a falling edge
	return 1;																					// if we are here, it was a falling edge, so return a 1																	
}

/**
* @brief Simple SPI send function. Byte to transmit is written in the send buffer and processed.
* Wait for transfer finished is done by looping through the SPI register and checking the ready bit
* @param data	Byte to transfer
* @return		If an answer is given, it will be returned
*/
uint8_t ccSendByte(uint8_t data) {
	SPDR = data;																				// send byte
	while (!(SPSR & _BV(SPIF))); 																// wait until transfer finished
	return SPDR;																				// return the data register
}

/**
* @brief Chip select and deselect for the cc1101 module. Select is combined with a wait function
* while we have to wait after a select till the modul is ready for communication
*/
void    ccSelect(void) {
	SET_PIN_LOW(CC_CS);
	while (GET_PIN_STATUS(CC_MISO));
}
void    ccDeselect(void) {
	SET_PIN_HIGH(CC_CS);
}
//- -----------------------------------------------------------------------------------------------------------------------


//- status led related functions -------------------------------------------------------------------------------------------------
/**
* @brief Definition of the blink pattern for the LED module. Reason for defining it here is the 
* limitation of Arduino while #defines from sketch area not seen in library folder
*/
#if   defined(LED_RED) && defined(LED_GRN)														// two leds defined  
//LD ld;				///< status led

const struct s_blPat blPat[] PROGMEM = {														// definition of struct and functionallity can be seen in StatusLed.h
	{ 2, 0, 1, 1,{ 50, 50, }, },					// pairing
	{ 2, 1, 0, 1,{ 200, 0, }, },					// pairing success
	{ 2, 1, 1, 0,{ 200, 0, }, },					// pairing error
	{ 2, 1, 1, 1,{ 5, 1, }, },						// send indicator
	{ 2, 1, 0, 1,{ 5, 1, }, },						// ack indicator
	{ 2, 1, 1, 0,{ 10, 1, }, },						// no ack indicator
	{ 6, 3, 1, 0,{ 50, 10, 10, 10, 10 ,100, }, },	// battery low indicator
	{ 6, 3, 1, 0,{ 10, 10, 10, 10, 10, 100, }, },	// defect indicator
	{ 6, 1, 0, 1,{ 10, 10, 50, 10, 50, 100, }, },	// welcome indicator
	{ 2, 6, 1, 0,{ 20, 20, }, },					// key long indicator
};
#elif defined(LED_RED) || defined(LED_GRN)														// only one led defined
const struct s_blPat blPat[] PROGMEM = {																// definition of struct and functionallity can be seen in StatusLed.h
	{ 2, 0, 1, 0,{ 50, 50, } },						// pairing
	{ 2, 1, 1, 0,{ 200, 0, } },						// pairing success
	{ 2, 3, 1, 0,{ 5, 10, } },						// pairing error
	{ 2, 1, 1, 0,{ 5, 1, } },						// send indicator
	{ 0, 0, 0, 0,{ 0, 0, } },						// ack indicator
	{ 0, 0, 0, 0,{ 0, 0, } },						// no ack indicator
	{ 6, 3, 1, 0,{ 50, 10, 10, 10, 10, 100 } },		// battery low indicator
	{ 6, 3, 1, 0,{ 10, 10, 10, 10, 10, 100 } },		// defect indicator
	{ 6, 1, 1, 0,{ 10, 10, 50, 10, 50, 100 } },		// welcome indicator
	{ 2, 6, 1, 0,{ 20, 20, } },						// key long indicator
};
#else																							// no led defined
const struct s_blPat blPat[1] PROGMEM;															// definition of struct and functionallity can be seen in StatusLed.h
#endif

/**
* @brief Initialize the led pins, but only if there are some defined.
*/
void    initLeds(void) {
	#ifdef LED_RED																				// check if the LED is defined
		SET_PIN_OUTPUT(LED_RED);																// set the led pins in port
		if (LED_ACTIVE_LOW) SET_PIN_HIGH(LED_RED);												// if the led is connected against VCC, set it high, or off
	#endif
	#ifdef LED_GRN																				// check if the LED is defined
		SET_PIN_OUTPUT(LED_GRN);																// set the led pins in port
		if (LED_ACTIVE_LOW) SET_PIN_HIGH(LED_GRN);												// if the led is connected against VCC, set it high, or off
	#endif
}

/**
* @brief Set led status, on or off or toogle, but only if they are defined
* @param  stat   1 for on, 0 for off, any other value to toogle 
*/
void    ledRed(uint8_t stat) {
	#ifdef LED_RED																				// check if the LED is defined
		stat ^= LED_ACTIVE_LOW;																	// xor the stat with LED_ACTIVE_LOW to set the right status if the led is connected against VCC
		if (stat == 1) SET_PIN_HIGH(LED_RED);
		else if (stat == 0) SET_PIN_LOW(LED_RED);
		else                SET_PIN_TOOGLE(LED_RED);
	#endif
}
void    ledGrn(uint8_t stat) {
	#ifdef LED_GRN																				// check if the LED is defined
		stat ^= LED_ACTIVE_LOW;																	// xor the stat with LED_ACTIVE_LOW to set the right status if the led is connected against VCC
		if (stat == 1) SET_PIN_HIGH(LED_GRN);
		else if (stat == 0) SET_PIN_LOW(LED_GRN);
		else                SET_PIN_TOOGLE(LED_GRN);
	#endif
}
//- -----------------------------------------------------------------------------------------------------------------------



//- config key related functions -------------------------------------------------------------------------------------------------
/**
* @brief Initialize the config key pin and register the port for a pin change interrupt.
* Interuptflag will be polled while debouncing is needed.
*/
void    initConfKey(void) {
	registerPCINT(CONFIG_KEY);
}

/**
* Function is called from conbutton class to check the config key status
* No parameter needed while config key is defined in hardware.h
*/
uint8_t checkConfKey(void) {
	return checkPCINT(CONFIG_KEY, 1);															// checks the conf key if there had something happened, debounce set to 1
}															
//- -----------------------------------------------------------------------------------------------------------------------



//- pin change interrupt related functions -------------------------------------------------------------------------------------------------
/**
* @brief Function to register a pin change interrupt. Port is set within this function, but also gets the pin registered in the vector struct
* and pin change registers set automatically.
* @ param    simple use the defined pins from HAL_atmega.h like PIN_B2
*/
void    registerPCINT(uint8_t PINBIT, volatile uint8_t *DDREG, volatile uint8_t *PORTREG, volatile uint8_t *PINREG, uint8_t PCINR, uint8_t PCIBYTE, volatile uint8_t *PCICREG, volatile uint8_t *PCIMASK, uint8_t PCIEREG, uint8_t VEC) {
	_SET_PIN_INPUT(DDREG, PINBIT);																// set the pin as input
	_SET_PIN_HIGH(PORTREG, PINBIT);																// key is connected against ground, set it high to detect changes

	pcint_vector_byte[VEC].PINR =  PINREG;														// set the vector struct
	pcint_vector_byte[VEC].curr |=  _GET_PIN_STATUS(PINREG, PINBIT);
	pcint_vector_byte[VEC].prev  = pcint_vector_byte[VEC].curr;
	pcint_vector_byte[VEC].mask |= _BV(PINBIT);

	_REG_PCI_ICR(PCICREG, PCIEREG);																// make the pci active
	_REG_PCI_PIN(PCIMASK, PCIBYTE);
}

/**
* @brief This function checks is polled while an interrupt pin is registered and here is the handover.
* This functions returns a 3 for a raising edge, 2 for a falling edge, 1 if no interrupt had happend and the pin is high 
* and a 0 if the pin is low.
*
* @param port   The interrupt vector, similar to PCIE0
* @param pin    The Pin we had registered, PORTB2 or PCINT17
* @param debounce: when true (1) then wait DEBOUNCE time before returning new status
*/
uint8_t checkPCINT(uint8_t PINBIT, volatile uint8_t *DDREG, volatile uint8_t *PORTREG, volatile uint8_t *PINREG, uint8_t PCINR, uint8_t PCIBYTE, volatile uint8_t *PCICREG, volatile uint8_t *PCIMASK, uint8_t PCIEREG, uint8_t VEC, uint8_t debounce) {
	return checkPCINT(VEC, PINBIT, debounce);
}
uint8_t checkPCINT(uint8_t port, uint8_t pin, uint8_t debounce) {

	uint8_t status = pcint_vector_byte[port].curr & _BV(pin) ? 1 : 0;							// evaluate the pin status
	uint8_t prev = pcint_vector_byte[port].prev & _BV(pin) ? 1 : 0;								// evaluate the previous pin status

	if (status == prev) return status;															// check if something had changed since last time
	if (debounce && ((getMillis() - pcint_vector_byte[port].time) < DEBOUNCE)) return status;	// seems there is a change, check if debounce is necassary and done
	
	pcint_vector_byte[port].prev ^= _BV(pin);													// if we are here, there was a change and debounce check was passed, remember for next time

	if (status) return 3;																		// pin is 1, old was 0
	else return 2;																				// pin is 0, old was 1
}

/**
* @brief This function is called from the different interrupt vector functions while an interrupt 
* had happened. Within this function we check also if there is a callback for an interrupt registered.
*
* @param vec    Is the vector byte, were the interrupt comes from
*/
void    maintainPCINT(uint8_t vec) {
	pcint_vector_byte[vec].curr = *pcint_vector_byte[vec].PINR  & pcint_vector_byte[vec].mask;	// read the pin port and mask out only pins registered
	pcint_vector_byte[vec].time = getMillis();													// store the time, if debounce is asked for

	#ifdef PCINT_CALLBACK																		// callback only needed if defined in hardware.h
	uint8_t bInt = pcint_vector_byte[vec].curr ^ pcint_vector_byte[vec].prev;					// evaluate the pin which raised the interrupt
	pci_callback(vec, bInt, pcint_vector_byte[vec].curr & bInt);								// callback the interrupt function in user sketch
	#endif
}
//- -----------------------------------------------------------------------------------------------------------------------



/*************************************
*** Battery measurement functions ***
*************************************/
void    initExtBattMeasurement(void);
void    switchExtBattMeasurement(uint8_t stat);

/**
* get the voltage off battery
*/
uint8_t  getBatteryVoltage(void) {
#if defined EXT_BATTERY_MEASUREMENT
	initExtBattMeasurement();
	switchExtBattMeasurement(1);
#endif

#if defined EXT_BATTERY_MEASUREMENT
	uint16_t adcValue = getAdcValue(										// Voltage Reference = Internal 1.1V; Input Channel = external battery measure pin
		(1 << REFS1) | (1 << REFS0) | EX(_PIN, BATT_MEASURE)
		);

	adcValue = adcValue * AVR_BANDGAP_VOLTAGE / 1023 / BATTERY_FACTOR;		// calculate battery voltage in V/10
	switchExtBattMeasurement(0);
#else
	uint16_t adcValue = getAdcValue(										// Voltage Reference = AVCC with external capacitor at AREF pin; Input Channel = 1.1V (V BG)
		(0 << REFS1) | (1 << REFS0) | (1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (0 << MUX0)
		);

	adcValue = AVR_BANDGAP_VOLTAGE * 1023 / adcValue / 100;					// calculate battery voltage in V/10
#endif

	return (uint8_t)adcValue;
}

/**
* Initialize battery measurement pin for external battery measurement
*/
void    initExtBattMeasurement(void) {
	SET_PIN_INPUT(BATT_MEASURE);						// set the ADC pin as input
	SET_PIN_INPUT(BATT_ENABLE);						// set the measurement enable pin as input, otherwise we waste energy over the resistor network against VCC
}

/**
* activate / deactivate battery measurement pin
* @param	stat	1: activate, 0: deactivate
*/
void    switchExtBattMeasurement(uint8_t stat) {
	if (stat == 1) {
		SET_PIN_OUTPUT(BATT_ENABLE);					// set pin as out put
		SET_PIN_LOW(BATT_ENABLE);						// set low to measure the resistor network
		SET_PIN_LOW(BATT_MEASURE);
	}
	else {
		SET_PIN_INPUT(BATT_ENABLE);

		// todo: check
		SET_PIN_HIGH(BATT_MEASURE);					// switch on pull up, otherwise we waste energy over the resistor network against VCC
	}
}
//- -----------------------------------------------------------------------------------------------------------------------



//- -----------------------------------------------------------------------------------------------------------------------
/**
* @brief Pin Change Interrupt catch functions
*
*/
ISR(PCINT0_vect) {
	maintainPCINT(0);
	//dbg << "i2:" << PINB << "\n";
}
#if PCINT_PCIE_SIZE > 1
ISR(PCINT1_vect) {
	maintainPCINT(1);
	//dbg << "i2:" << PINC << "\n";
}
#endif
#if PCINT_PCIE_SIZE > 2
ISR(PCINT2_vect) {
	maintainPCINT(2);
	//dbg << "i3:" << PIND  << "\n";
}
#endif
#if PCINT_PCIE_SIZE > 3
ISR(PCINT3_vect) {
	maintainPCINT(3);
	//dbg << "i3:" << PINE  << "\n";
}
#endif
//- -----------------------------------------------------------------------------------------------------------------------
