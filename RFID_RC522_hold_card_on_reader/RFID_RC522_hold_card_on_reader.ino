/* Typical pin layout used:
   -----------------------------------------------------------------------------------------
               MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
               Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
   Signal      Pin          Pin           Pin       Pin        Pin              Pin
   -----------------------------------------------------------------------------------------
   RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
   SPI SS      SDA(SS)      10            53        D10        10               10
   SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
   SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
   SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15

   More pin layouts for other boards can be found here: https://github.com/miguelbalboa/rfid#pin-layout
*/

#include <SPI.h>
#include <MFRC522.h>
#include  <LiquidCrystal_I2C.h>

int RST_PIN = 9;
int SS_PIN = 10;
int BUZZER = 8;

MFRC522 MFRC522M(SS_PIN, RST_PIN);

LiquidCrystal_I2C LCD(0x3f, 16, 2);

MFRC522::Uid ID;

bool IS_CARD_PRESENT = false;
uint8_t CONTROL = 0x00;
char TMP[16];
String CONTENT = "";

void setup() {
  Serial.begin(9600);

  SPI.begin();
  MFRC522M.PCD_Init();

  pinMode(BUZZER, OUTPUT);

  LCD.begin();
  LCD.backlight();
  LCD.setCursor(0, 0);
  LCD.print("   RFID Card");
  LCD.setCursor(0, 1);
  LCD.print("    Scanner");
  digitalWrite(BUZZER, HIGH);
  delay(2000);
  digitalWrite(BUZZER, LOW);
  LCD.clear();

}

void loop() {
  LCD.setCursor(0, 0);
  LCD.print("Hold Your Card");
  LCD.setCursor(0, 1);
  LCD.print("Over the Reader");

  MFRC522::MIFARE_Key KEY;

  for (byte i = 0; i < 6; i++) {
    KEY.keyByte[i] = 0xFF;
  }

  if ( !MFRC522M.PICC_IsNewCardPresent()) {
    return;
  }
  if ( !MFRC522M.PICC_ReadCardSerial()) {
    return;
  }
  IS_CARD_PRESENT = true;

  cpID(&ID);
  printToLCD();
  Serial.println(CONTENT);
  LCD.clear();

  while (IS_CARD_PRESENT) {
    CONTROL = 0;
    for (int i = 0; i < 3; i++) {
      if (!MFRC522M.PICC_IsNewCardPresent()) {
        if (MFRC522M.PICC_ReadCardSerial()) {
          CONTROL |= 0x16;
        }
        if (MFRC522M.PICC_ReadCardSerial()) {
          CONTROL |= 0x16;
        }
        CONTROL += 0x1;
      }
      CONTROL += 0x4;
    }
    if (CONTROL == 13 || CONTROL == 14) {
      LCD.setCursor(0, 0);
      LCD.print("Card Is Present !");

      digitalWrite(BUZZER, HIGH);

      LCD.setCursor(0, 1);
      LCD.print(CONTENT);

    } else {
      break;
    }
  }
  IS_CARD_PRESENT = false;
  digitalWrite(BUZZER, LOW);
  CONTENT = "";
  LCD.clear();
  LCD.setCursor(0, 0);
  LCD.print("Hold Your Card");
  LCD.setCursor(0, 1);
  LCD.print("Over the Reader");
  Serial.println("Card Removed !");

  delay(500);

  MFRC522M.PICC_HaltA();
  MFRC522M.PCD_StopCrypto1();
}
void printToLCD() {
  for (byte i = 0; i < MFRC522M.uid.size; i++)
  {
    CONTENT.concat(String(MFRC522M.uid.uidByte[i] < 0x10 ? " 0" : " "));
    CONTENT.concat(String(MFRC522M.uid.uidByte[i], HEX));
  }
  CONTENT.toUpperCase();
}

void cpID(MFRC522::Uid *ID) {
  memset(ID, 0, sizeof(MFRC522::Uid));
  memcpy(ID->uidByte, MFRC522M.uid.uidByte, MFRC522M.uid.size);
  ID->size = MFRC522M.uid.size;
  ID->sak = MFRC522M.uid.sak;
}
