#include <bb_spi_lcd.h>
#include <VL53L5cx.h>
#include <Wire.h>

#define TFT_CS         4
#define TFT_RST        22
#define TFT_DC         21
#define TFT_LED        26
#define TFT_MOSI       23
#define TFT_SCK        18

SPILCD lcd; // my display library
// Choices are RESOLUTION_4X4 and RESOLUTION_8X8
static VL53L5cx::resolution_t RESOLUTION = VL53L5cx::RESOLUTION_8X8;

//static VL53L5cxAutonomous sensor = VL53L5cxAutonomous(5);
static VL53L5cx sensor = VL53L5cx(5, // LPN pin
                                  0x29, // I^2C address
                                  RESOLUTION,
                                  VL53L5cx::TARGET_ORDER_CLOSEST,
                                  10); // ranging frequency 
// Determine image size from resolution
static const uint8_t SIZE = RESOLUTION == VL53L5cx::RESOLUTION_8X8 ? 8 : 4;

void setup()
{
  // Start I^2C
  Wire.begin(13,14);
  Wire.setClock(400000L);
  delay(100);

  Serial.begin(115200);
  delay(500); // give Serial a little time to start

  spilcdInit(&lcd, LCD_ST7789_135, FLAGS_NONE, 40000000, TFT_CS, TFT_DC, TFT_RST, TFT_LED, -1, TFT_MOSI, TFT_SCK);
  spilcdSetOrientation(&lcd, LCD_ORIENTATION_270);
  spilcdFill(&lcd, 0, DRAW_TO_LCD);
  spilcdWriteString(&lcd, 0,0,(char *)"VL53L5 Range Sensor", 0x7e0, 0, FONT_12x16,DRAW_TO_LCD);

  // Start sensor
  sensor.begin();

}

void loop()
{
char szTemp[32], szLine[128];
int iFont, iHeight;

  if (SIZE == 4) {
    iFont = FONT_12x16;
    iHeight = 16;
  } else {
    iFont = FONT_6x8;
    iHeight = 10;
  }
  
  while (1) {
      // Use polling function to know when a new measurement is ready.
    if (sensor.isReady()) {

// Display dots instead of numbers
#ifdef NUMBERS
        for (uint8_t i=0; i<SIZE; i++) {
            szLine[0] = 0;
            for (uint8_t j=0; j<SIZE; j++) {

                uint16_t d = sensor.getDistance(j*SIZE+i);
                // max depth values are around 2100mm
                sprintf(szTemp, "%04d ", d);
                strcat(szLine, szTemp);
            } // for j
            spilcdWriteString(&lcd, 0,40+i*iHeight,szLine, 0xffff, 0, iFont,DRAW_TO_LCD);
        } // for i
#else
        for (uint8_t i=0; i<SIZE; i++) {
            szLine[0] = 0;
            for (uint8_t j=0; j<SIZE; j++) {
                uint16_t d = sensor.getDistance(j*SIZE+i);
                if (d > 2047) d = 2047; // bound to 11 bits
                d = 31 - (d >>= 6); // get it as a 5-bit grayscale value (near = lighter)
                d = d | (d << 6) | (d << 11); // turn it into RGB565 grayscale 
                spilcdRectangle(&lcd, 80 + j * 10, 32 + i * 10, 8, 8, d, d, 1, DRAW_TO_LCD);
            } // for j
        } // for i
#endif
    } else {
      delay(50); // don't poll the sensor too quickly
    }
  } // while (1)
} /* loop() */
