#include <PS4USB.h>
#include <SPI.h>

// pins
#define A_BUTTON A0
#define B_BUTTON A1
#define CS 6

#define X_POT 0
#define Y_POT 1

USB Usb;
PS4USB PS4(&Usb);

SPISettings spiSettings(10000000, MSBFIRST, SPI_MODE0);
uint8_t writeCommands[2] = 
{ 
  B00010001, // XXCCXXPP
  B00010010 // where C is the command (01 for write) and P is the pot selector
};
uint8_t lastWrites[2] = { 127, 127 };
char potAxis[2] = { 'X', 'Y' };

void potWrite(byte potIndex, byte value)
{
  bool logValue = false;

  if (lastWrites[potIndex] != value)
  {
    Serial.print(potAxis[potIndex]);
    Serial.print(F(" : "));
    Serial.print(value);
    lastWrites[potIndex] = value;
    logValue = true;
  }

  SPI.beginTransaction(spiSettings);
  digitalWrite(CS, LOW);
  uint16_t fullWrite = writeCommands[potIndex] * 256 + value;
  uint16_t writeResult = SPI.transfer16(fullWrite);
  digitalWrite(CS, HIGH);
  SPI.endTransaction();

  if (logValue)
  {
    Serial.print(F(" | SPI string : "));
    Serial.print(fullWrite, BIN);
    Serial.println();
    
    //Serial.print(F("Write result : "));
    //Serial.print(writeResult, BIN);
    //Serial.println();
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

  pinMode(A_BUTTON, OUTPUT);
  pinMode(B_BUTTON, OUTPUT);
  
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

  if (PS4.getButtonPress(RIGHT))
    potWrite(X_POT, 255);
  else if (PS4.getButtonPress(LEFT))
    potWrite(X_POT, 0);
  else
  {
    uint8_t value = PS4.getAnalogHat(LeftHatX);
    if (value < 122 || value > 132)
      potWrite(X_POT, 255 - value);
    else
      potWrite(X_POT, 127);
  }

  if (PS4.getButtonPress(UP))
    potWrite(Y_POT, 255);
  else if (PS4.getButtonPress(DOWN))
    potWrite(Y_POT, 0);
  else
  {
    uint8_t value = PS4.getAnalogHat(LeftHatY);
    if (value < 122 || value > 132)
      potWrite(Y_POT, 255 - value);
    else
      potWrite(Y_POT, 127);
  }
 
  if (PS4.getButtonClick(CIRCLE) || PS4.getButtonClick(SQUARE)) 
  {
    digitalWrite(A_BUTTON, HIGH);
    Serial.println(F("A"));
  } 
  else 
    digitalWrite(A_BUTTON, LOW);

  if (PS4.getButtonClick(CROSS)) 
  {
    digitalWrite(B_BUTTON, HIGH);
    Serial.println(F("B"));
  } 
  else 
    digitalWrite(B_BUTTON, LOW);
}
