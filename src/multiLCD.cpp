// LCD + PCF8574T I2C extender-näytöille (Pajan DSP-1182)
// Toiminut UNO R3:lla, 4.7k ylösvetovastuksilla, 2 näyttöä rinnan
// Virhetapauksessa blinkkaa LEDiä
//
// hd44780 (bill perry) -kirjaston hd44780_I2Cexp/I2CexpDiag
// Arduino IDE esimerkki voi auttaa virhetapauksissa
//
// ----------------------------------------------------------------------------

#include <Arduino.h>
#include <Wire.h>
#include <hd44780.h>                       // main hd44780 header
#include <hd44780ioClass/hd44780_I2Cexp.h> // i2c expander i/o class header

hd44780_I2Cexp lcd1; // declare lcd object: auto locate & auto config expander chip
hd44780_I2Cexp lcd2;

// If you wish to use an i/o expander at a specific address, you can specify the
// i2c address and let the library auto configure it. If you don't specify
// the address, or use an address of zero, the library will search for the
// i2c address of the device.
// hd44780_I2Cexp lcd(i2c_address); // specify a specific i2c address
//
// It is also possible to create multiple/seperate lcd objects
// and the library can still automatically locate them.
// Example:
// hd4480_I2Cexp lcd1;
// hd4480_I2Cexp lcd2;
// The individual lcds would be referenced as lcd1 and lcd2
// i.e. lcd1.home() or lcd2.clear()
//
// It is also possible to specify the i2c address
// when declaring the lcd object.
// Example:
// hd44780_I2Cexp lcd1(0x20);
// hd44780_I2Cexp lcd2(0x27);
// This ensures that each each lcd object is assigned to a specific
// lcd device rather than letting the library automatically asign it.

// LCD geometry
const int LCD_COLS = 16;
const int LCD_ROWS = 2;

void setup()
{
int status1;
int status2;

	// initialize LCD with number of columns and rows: 
	// hd44780 returns a status from begin() that can be used
	// to determine if initalization failed.
	// the actual status codes are defined in <hd44780.h>
	// See the values RV_XXXX
	//
	// looking at the return status from begin() is optional
	// it is being done here to provide feedback should there be an issue
	//
	// note:
	//	begin() will automatically turn on the backlight
	//
	status1 = lcd1.begin(LCD_COLS, LCD_ROWS);
	status2 = lcd2.begin(LCD_COLS, LCD_ROWS);
	if(status1) hd44780::fatalError(status1);
	if(status2) hd44780::fatalError(status2);

	// initalization was successful, the backlight should be on now
	// Print a message to the LCD
	lcd1.print("Hello,");
	lcd2.print("World");
}

void loop() {}
