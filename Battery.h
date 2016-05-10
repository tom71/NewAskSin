//- -----------------------------------------------------------------------------------------------------------------------
// AskSin driver implementation
// 2013-08-03 <trilu@gmx.de> Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------
//- AskSin battery status functions ---------------------------------------------------------------------------------------
//- with a lot of support from dirk at FHEM forum
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef _BT_H
#define _BT_H

#include "HAL.h"


class BT {
	friend class AS;

  public:		//---------------------------------------------------------------------------------------------------------
  protected:	//---------------------------------------------------------------------------------------------------------
  private:		//---------------------------------------------------------------------------------------------------------
	
	class AS *pHM;								// pointer to main class for function calls

	uint16_t  checkCentiVolt;					// holds the proof point
	uint16_t  measureCentiVolt;					// variable to hold last measured value
	uint8_t  bState        :1;					// holds status bit
	uint8_t  bMode         :2;					// mode variable
	uint32_t bDuration;							// duration for the next check
	
  public:		//---------------------------------------------------------------------------------------------------------
	BT();
	void	set(uint16_t centiVolt, uint32_t duration);
	uint16_t getVolts(void);
	uint8_t getStatus(void);
		
  protected:	//---------------------------------------------------------------------------------------------------------
  private:		//---------------------------------------------------------------------------------------------------------
	void    init(AS *ptrMain);
	void    poll(void);
};


#endif
