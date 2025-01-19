#include "Adafruit_TinyUSB.h"
#include "Wire.h"

////////////////////////////////////////
// Various config options

// This is the frequency at which the loop will run.
const int loop_Hz = 2;

///// Uncomment the following to exercise various hardware

// Serial output over USB
// #define SERIAL_DEBUG

// Piezo speaker
#define TEST_PIEZO

// NeoPixel
#define TEST_NEOPIXEL

// i2c display (SSD1327, since that's what I have lying around right now)
#define TEST_I2C_SSD1327

//
//////////////////////////////////////////////

#if defined(TEST_NEOPIXEL)
  #include <Adafruit_NeoPixel.h>
  Adafruit_NeoPixel pixel(1, PIN_NEOPIXEL);
#endif

#if defined(TEST_I2C_SSD1327)
  #define TEST_DISPLAY_GRAY4 1
  #define TEST_DISPLAY_I2C 1
  #include <Adafruit_SSD1327.h>

  Adafruit_SSD1327 display(128, 128, &Wire, -1, 1000000UL);
  const int display_address = SSD1327_I2C_ADDRESS;
  const uint16_t text_color = SSD1327_WHITE;
  const uint16_t text_bg = SSD1327_BLACK;

  bool display_ready = false;

  void reset_display()
  {
    display.clearDisplay();
    display.fillScreen(text_bg);
    display.setTextSize(1);
    display.setTextWrap(true);
    display.setTextColor(text_color, text_bg);
    display.setCursor(0,0);

    // Leave room for live updates
    display.print("\n\n\n\n");
    display.print(F("It's alive!"));
    display.setContrast(0x7f);
    // display.setContrast(0x1f);
  }

  bool i2c_probe_bus()
  {
    // Check to see whether it looks like there's anything connected to the SDA/SCL pins.
    // This is needed because calling Wire.begin() without anything conneted seems to break things.
    // Specifically, it returns false positives and sometimes hangs the device.

    // If i2c is connected, both pins should have pull-up resistors, which means they will read high.
    pinMode(PIN_WIRE0_SDA, INPUT);
    pinMode(PIN_WIRE0_SCL, INPUT);
    return (digitalRead(PIN_WIRE0_SDA) == HIGH) && (digitalRead(PIN_WIRE0_SCL) == HIGH);
  }

  bool i2c_probe_device(byte address)
  {
    // Probe the specified address to see if a device seems to be present.
    Wire.beginTransmission(address);
    return (Wire.endTransmission() == 0);
  }

#endif

// HID report descriptor using TinyUSB's template
// Single Report (no ID) descriptor
uint8_t const desc_hid_report[] =
{
  TUD_HID_REPORT_DESC_MOUSE()
};

// USB HID object
Adafruit_USBD_HID usb_hid;

class DebugLogger: public Print
{
public:
	// Print subclass methods
    virtual size_t write(uint8_t);
    virtual size_t write(const uint8_t *buffer, size_t size);

	// Other stuff
	// Returns true iff the log is connected to something.
	// Can be used to skip expensive operations that are only done to generate logs.
	bool enabled();
};

DebugLogger debugLogger;

size_t DebugLogger::write(uint8_t c)
{
  size_t result = 0;
#if defined(SERIAL_DEBUG)
  if (tud_cdc_connected())
  {
    result = Serial.write(c);
    yield();
  }
#endif

  return result;
}
size_t DebugLogger::write(const uint8_t *buffer, size_t size)
{
  size_t result = 0;
#if defined(SERIAL_DEBUG)
  if (tud_cdc_connected())
  {
    result = Serial.write(buffer, size);
    yield();
  }
#endif

  return result;
}

bool DebugLogger::enabled()
{
  bool result = false;
#if defined(SERIAL_DEBUG)
  if (tud_cdc_connected())
  {
    result = true;
  }
#endif
  return result;
}

void click()
{
#if !defined(PIEZO_FREQUENCY)
  #define PIEZO_FREQUENCY 5000
#endif

#if !defined(PIEZO_DURATION)
  #define PIEZO_DURATION 5
#endif

#ifdef PIN_PIEZO
  pinMode(PIN_PIEZO, OUTPUT);
  // This is MUCH louder than just toggling the high/low once with digitalWrite().
  tone(PIN_PIEZO, PIEZO_FREQUENCY, PIEZO_DURATION);
#endif
}

void setup() 
{
  // TinyUSB Setup
  USBDevice.setProductDescriptor("rp2040_mwtrackball");
  usb_hid.setStringDescriptor("rp2040_mwtrackball");
  usb_hid.setPollInterval(2);
  usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));

  usb_hid.begin();

  // wait until device mounted
  while( !USBDevice.mounted() ) delay(1);

#if defined(SERIAL_DEBUG)
  Serial.begin(115200);

  // Wait for the serial port to be opened.
  // NOTE: this will block forever, which can be inconvenient if you're trying to use the device.
  while (!Serial);

  // alternate: add a short delay so I can get the console open before things start happening.
  // delay(2000);

  debugLogger.println(F("Opened serial port"));
#endif

#if defined(TEST_NEOPIXEL)
  pixel.begin();  // initialize the pixel
#endif

#if defined(TEST_I2C_SSD1327)
  if (!i2c_probe_bus())
  {
    debugLogger.println(F("i2c bus appears disconnected"));
  }
  else
  {
    debugLogger.println(F("i2c bus seems sane"));
    Wire.begin();
    if (!i2c_probe_device(display_address))
    {
      debugLogger.println(F("Display not found"));
    }
    else
    {
      debugLogger.println(F("Display found"));
      // Something responded at the correct address. Assume it's the display.
      if (!display.begin(display_address)) 
      {
        debugLogger.println(F("Display init failed"));
      }
      else
      {
        display_ready = true;
        debugLogger.println(F("Display initialized"));

        reset_display();
      }
    }
  }
#endif

  debugLogger.println(F("Initialization complete."));

}

const int loop_microseconds = 1000000 / loop_Hz;

void loop() 
{
  unsigned long loop_start_time = micros();
  // Time taken by the last loop, saved so we can display it during the next loop.
  static unsigned long loop_time = 0;

  debugLogger.println("Hello, world!");

#if defined(TEST_PIEZO)
  // Play a click for each loop
  click();
#endif

  static int pixel_color = 0;
  const int pixel_color_scale = 16;
  int red = (pixel_color & 0x03) * pixel_color_scale;
  int green = ((pixel_color >> 2) & 0x03) * pixel_color_scale;
  int blue = ((pixel_color >> 4) & 0x03) * pixel_color_scale;

#if defined(TEST_NEOPIXEL)
  pixel.setPixelColor(0, red, green, blue);
  pixel.show();
#endif

#if defined(TEST_I2C_SSD1327)
    if (display_ready)
    {
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(0,0);

      display.print(F("R = "));
      display.print(red);
      display.print(F("\nG = "));
      display.print(green);
      display.print(F("\nB = "));
      display.print(blue);

      display.display();
    }
#endif

  pixel_color++;
  pixel_color &= 0x3F;

  // Delay to keep the loop time right around loop_microseconds
  loop_time = micros() - loop_start_time;
  if (loop_time < loop_microseconds)
  {
    delayMicroseconds(loop_microseconds - loop_time);
  }
}


