#include <PS4USB.h>
#include <SPI.h>

USB Usb;
PS4USB PS4(&Usb);

SPISettings spiSettings(10000000, MSBFIRST, SPI_MODE0);

uint8_t xAddress = B00000000;
uint8_t yAddress = B00010000;
uint8_t CS = 6;

uint8_t lastXWrite = 127;
uint8_t lastYWrite = 127;

#define ABUTTON A0
#define BBUTTON A1

void potWrite(byte address, byte value)
{
  bool logValue = false;
  if (address == xAddress && lastXWrite != value) 
  {
    Serial.print(F("X (address "));
    Serial.print(address, BIN);
    Serial.print(F(") = "));
    Serial.println(value);
    lastXWrite = value;
    logValue = true;
  }
  if (address == yAddress && lastYWrite != value) 
  {
    Serial.print(F("Y (address "));
    Serial.print(address, BIN);
    Serial.print(F(") = "));
    Serial.println(value);
    lastYWrite = value;
    logValue = true;
  }

  SPI.beginTransaction(spiSettings);
  digitalWrite(CS, LOW);
  uint16_t fullWrite = address * 256 + value;
  uint16_t writeResult = SPI.transfer16(fullWrite);
  digitalWrite(CS, HIGH);
  SPI.endTransaction();

  if (logValue)
  {
    SPI.beginTransaction(spiSettings);
    digitalWrite(CS, LOW);
    uint16_t readResult = SPI.transfer16((B00001100 + address) * 256);
    digitalWrite(CS, HIGH);
    SPI.endTransaction();

    Serial.print(F("Written SPI string : "));
    Serial.print(fullWrite, BIN);
    Serial.println();
    
    Serial.print(F("Write result : "));
    Serial.print(writeResult, BIN);
    Serial.println();
  
    Serial.print(F("Read result : "));
    Serial.print(readResult, BIN);
    Serial.println();
  }
}

void setup() 
{
  Serial.begin(115200);
  if (Usb.Init() == -1) 
  {
    Serial.print(F("\r\nOSC did not start"));
    while (1); // Halt
  }

  pinMode(ABUTTON, OUTPUT);
  pinMode(BBUTTON, OUTPUT);
  pinMode(CS, OUTPUT);
  
  SPI.begin();
}

void loop() 
{
  Usb.Task();

  if (!PS4.connected()) 
  {
    delay(1);
    return;
  }

  uint8_t stickX = PS4.getAnalogHat(LeftHatX);
  uint8_t stickY = PS4.getAnalogHat(LeftHatY);

  if (stickX > 137 || stickX < 117)
  {
    potWrite(xAddress, stickX);
  }
  else if (PS4.getButtonPress(LEFT))
    potWrite(xAddress, 0);
  else if (PS4.getButtonPress(RIGHT))
    potWrite(xAddress, 255);
  else
    potWrite(xAddress, 127);

  if (stickY > 137 || stickY < 117) 
  {
    potWrite(yAddress, stickY);
  }
  else if (PS4.getButtonPress(UP))
    potWrite(yAddress, 255);
  else if (PS4.getButtonPress(DOWN))
    potWrite(yAddress, 0);
  else
    potWrite(yAddress, 127);  

  if (PS4.getButtonClick(CIRCLE) || PS4.getButtonClick(SQUARE)) 
  {
    digitalWrite(ABUTTON, HIGH);
    Serial.println(F("A"));
  } 
  else 
    digitalWrite(ABUTTON, LOW);

  if (PS4.getButtonClick(CROSS)) 
  {
    digitalWrite(BBUTTON, HIGH);
    Serial.println(F("B"));
  } 
  else 
    digitalWrite(BBUTTON, LOW);
}
