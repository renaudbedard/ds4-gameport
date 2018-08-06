#include <PS4USB.h>
#include <SPI.h>

USB Usb;
PS4USB PS4(&Usb);

SPISettings spiSettings(10000000, MSBFIRST, SPI_MODE0);

byte xAddress = B00000000;
byte yAddress = B00010000;
int CS = 6;

int lastXWrite = 127;
int lastYWrite = 127;

#define ABUTTON A0
#define BBUTTON A1

// Prints a binary number with leading zeros (Automatic Handling)
// https://forum.arduino.cc/index.php?topic=475435.msg3730840#msg3730840
#define PRINTBIN(Num) for (uint32_t t = (1UL<< (sizeof(Num)*8)-1); t; t >>= 1) Serial.write(Num  & t ? '1' : '0');

int potWrite(byte address, byte value)
{
  bool logValue = false;
  if (address == xAddress && lastXWrite != value) 
  {
    Serial.print(F("X (address "));
    PRINTBIN(address);
    Serial.print(F(") = "));
    Serial.println(value);
    lastXWrite = value;
    logValue = true;
  }
  if (address == yAddress && lastYWrite != value) 
  {
    Serial.print(F("Y (address "));
    PRINTBIN(address);
    Serial.print(F(") = "));
    Serial.println(value);
    lastYWrite = value;
    logValue = true;
  }

  SPI.beginTransaction(spiSettings);
  digitalWrite(CS, LOW);
  SPI.transfer(address);
  SPI.transfer(value);
  digitalWrite(CS, HIGH);
  SPI.endTransaction();

  if (logValue)
  {
    // I expected this to print out the value I just wrote, but it returns all ones?
    SPI.beginTransaction(spiSettings);
    digitalWrite(CS, LOW);
    unsigned int afterWrite = SPI.transfer16(((B00001100 & address) << 8) & 00000000);
    digitalWrite(CS, HIGH);
    SPI.endTransaction();
  
    Serial.print(F("Read result : "));
    PRINTBIN(afterWrite);
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

  // increment wiper on first pot (I don't think this works?)
  /*
  SPI.beginTransaction(spiSettings);
  digitalWrite(CS, LOW);
  byte incrementResult = SPI.transfer(B00000100);
  digitalWrite(CS, HIGH);
  SPI.endTransaction();
  */
}

void loop() 
{
  Usb.Task();

  if (!PS4.connected()) 
  {
    delay(1);
    return;
  }

  int stickX = PS4.getAnalogHat(LeftHatX);
  int stickY = PS4.getAnalogHat(LeftHatY);

  if (stickX > 137 || stickX < 117)
  {
    potWrite(xAddress, (byte)stickX);
  }
  else if (PS4.getButtonPress(LEFT))
    potWrite(xAddress, 0);
  else if (PS4.getButtonPress(RIGHT))
    potWrite(xAddress, 255);
  else
    potWrite(xAddress, 127);

  if (stickY > 137 || stickY < 117) 
  {
    potWrite(yAddress, (byte)stickY);
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
    Serial.println("A");
  } 
  else 
    digitalWrite(ABUTTON, LOW);

  if (PS4.getButtonClick(CROSS)) 
  {
    digitalWrite(BBUTTON, HIGH);
    Serial.println("B");
  } 
  else 
    digitalWrite(BBUTTON, LOW);
}
