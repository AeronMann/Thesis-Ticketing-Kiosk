#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,20,4);
#include <Servo.h>
Servo myservo;
int pos = 200;

/// RFID Initialization ///
#define RST_PIN 5
#define SS_PIN 53
int state = 0;
String card;
MFRC522 mfrc522(SS_PIN, RST_PIN);

/// CARD VALUE ///
int card1Balance = 100;
int card2Balance = 200;
int card3Balance = 300;
int card4Balance = 500;
int card5Balance = 1000;

///  Buttons ///
#define ticketingBtn  43 //1
#define refillingBtn  24 //2
#define chargingBtn  26 //3
#define dispensingBtn  28 //4
#define rechargeBtn  30 //5
#define rfidPaymentBtn  32 //6
#define billPaymentBtn  34 //7
#define resetBtn 36 //8

/// IR SENSOR
#define irSensor 35

/// RelayPins ///
#define relayPinCharge 10
#define relayPinRefill 11
#define relayPinDispensing 12

/// Function State ///
int ticketingState = 0; //done
int refillingState = 0; //done
int chargingState = 0; //done
int dispensingState = 0; //done
int rechargeState = 0; // done
int rfidPaymentState = 0;
int billPaymentState = 0;

int startRefillState = 0;
int startRechargeState = 0;
int dispensingRfidPaymentState = 0;
int dispensingBillPaymentState = 0;

/// Bill Acceptor ///
volatile unsigned int credit = 0;
volatile unsigned long bouncetime = 0;
#define BILL 2
#define BOUNCE_DURATIONBILL 25

/// Recharge RFID ///
String rechargeRfidContent = "";
int whatCard = 0;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  ///LCD
  lcd.init();
  lcd.backlight();
  ///Servo
  myservo.attach(27);
  myservo.write(pos);
  /// IR
  pinMode(irSensor,INPUT);
  
  /// pin mode Button ///
  pinMode(ticketingBtn, INPUT);
  pinMode(refillingBtn, INPUT);
  pinMode(chargingBtn, INPUT);
  pinMode(dispensingBtn, INPUT);
  pinMode(rechargeBtn, INPUT);
  pinMode(rfidPaymentBtn, INPUT);
  pinMode(billPaymentBtn, INPUT);
  pinMode(resetBtn, INPUT);

  ////BILL ACCEPTOR///
  pinMode(BILL, INPUT_PULLUP);
  attachInterrupt (digitalPinToInterrupt(BILL), ISR_count, FALLING);

  ////RELAY////
  pinMode(relayPinCharge, OUTPUT);
  pinMode(relayPinDispensing, OUTPUT); 
  pinMode(relayPinRefill, OUTPUT);
  digitalWrite(relayPinCharge,HIGH);
  digitalWrite(relayPinDispensing,HIGH);
  digitalWrite(relayPinRefill,HIGH);

}

void loop() {
  ////////////// TICKETING ////////////////
  if (ticketingState == 1){
    Serial.println("ticketing");
    ticketing();
    }
  else if (digitalRead(ticketingBtn)==0){
    ticketingState = 1;
    delay(200);
    }
  else if (rfidPaymentState == 1){
    rfidPayment();
    }
  else if (billPaymentState == 1){
    billPaymentTicketing();
    }

  /////////////// REFILLING ////////////////
  else if (refillingState == 1){
    Serial.println("refilling");
    refilling();
    }
  else if (digitalRead(refillingBtn) == LOW){
    refillingState = 1;
    }
  else if (startRefillState == 1){
    startRefill();
    }

  ///////////// CHARGING USING RFID //////////////
  else if (chargingState == 1){
    Serial.println("charge device");
    charging();
    }
  else if (digitalRead(chargingBtn) == LOW){
    chargingState = 1;
    }

  ///////////// DISPENSING /////////////
  else if (dispensingState == 1){
    Serial.println("ALCOHOL DISPENSING");
    dispensing();
    }
  else if (digitalRead(dispensingBtn) == LOW){
    dispensingState = 1;
    }
  else if (dispensingRfidPaymentState == 1){
    dispensingRfidPayment();
    }
  else if (dispensingBillPaymentState == 1){
    dispensingBillPayment();
    }
    
  ///////////// RFID RECHARGE /////////////
  else if (rechargeState == 1){
    Serial.println("recharge RFID");
    recharge();
    }
  else if (digitalRead(rechargeBtn) == LOW){
    rechargeState = 1;
    }
  else if (startRechargeState == 1){
    startRecharge();
    }

  //////////// STANDBY MODE ///////////////
  else{
    credit = 0;
    digitalWrite(relayPinCharge, HIGH);
    digitalWrite(relayPinDispensing, HIGH); 
    digitalWrite(relayPinRefill, HIGH);
    standbyMode();
    }
}

///////////////////////////////////////// STANDBY MODE ////////////////////////////////
void standbyMode(){
  lcd.clear();
  lcd.setCursor(1,1);
  lcd.print("Hello, Welcome to");
  lcd.setCursor(1,2);
  lcd.print("Our Vending Kiosk");
  delay(1500);
  lcd.clear();
  lcd.setCursor(1,1);
  lcd.print("Please Choose Our");
  lcd.setCursor(6,2);
  lcd.print("Service");
  delay(1500);
}

////////////////////////////////// BILL ACCEPTOR FUCNTION //////////////////////
void ISR_count() {
  if (millis() > bouncetime) {
    if (!digitalRead (BILL)) {
      credit += 10;
      Serial.print("credit: ");
      Serial.println(credit); 
    }
    bouncetime = millis() + BOUNCE_DURATIONBILL;
  }
}

///////////////////////////////////////// TICKETING ////////////////////////////////
void ticketing(){
  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print("Ticketing System");
  lcd.setCursor(0,1);
  lcd.print("Choose Your Payment");
  lcd.setCursor(0,2);
  lcd.print("RFID ==> Button 6");
  lcd.setCursor(0,3);
  lcd.print("CASH ==> Button 7");
  delay(1000);
  ////////// RFID PAYMENT ////////////
  if (digitalRead(rfidPaymentBtn)==LOW){
    rfidPaymentState = 1;
    ticketingState = 0;
    }
  ////////// BILL PAYMENT ////////////
  else if (digitalRead(billPaymentBtn)==LOW){
    billPaymentState = 1;
    ticketingState = 0;
    }

  else if (digitalRead(resetBtn)==LOW){
    ticketingState = 0; 
    refillingState = 0; 
    chargingState = 0; 
    dispensingState = 0; 
    rechargeState = 0; 
    rfidPaymentState = 0;
    billPaymentState = 0;
    lcd.clear();
    lcd.setCursor(2,1);
    lcd.print("Processing Reset");
    lcd.setCursor(3,2);
    lcd.print("Please Wait...");
    delay(2000);
    }
}

void billPaymentTicketing(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Insert 50 Peso Bill");
  lcd.setCursor(1,1);
  lcd.print("<=====<=====<=====");
  delay(300);
  Serial.println(credit);
  
  if (credit == 50){
      ///////// DROPPING TICKET CODE //////////////
    lcd.clear();
    lcd.setCursor(5,1);
    lcd.print("Processing");
    lcd.setCursor(3,2);
    lcd.print("Please Wait...");
    /// SERVO PROCESS
    for (pos = 0; pos <= 200; pos +=4) { 
      myservo.write(pos);
      delay(10);}
    for (pos = 0; pos >= 200; pos -=4) {
      myservo.write(pos);
      delay(10);}
      billPaymentState = 0;
      ticketingState = 0;
      credit = 0;
      lcd.clear();
      lcd.setCursor(8,0);
      lcd.print("Done");
      delay(3000);
      }
  if (credit == 100){
      ///////// DROPPING TICKET CODE //////////////
    lcd.clear();
    lcd.setCursor(5,1);
    lcd.print("Processing");
    lcd.setCursor(3,2);
    lcd.print("Please Wait...");
    /// SERVO PROCESS
    for (pos = 0; pos <= 200; pos +=4) { 
      myservo.write(pos);
      delay(10);}
    for (pos = 0; pos >= 200; pos -=4) {
      myservo.write(pos);
      delay(10);}
    delay(1000);
    for (pos = 0; pos <= 200; pos +=4) { 
      myservo.write(pos);
      delay(10);}
    for (pos = 0; pos >= 200; pos -=4) {
      myservo.write(pos);
      delay(10);}
    billPaymentState = 0;
    ticketingState = 0;
    credit = 0;
    lcd.clear();
    lcd.setCursor(8,0);
    lcd.print("Done");
    delay(3000);
      }
  if (digitalRead(resetBtn)==LOW){
    ticketingState = 0; 
    refillingState = 0; 
    chargingState = 0; 
    dispensingState = 0; 
    rechargeState = 0; 
    rfidPaymentState = 0;
    billPaymentState = 0;
    lcd.clear();
    lcd.setCursor(2,1);
    lcd.print("Processing Reset");
    lcd.setCursor(3,2);
    lcd.print("Please Wait...");
    delay(2000);
    }
  }
void rfidPayment(){
  while(rfidPaymentState == 1){
    lcd.clear();
    lcd.setCursor(0,1);
    lcd.print("Scan your RFID here");
    lcd.setCursor(1,2);
    lcd.print("=====>=====>=====>");
    delay(500);
    if ( ! mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }
  if ( ! mfrc522.PICC_ReadCardSerial())
  {
    return;
  }

  String content = "";
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();
  /// ONE ///
  if (content.substring(1) == "EB 7F BD 28")
  {
    if (card1Balance >= 50)
    {
      lcd.clear();
      lcd.setCursor(5,1);
      lcd.print("Processing");
      lcd.setCursor(3,2);
      lcd.print("Please Wait...");
      card1Balance = card1Balance - 50;
      /// SERVO PROCESS
      for (pos = 0; pos <= 200; pos +=4) { 
        myservo.write(pos);
        delay(10);}
      for (pos = 0; pos >= 200; pos -=4) {
        myservo.write(pos);
        delay(10);}
      lcd.clear();
      lcd.setCursor(2,0);
      lcd.print("----Payment----");
      lcd.setCursor(6,1);
      lcd.print("PHP -50");
      lcd.setCursor(0,2);
      lcd.print("Your Card Balance is");
      lcd.setCursor(5,3);
      lcd.print("PHP");
      lcd.setCursor(8,3);
      lcd.print(card1Balance);
      Serial.println(card1Balance);
      delay(4000);
      state = 1;
      rfidPaymentState  = 0;
      ticketingState = 0;
    }
    else
    {
      card = content.substring(1);
      lcd.clear();
      lcd.setCursor(4,0);
      lcd.print("Insufficient");
      lcd.setCursor(6,1);
      lcd.print("Balance");
      lcd.setCursor(5,2);
      lcd.print("PHP");
      lcd.setCursor(9,2);
      lcd.print(card1Balance);
      lcd.setCursor(2,3);
      lcd.print("Please Recharge!");
      Serial.println(card1Balance);
      Serial.println("Please Recharge");
      delay(2000);
      state = 0;
      ticketingState = 0;
      rfidPaymentState  = 0;
    }
  }
  /// TWO ///
  else if (content.substring(1) == "A3 B3 D1 18")
  {
    if (card2Balance >= 50)
    {
      lcd.clear();
      lcd.setCursor(5,1);
      lcd.print("Processing");
      lcd.setCursor(3,2);
      lcd.print("Please Wait...");
      card2Balance = card2Balance - 50;
      /// SERVO PROCESS
      for (pos = 0; pos <= 200; pos +=4) { 
        myservo.write(pos);
        delay(10);}
      for (pos = 0; pos >= 200; pos -=4) {
        myservo.write(pos);
        delay(10);}
      lcd.clear();
      lcd.setCursor(2,0);
      lcd.print("----Payment----");
      lcd.setCursor(6,1);
      lcd.print("PHP -50");
      lcd.setCursor(0,2);
      lcd.print("Your Card Balance is");
      lcd.setCursor(5,3);
      lcd.print("PHP");
      lcd.setCursor(8,3);
      lcd.print(card2Balance);
      Serial.println(card2Balance);
      delay(4000);
      state = 1;
      rfidPaymentState  = 0;
      ticketingState = 0;
    }
    else
    {
      card = content.substring(1);
      delay(2000);
      lcd.setCursor(4,0);
      lcd.print("Insufficient");
      lcd.setCursor(6,1);
      lcd.print("Balance");
      lcd.setCursor(5,2);
      lcd.print("PHP");
      lcd.setCursor(9,2);
      lcd.print(card2Balance);
      lcd.setCursor(2,3);
      lcd.print("Please Recharge!");
      Serial.println(card2Balance);
      Serial.println("Please Recharge");
      delay(1000);
      state = 0;
      ticketingState = 0;
      rfidPaymentState  = 0;
    }
  }
  /// THREE ///
  else if (content.substring(1) == "0B 3D C1 28")
  {
    if (card3Balance >= 50)
    {
      lcd.clear();
      lcd.setCursor(5,1);
      lcd.print("Processing");
      lcd.setCursor(3,2);
      lcd.print("Please Wait...");
      card3Balance = card3Balance - 50;
      /// SERVO PROCESS
      for (pos = 0; pos <= 200; pos +=4) { 
        myservo.write(pos);
        delay(10);}
      for (pos = 0; pos >= 200; pos -=4) {
        myservo.write(pos);
        delay(10);}
      lcd.clear();
      lcd.setCursor(2,0);
      lcd.print("----Payment----");
      lcd.setCursor(6,1);
      lcd.print("PHP -50");
      lcd.setCursor(0,2);
      lcd.print("Your Card Balance is");
      lcd.setCursor(5,3);
      lcd.print("PHP");
      lcd.setCursor(8,3);
      lcd.print(card3Balance);
      Serial.println(card3Balance);
      delay(4000);
      state = 1;
      rfidPaymentState  = 0;
      ticketingState = 0;
    }
    else
    {
      card = content.substring(1);
      delay(2000);
      lcd.setCursor(4,0);
      lcd.print("Insufficient");
      lcd.setCursor(6,1);
      lcd.print("Balance");
      lcd.setCursor(5,2);
      lcd.print("PHP");
      lcd.setCursor(9,2);
      lcd.print(card3Balance);
      lcd.setCursor(2,3);
      lcd.print("Please Recharge!");
      Serial.println(card3Balance);
      Serial.println("Please Recharge");
      delay(1000);
      state = 0;
      ticketingState = 0;
      rfidPaymentState  = 0;
    }
  }
  /// FOUR ///
  else if (content.substring(1) == "3B 04 AE 28")
  {
    if (card4Balance >= 50)
    {
      lcd.clear();
      lcd.setCursor(5,1);
      lcd.print("Processing");
      lcd.setCursor(3,2);
      lcd.print("Please Wait...");
      card4Balance = card4Balance - 50;
      /// SERVO PROCESS
      for (pos = 0; pos <= 200; pos +=4) { 
        myservo.write(pos);
        delay(10);}
      for (pos = 0; pos >= 200; pos -=4) {
        myservo.write(pos);
        delay(10);}
      lcd.clear();
      lcd.setCursor(2,0);
      lcd.print("----Payment----");
      lcd.setCursor(6,1);
      lcd.print("PHP -50");
      lcd.setCursor(0,2);
      lcd.print("Your Card Balance is");
      lcd.setCursor(5,3);
      lcd.print("PHP");
      lcd.setCursor(8,3);
      lcd.print(card4Balance);
      Serial.println(card4Balance);
      delay(4000);
      state = 1;
      rfidPaymentState  = 0;
      ticketingState = 0;
    }
    else
    {
      card = content.substring(1);
      delay(2000);
      lcd.setCursor(4,0);
      lcd.print("Insufficient");
      lcd.setCursor(6,1);
      lcd.print("Balance");
      lcd.setCursor(5,2);
      lcd.print("PHP");
      lcd.setCursor(9,2);
      lcd.print(card4Balance);
      lcd.setCursor(2,3);
      lcd.print("Please Recharge!");
      Serial.println(card4Balance);
      Serial.println("Please Recharge");
      delay(1000);
      state = 0;
      ticketingState = 0;
      rfidPaymentState  = 0;
    }
  }
  /// FIVE ///
  else if (content.substring(1) == "9B 01 BD 28")
  {
    if (card5Balance >= 50)
    {
      lcd.clear();
      lcd.setCursor(5,1);
      lcd.print("Processing");
      lcd.setCursor(3,2);
      lcd.print("Please Wait...");
      card5Balance = card5Balance - 50;
      /// SERVO PROCESS
      for (pos = 0; pos <= 200; pos +=4) { 
        myservo.write(pos);
        delay(10);}
      for (pos = 0; pos >= 200; pos -=4) {
        myservo.write(pos);
        delay(10);}
      lcd.clear();
      lcd.setCursor(2,0);
      lcd.print("----Payment----");
      lcd.setCursor(6,1);
      lcd.print("PHP -50");
      lcd.setCursor(0,2);
      lcd.print("Your Card Balance is");
      lcd.setCursor(5,3);
      lcd.print("PHP");
      lcd.setCursor(8,3);
      lcd.print(card5Balance);
      Serial.println(card5Balance);
      delay(4000);
      state = 1;
      rfidPaymentState  = 0;
      ticketingState = 0;
    }
    else
    {
      card = content.substring(1);
      delay(2000);
      lcd.setCursor(4,0);
      lcd.print("Insufficient");
      lcd.setCursor(6,1);
      lcd.print("Balance");
      lcd.setCursor(5,2);
      lcd.print("PHP");
      lcd.setCursor(9,2);
      lcd.print(card5Balance);
      lcd.setCursor(2,3);
      lcd.print("Please Recharge!");
      Serial.println(card5Balance);
      Serial.println("Please Recharge");
      delay(1000);
      state = 0;
      ticketingState = 0;
      rfidPaymentState  = 0;
    }
  }
  

  else if (digitalRead(resetBtn) == LOW){
    ticketingState = 0; 
    refillingState = 0; 
    chargingState = 0; 
    dispensingState = 0; 
    rechargeState = 0; 
    rfidPaymentState = 0;
    billPaymentState = 0;
    startRefillState = 0;
    lcd.clear();
    lcd.setCursor(2,1);
    lcd.print("Processing Reset");
    lcd.setCursor(3,2);
    lcd.print("Please Wait...");
    delay(2000);
    
    } 

  else   {
    lcd.clear();
    lcd.setCursor(3,0);
    lcd.print("Access Denied");
    lcd.setCursor(0,1);
    lcd.print("--------------------");
    lcd.setCursor(1,2);
    lcd.print("Unregistered RFID");
    Serial.println("Access denied");
    rfidPaymentState  = 0;
    ticketingState = 0;
    delay(2000);
    }
  }
}

////////////////////////////////// CHARGING FUNCTION //////////////////////////////
void charging(){
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("-Charging Station-");
  lcd.setCursor(2,2);
  lcd.print("Scan Your RFID");
  lcd.setCursor(1,3);
  lcd.print("=====>=====>=====>");
  delay(1000);
  if (digitalRead(resetBtn)== LOW){
    ticketingState = 0; 
    refillingState = 0; 
    chargingState = 0; 
    dispensingState = 0; 
    rechargeState = 0; 
    rfidPaymentState = 0;
    billPaymentState = 0;
    lcd.clear();
    lcd.setCursor(2,1);
    lcd.print("Processing Reset");
    lcd.setCursor(3,2);
    lcd.print("Please Wait...");
    delay(2000);
   }
  if ( ! mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }
  if ( ! mfrc522.PICC_ReadCardSerial())
  {
    return;
  }

  String content = "";
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();

  /// ONE ///
  if (content.substring(1) == "EB 7F BD 28")
  {
    if (card1Balance >= 5)
    {
      card1Balance = card1Balance - 5;
      Serial.println(card1Balance);
      Serial.println("Connect your device");
      lcd.clear();
      lcd.setCursor(3,0);
      lcd.print("Connect Device");
      lcd.setCursor(0,1);
      lcd.print("--------------------");
      lcd.setCursor(2,2);
      lcd.print("Current Balance:");
      lcd.setCursor(7,3);
      lcd.print(card1Balance);
      digitalWrite(relayPinCharge, LOW);
      delay(500);
      digitalWrite(relayPinCharge, HIGH);
      delay(2000);
      chargingState  = 0;
    }
    else
    {
      card = content.substring(1);
      Serial.println(card1Balance);
      Serial.println("Please Recharge");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Insufficient Balance");
      lcd.setCursor(2,1);
      lcd.print("Please Recharge");
      lcd.setCursor(0,2);
      lcd.print("--------------------");
      lcd.setCursor(0,3);
      lcd.print("Card Balance:");
      lcd.setCursor(14,3);
      lcd.print(card1Balance);
      delay(2000);
      chargingState  = 0;
    }
  }
  /// TWO ///
  else if (content.substring(1) == "A3 B3 D1 18")
  {
    if (card2Balance >= 5)
    {
      card2Balance = card2Balance - 5;
      Serial.println(card2Balance);
      Serial.println("Connect your device");
      lcd.clear();
      lcd.setCursor(3,0);
      lcd.print("Connect Device");
      lcd.setCursor(0,1);
      lcd.print("--------------------");
      lcd.setCursor(2,2);
      lcd.print("Current Balance:");
      lcd.setCursor(7,3);
      lcd.print(card2Balance);
      digitalWrite(relayPinCharge, LOW);
      delay(500);
      digitalWrite(relayPinCharge, HIGH);
      delay(2000);
      chargingState  = 0;
    }
    else
    {
      card = content.substring(1);
      Serial.println(card2Balance);
      Serial.println("Please Recharge");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Insufficient Balance");
      lcd.setCursor(2,1);
      lcd.print("Please Recharge");
      lcd.setCursor(0,2);
      lcd.print("--------------------");
      lcd.setCursor(0,3);
      lcd.print("Card Balance:");
      lcd.setCursor(14,3);
      lcd.print(card2Balance);
      delay(2000);
      chargingState  = 0;
    }
  }
  /// THREE ///
  else if (content.substring(1) == "0B 3D C1 28")
  {
    if (card3Balance >= 5)
    {
      card3Balance = card3Balance - 5;
      Serial.println(card3Balance);
      Serial.println("Connect your device");
      lcd.clear();
      lcd.setCursor(3,0);
      lcd.print("Connect Device");
      lcd.setCursor(0,1);
      lcd.print("--------------------");
      lcd.setCursor(2,2);
      lcd.print("Current Balance:");
      lcd.setCursor(7,3);
      lcd.print(card3Balance);
      digitalWrite(relayPinCharge, LOW);
      delay(500);
      digitalWrite(relayPinCharge, HIGH);
      delay(2000);
      chargingState  = 0;
    }
    else
    {
      card = content.substring(1);
      Serial.println(card3Balance);
      Serial.println("Please Recharge");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Insufficient Balance");
      lcd.setCursor(2,1);
      lcd.print("Please Recharge");
      lcd.setCursor(0,2);
      lcd.print("--------------------");
      lcd.setCursor(0,3);
      lcd.print("Card Balance:");
      lcd.setCursor(14,3);
      lcd.print(card3Balance);
      delay(2000);
      chargingState  = 0;
    }
  }
  /// FOUR ///
  else if (content.substring(1) == "3B 04 AE 28")
  {
    if (card4Balance >= 5)
    {
      card4Balance = card4Balance - 5;
      Serial.println(card4Balance);
      Serial.println("Connect your device");
      lcd.clear();
      lcd.setCursor(3,0);
      lcd.print("Connect Device");
      lcd.setCursor(0,1);
      lcd.print("--------------------");
      lcd.setCursor(2,2);
      lcd.print("Current Balance:");
      lcd.setCursor(7,3);
      lcd.print(card4Balance);
      digitalWrite(relayPinCharge, LOW);
      delay(500);
      digitalWrite(relayPinCharge, HIGH);
      delay(2000);
      chargingState  = 0;
    }
    else
    {
      card = content.substring(1);
      Serial.println(card4Balance);
      Serial.println("Please Recharge");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Insufficient Balance");
      lcd.setCursor(2,1);
      lcd.print("Please Recharge");
      lcd.setCursor(0,2);
      lcd.print("--------------------");
      lcd.setCursor(0,3);
      lcd.print("Card Balance:");
      lcd.setCursor(14,3);
      lcd.print(card4Balance);
      delay(2000);
      chargingState  = 0;
    }
  }
  /// FIVE ///
  else if (content.substring(1) == "9B 01 BD 28")
  {
    if (card5Balance >= 5)
    {
      card5Balance = card5Balance - 5;
      Serial.println(card5Balance);
      Serial.println("Connect your device");
      lcd.clear();
      lcd.setCursor(3,0);
      lcd.print("Connect Device");
      lcd.setCursor(0,1);
      lcd.print("--------------------");
      lcd.setCursor(2,2);
      lcd.print("Current Balance:");
      lcd.setCursor(7,3);
      lcd.print(card5Balance);
      digitalWrite(relayPinCharge, LOW);
      delay(500);
      digitalWrite(relayPinCharge, HIGH);
      delay(2000);
      chargingState  = 0;
    }
    else
    {
      card = content.substring(1);
      Serial.println(card5Balance);
      Serial.println("Please Recharge");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Insufficient Balance");
      lcd.setCursor(2,1);
      lcd.print("Please Recharge");
      lcd.setCursor(0,2);
      lcd.print("--------------------");
      lcd.setCursor(0,3);
      lcd.print("Card Balance:");
      lcd.setCursor(14,3);
      lcd.print(card5Balance);
      delay(2000);
      chargingState  = 0;
    }
  }

  
  else   {
    lcd.clear();
    lcd.setCursor(3,0);
    lcd.print("Access Denied");
    lcd.setCursor(0,1);
    lcd.print("--------------------");
    lcd.setCursor(1,2);
    lcd.print("Unregistered RFID");
    Serial.println("Access denied");
    chargingState  = 0;
    delay(2000);
  }
}

///////////////////////////////////////// REFILLING ////////////////////////////////
void refilling(){
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("-Refilling Station-");
  lcd.setCursor(2,2);
  lcd.print("Scan Your RFID");
  lcd.setCursor(1,3);
  lcd.print("=====>=====>=====>");
  delay(1000);
  if (digitalRead(resetBtn)== LOW){
    ticketingState = 0; 
    refillingState = 0; 
    chargingState = 0; 
    dispensingState = 0; 
    rechargeState = 0; 
    rfidPaymentState = 0;
    billPaymentState = 0;
    lcd.clear();
    lcd.setCursor(2,1);
    lcd.print("Processing Reset");
    lcd.setCursor(3,2);
    lcd.print("Please Wait...");
    delay(2000);
   } 
  if ( ! mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }
  if ( ! mfrc522.PICC_ReadCardSerial())
  {
    return;
  }

  String content = "";
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();
  /// ONE ///
  if (content.substring(1) == "EB 7F BD 28")
  {
    if (card1Balance >= 10)
    {
      card1Balance = card1Balance - 10;
      lcd.clear();
      lcd.setCursor(4,0);
      lcd.print("Your Current");
      lcd.setCursor(4,1);
      lcd.print("RFID balance:");
      lcd.setCursor(8,2);
      lcd.print(card1Balance);
      delay(2000);
      refillingState = 0;
      startRefillState = 1;
    }
    else
    {
      card = content.substring(1);
      Serial.println(card1Balance);
      Serial.println("Please Recharge");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Insufficient Balance");
      lcd.setCursor(0,1);
      lcd.print("Please Recharge");
      lcd.setCursor(0,2);
      lcd.print("Card Balance");
      lcd.setCursor(0,3);
      lcd.print(card1Balance);
      delay(2000);
      refillingState = 0;
    }
  }
  /// TWO ///
  else if (content.substring(1) == "A3 B3 D1 18")
  {
    if (card2Balance >= 10)
    {
      card2Balance = card2Balance - 10;
      lcd.clear();
      lcd.setCursor(4,0);
      lcd.print("Your Current");
      lcd.setCursor(4,1);
      lcd.print("RFID balance:");
      lcd.setCursor(8,2);
      lcd.print(card2Balance);
      delay(2000);
      refillingState = 0;
      startRefillState = 1;
    }
    else
    {
      card = content.substring(1);
      Serial.println(card2Balance);
      Serial.println("Please Recharge");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Insufficient Balance");
      lcd.setCursor(0,1);
      lcd.print("Please Recharge");
      lcd.setCursor(0,2);
      lcd.print("Card Balance");
      lcd.setCursor(0,3);
      lcd.print(card2Balance);
      delay(2000);
      refillingState = 0;
    }
  }
  /// THREE ///
  else if (content.substring(1) == "0B 3D C1 28")
  {
    if (card3Balance >= 10)
    {
      card3Balance = card3Balance - 10;
      lcd.clear();
      lcd.setCursor(4,0);
      lcd.print("Your Current");
      lcd.setCursor(4,1);
      lcd.print("RFID balance:");
      lcd.setCursor(8,2);
      lcd.print(card3Balance);
      delay(2000);
      refillingState = 0;
      startRefillState = 1;
    }
    else
    {
      card = content.substring(1);
      Serial.println(card3Balance);
      Serial.println("Please Recharge");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Insufficient Balance");
      lcd.setCursor(0,1);
      lcd.print("Please Recharge");
      lcd.setCursor(0,2);
      lcd.print("Card Balance");
      lcd.setCursor(0,3);
      lcd.print(card3Balance);
      delay(2000);
      refillingState = 0;
    }
  }
  /// FOUR ///
  else if (content.substring(1) == "3B 04 AE 28")
  {
    if (card4Balance >= 10)
    {
      card4Balance = card4Balance - 10;
      lcd.clear();
      lcd.setCursor(4,0);
      lcd.print("Your Current");
      lcd.setCursor(4,1);
      lcd.print("RFID balance:");
      lcd.setCursor(8,2);
      lcd.print(card4Balance);
      delay(2000);
      refillingState = 0;
      startRefillState = 1;
    }
    else
    {
      card = content.substring(1);
      Serial.println(card4Balance);
      Serial.println("Please Recharge");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Insufficient Balance");
      lcd.setCursor(0,1);
      lcd.print("Please Recharge");
      lcd.setCursor(0,2);
      lcd.print("Card Balance");
      lcd.setCursor(0,3);
      lcd.print(card4Balance);
      delay(2000);
      refillingState = 0;
    }
  }
  /// FIVE ///
  else if (content.substring(1) == "9B 01 BD 28")
  {
    if (card5Balance >= 10)
    {
      card5Balance = card5Balance - 10;
      lcd.clear();
      lcd.setCursor(4,0);
      lcd.print("Your Current");
      lcd.setCursor(4,1);
      lcd.print("RFID balance:");
      lcd.setCursor(8,2);
      lcd.print(card5Balance);
      delay(2000);
      refillingState = 0;
      startRefillState = 1;
    }
    else
    {
      card = content.substring(1);
      Serial.println(card5Balance);
      Serial.println("Please Recharge");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Insufficient Balance");
      lcd.setCursor(0,1);
      lcd.print("Please Recharge");
      lcd.setCursor(0,2);
      lcd.print("Card Balance");
      lcd.setCursor(0,3);
      lcd.print(card5Balance);
      delay(2000);
      refillingState = 0;
    }
  }
  else   {
    lcd.clear();
    lcd.setCursor(3,0);
    lcd.print("Access Denied");
    lcd.setCursor(0,1);
    lcd.print("--------------------");
    lcd.setCursor(1,2);
    lcd.print("Unregistered RFID");
    Serial.println("Access denied");
    refillingState = 0;
    delay(2000);
  }
}

void startRefill(){
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("Put your bottle in");
  lcd.setCursor(3,1);
  lcd.print("Refilling Area");
  lcd.setCursor(1,3);
  lcd.print("<=====<=====<=====");
  delay(500);
  if (digitalRead(irSensor)== LOW)
  {
    lcd.clear();
    lcd.setCursor(2,1);
    lcd.print("Refilling Bottle");
    lcd.setCursor(2,2);
    lcd.print("Please Wait...");
    delay(2000);
    digitalWrite(relayPinRefill, LOW);
    delay(1800);
    digitalWrite(relayPinRefill, HIGH);
    lcd.clear();
    lcd.setCursor(2,0);
    lcd.print("----DONE----");
    
    delay(4000);
    lcd.clear();
    startRefillState = 0;
  }
  
  else if (digitalRead(resetBtn) == LOW){
    ticketingState = 0; 
    refillingState = 0; 
    chargingState = 0; 
    dispensingState = 0; 
    rechargeState = 0; 
    rfidPaymentState = 0;
    billPaymentState = 0;
    startRefillState = 0;
    lcd.clear();
    lcd.setCursor(5,1);
    lcd.print("Stand by");
    lcd.setCursor(3,2);
    lcd.print("Please Wait...");
    delay(1000);
  }
}
//////////////////////////////////////RECHARGE////////////////////////////////////
void recharge(){
  Serial.println("Scan RFID");
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("-Recharge Feature-");
  lcd.setCursor(2,2);
  lcd.print("Scan your RFID");
  delay(1000);
  if (digitalRead(resetBtn) == LOW){
    ticketingState = 0; 
    refillingState = 0; 
    chargingState = 0; 
    dispensingState = 0; 
    rechargeState = 0; 
    rfidPaymentState = 0;
    billPaymentState = 0;
    lcd.clear();
    lcd.setCursor(2,1);
    lcd.print("Processing Reset");
    lcd.setCursor(3,2);
    lcd.print("Please Wait...");
    delay(2000);
    }
  if ( ! mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }
  if ( ! mfrc522.PICC_ReadCardSerial())
  {
    return;
  }

  //String content = "";
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    rechargeRfidContent.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    rechargeRfidContent.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  rechargeRfidContent.toUpperCase();

  /// ONE ///
  if (rechargeRfidContent.substring(1) == "EB 7F BD 28")
  {
    rechargeState = 0;
    startRechargeState = 1;
    whatCard = 1;

  }
  /// TWO ///
  else if (rechargeRfidContent.substring(1) == "A3 B3 D1 18")
  {
    rechargeState = 0;
    startRechargeState = 1;
    whatCard = 2;
  }
  /// THREE ///
  else if (rechargeRfidContent.substring(1) == "0B 3D C1 28")
  {
    rechargeState = 0;
    startRechargeState = 1;
    whatCard = 3;
  }
  /// FOUR ///
  else if (rechargeRfidContent.substring(1) == "3B 04 AE 28")
  {
    rechargeState = 0;
    startRechargeState = 1;
    whatCard = 4;
  }
  /// FIVE ///
  else if (rechargeRfidContent.substring(1) == "9B 01 BD 28")
  {
    rechargeState = 0;
    startRechargeState = 1;
    whatCard = 5;
  }
  

  else   {
    lcd.clear();
    lcd.setCursor(3,0);
    lcd.print("Access Denied");
    lcd.setCursor(0,1);
    lcd.print("--------------------");
    lcd.setCursor(1,2);
    lcd.print("Unregistered RFID");
    Serial.println("Access denied");
    rechargeRfidContent = "";
    rechargeState = 0;
    startRechargeState = 0;
    delay(2000);
  }
}
void startRecharge(){
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("Insert Bill Amount");
  lcd.setCursor(1,1);
  lcd.print("Press Button 5 if");
  lcd.setCursor(2,2);
  lcd.print("Done Recharging");
  lcd.setCursor(3,3);
  lcd.print("Credit:");
  lcd.setCursor(11,3);
  lcd.print(credit);
  delay(1500);
  ///ONE///
  if(whatCard == 1){
    Serial.println(rechargeRfidContent);
    if(digitalRead(rechargeBtn) == LOW){
      lcd.clear();
      card1Balance = card1Balance + credit;
      Serial.println(card1Balance);
      Serial.println(credit);
      lcd.setCursor(0,0);
      lcd.print("Adding: PHP");
      lcd.setCursor(12,0);
      lcd.print(credit);
      lcd.setCursor(0,1);
      lcd.print("--------------------");
      lcd.setCursor(0,2);
      lcd.print("Your Balance Now is:");
      lcd.setCursor(7,3);
      lcd.print(card1Balance);
      delay(4000);
      credit = 0;
      rechargeRfidContent = "";
      rechargeState = 0;
      startRechargeState = 0;
      }
    }
  ///TWO///
  if(whatCard == 2){
    Serial.println(rechargeRfidContent);
    if(digitalRead(rechargeBtn) == LOW){
      lcd.clear();
      card2Balance = card2Balance + credit;
      Serial.println(card2Balance);
      Serial.println(credit);
      lcd.setCursor(0,0);
      lcd.print("Adding: PHP");
      lcd.setCursor(12,0);
      lcd.print(credit);
      lcd.setCursor(0,1);
      lcd.print("--------------------");
      lcd.setCursor(0,2);
      lcd.print("Your Balance Now is:");
      lcd.setCursor(7,3);
      lcd.print(card2Balance);
      delay(4000);
      credit = 0;
      rechargeRfidContent = "";
      rechargeState = 0;
      startRechargeState = 0;
      }
    }
  ///THREE///
  if(whatCard == 3){
    Serial.println(rechargeRfidContent);
    if(digitalRead(rechargeBtn) == LOW){
      lcd.clear();
      card3Balance = card3Balance + credit;
      Serial.println(card3Balance);
      Serial.println(credit);
      lcd.setCursor(0,0);
      lcd.print("Adding: PHP");
      lcd.setCursor(12,0);
      lcd.print(credit);
      lcd.setCursor(0,1);
      lcd.print("--------------------");
      lcd.setCursor(0,2);
      lcd.print("Your Balance Now is:");
      lcd.setCursor(7,3);
      lcd.print(card3Balance);
      delay(4000);
      credit = 0;
      rechargeRfidContent = "";
      rechargeState = 0;
      startRechargeState = 0;
      }
    }
  ///FOUR///
  if(whatCard == 4){
    Serial.println(rechargeRfidContent);
    if(digitalRead(rechargeBtn) == LOW){
      lcd.clear();
      card4Balance = card4Balance + credit;
      Serial.println(card4Balance);
      Serial.println(credit);
      lcd.setCursor(0,0);
      lcd.print("Adding: PHP");
      lcd.setCursor(12,0);
      lcd.print(credit);
      lcd.setCursor(0,1);
      lcd.print("--------------------");
      lcd.setCursor(0,2);
      lcd.print("Your Balance Now is:");
      lcd.setCursor(7,3);
      lcd.print(card4Balance);
      delay(4000);
      credit = 0;
      rechargeRfidContent = "";
      rechargeState = 0;
      startRechargeState = 0;
      }
    }
  ///FIVE///
  if(whatCard == 5){
    Serial.println(rechargeRfidContent);
    if(digitalRead(rechargeBtn) == LOW){
      lcd.clear();
      card5Balance = card5Balance + credit;
      Serial.println(card5Balance);
      Serial.println(credit);
      lcd.setCursor(0,0);
      lcd.print("Adding: PHP");
      lcd.setCursor(12,0);
      lcd.print(credit);
      lcd.setCursor(0,1);
      lcd.print("--------------------");
      lcd.setCursor(0,2);
      lcd.print("Your Balance Now is:");
      lcd.setCursor(7,3);
      lcd.print(card5Balance);
      delay(4000);
      credit = 0;
      rechargeRfidContent = "";
      rechargeState = 0;
      startRechargeState = 0;
      }
    }
  }

////////////////////////////// DISPENSING /////////////////////////////
void dispensing(){
  ////////// RFID PAYMENT ////////////
  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print("Alcohol Dispensing");
  lcd.setCursor(0,1);
  lcd.print("Choose Your Payment");
  lcd.setCursor(0,2);
  lcd.print("RFID ==> Button 6");
  lcd.setCursor(0,3);
  lcd.print("CASH ==> Button 7");
  delay(1000);
  if (digitalRead(rfidPaymentBtn) == LOW){
    dispensingRfidPaymentState = 1;
    dispensingState = 0;
    }
  ////////// BILL PAYMENT ////////////
  else if (billPaymentState == 1){
    
    }
  else if (digitalRead(billPaymentBtn) == LOW){
    dispensingBillPaymentState = 1;
    dispensingState = 0;
    }

  else if (digitalRead(resetBtn) == LOW){
    ticketingState = 0; 
    refillingState = 0; 
    chargingState = 0; 
    dispensingState = 0; 
    rechargeState = 0; 
    rfidPaymentState = 0;
    billPaymentState = 0;
    lcd.clear();
    lcd.setCursor(2,1);
    lcd.print("Processing Reset");
    lcd.setCursor(3,2);
    lcd.print("Please Wait...");
    delay(2000);
    }
}

void dispensingRfidPayment(){
  Serial.println("Scan RFID for Dispensing");
  lcd.clear();
  lcd.setCursor(3,0);
  lcd.print("-RFID Payment-");
  lcd.setCursor(2,2);
  lcd.print("Scan Your RFID");
  lcd.setCursor(1,3);
  lcd.print("=====>=====>=====>");
  delay(500);
  if ( ! mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }
  if ( ! mfrc522.PICC_ReadCardSerial())
  {
    return;
  }

  String content = "";
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();

  /// ONE ///
  if (content.substring(1) == "EB 7F BD 28")
  {
    if (card1Balance >= 20)
    {
      card1Balance = card1Balance - 20;
      Serial.println(card1Balance);
      Serial.println("Please wait Alcohol");
      lcd.clear();
      lcd.setCursor(5,0);
      lcd.print("Dispensing");
      lcd.setCursor(6,1);
      lcd.print("Alcohol");
      lcd.setCursor(0,2);
      lcd.print("--------------------");
      lcd.setCursor(2,3);
      lcd.print("Please Wait....");
      digitalWrite(relayPinDispensing, LOW);
      delay(800);
      digitalWrite(relayPinDispensing,HIGH);
      delay(2000);
      lcd.clear();
      lcd.setCursor(2,0);
      lcd.print("You can now get");
      lcd.setCursor(0,1);
      lcd.print("your Alcohol Bottle");
      lcd.setCursor(3,2);
      lcd.print("Paid 20 pesos");
      lcd.setCursor(0,3);
      lcd.print("Balance: ");
      lcd.setCursor(10,3);
      lcd.print(card1Balance);
      delay(2000);
      dispensingRfidPaymentState = 0;
      dispensingState = 0;
    }
    else
    {
      card = content.substring(1);
      Serial.println(card1Balance);
      Serial.println("Please Recharge");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Insufficient Balance");
      lcd.setCursor(0,1);
      lcd.print("Please Recharge");
      lcd.setCursor(0,2);
      lcd.print("Card Balance");
      lcd.setCursor(0,3);
      lcd.print(card1Balance);
      delay(2000);
      dispensingRfidPaymentState = 0;
      dispensingState = 0;
    }
  }
  /// TWO ///
  else if (content.substring(1) == "A3 B3 D1 18")
  {
    if (card2Balance >= 20)
    {
      card2Balance = card2Balance - 20;
      Serial.println(card2Balance);
      Serial.println("Please wait Alcohol");
      lcd.clear();
      lcd.setCursor(5,0);
      lcd.print("Dispensing");
      lcd.setCursor(6,1);
      lcd.print("Alcohol");
      lcd.setCursor(0,2);
      lcd.print("--------------------");
      lcd.setCursor(2,3);
      lcd.print("Please Wait....");
      digitalWrite(relayPinDispensing, LOW);
      delay(800);
      digitalWrite(relayPinDispensing,HIGH);
      delay(2000);
      lcd.clear();
      lcd.setCursor(2,0);
      lcd.print("You can now get");
      lcd.setCursor(0,1);
      lcd.print("your Alcohol Bottle");
      lcd.setCursor(3,2);
      lcd.print("Paid 20 pesos");
      lcd.setCursor(0,3);
      lcd.print("Balance: ");
      lcd.setCursor(10,3);
      lcd.print(card2Balance);
      delay(2000);
      dispensingRfidPaymentState = 0;
      dispensingState = 0;
    }
    else
    {
      card = content.substring(1);
      Serial.println(card2Balance);
      Serial.println("Please Recharge");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Insufficient Balance");
      lcd.setCursor(0,1);
      lcd.print("Please Recharge");
      lcd.setCursor(0,2);
      lcd.print("Card Balance");
      lcd.setCursor(0,3);
      lcd.print(card2Balance);
      delay(2000);
      dispensingRfidPaymentState = 0;
      dispensingState = 0;
    }
  }
  /// THREE ///
  else if (content.substring(1) == "0B 3D C1 28")
  {
    if (card3Balance >= 20)
    {
      card3Balance = card3Balance - 20;
      Serial.println(card3Balance);
      Serial.println("Please wait Alcohol");
      lcd.clear();
      lcd.setCursor(5,0);
      lcd.print("Dispensing");
      lcd.setCursor(6,1);
      lcd.print("Alcohol");
      lcd.setCursor(0,2);
      lcd.print("--------------------");
      lcd.setCursor(2,3);
      lcd.print("Please Wait....");
      digitalWrite(relayPinDispensing, LOW);
      delay(800);
      digitalWrite(relayPinDispensing,HIGH);
      delay(2000);
      lcd.clear();
      lcd.setCursor(2,0);
      lcd.print("You can now get");
      lcd.setCursor(0,1);
      lcd.print("your Alcohol Bottle");
      lcd.setCursor(3,2);
      lcd.print("Paid 20 pesos");
      lcd.setCursor(0,3);
      lcd.print("Balance: ");
      lcd.setCursor(10,3);
      lcd.print(card3Balance);
      delay(2000);
      dispensingRfidPaymentState = 0;
      dispensingState = 0;
    }
    else
    {
      card = content.substring(1);
      Serial.println(card3Balance);
      Serial.println("Please Recharge");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Insufficient Balance");
      lcd.setCursor(0,1);
      lcd.print("Please Recharge");
      lcd.setCursor(0,2);
      lcd.print("Card Balance");
      lcd.setCursor(0,3);
      lcd.print(card3Balance);
      delay(2000);
      dispensingRfidPaymentState = 0;
      dispensingState = 0;
    }
  }
  /// FOUR ///
  else if (content.substring(1) == "3B 04 AE 28")
  {
    if (card4Balance >= 20)
    {
      card4Balance = card4Balance - 20;
      Serial.println(card4Balance);
      Serial.println("Please wait Alcohol");
      lcd.clear();
      lcd.setCursor(5,0);
      lcd.print("Dispensing");
      lcd.setCursor(6,1);
      lcd.print("Alcohol");
      lcd.setCursor(0,2);
      lcd.print("--------------------");
      lcd.setCursor(2,3);
      lcd.print("Please Wait....");
      digitalWrite(relayPinDispensing, LOW);
      delay(800);
      digitalWrite(relayPinDispensing,HIGH);
      delay(2000);
      lcd.clear();
      lcd.setCursor(2,0);
      lcd.print("You can now get");
      lcd.setCursor(0,1);
      lcd.print("your Alcohol Bottle");
      lcd.setCursor(3,2);
      lcd.print("Paid 20 pesos");
      lcd.setCursor(0,3);
      lcd.print("Balance: ");
      lcd.setCursor(10,3);
      lcd.print(card4Balance);
      delay(2000);
      dispensingRfidPaymentState = 0;
      dispensingState = 0;
    }
    else
    {
      card = content.substring(1);
      Serial.println(card4Balance);
      Serial.println("Please Recharge");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Insufficient Balance");
      lcd.setCursor(0,1);
      lcd.print("Please Recharge");
      lcd.setCursor(0,2);
      lcd.print("Card Balance");
      lcd.setCursor(0,3);
      lcd.print(card4Balance);
      delay(2000);
      dispensingRfidPaymentState = 0;
      dispensingState = 0;
    }
  }
  /// FIVE ///
  else if (content.substring(1) == "9B 01 BD 28")
  {
    if (card5Balance >= 20)
    {
      card5Balance = card5Balance - 20;
      Serial.println(card5Balance);
      Serial.println("Please wait Alcohol");
      lcd.clear();
      lcd.setCursor(5,0);
      lcd.print("Dispensing");
      lcd.setCursor(6,1);
      lcd.print("Alcohol");
      lcd.setCursor(0,2);
      lcd.print("--------------------");
      lcd.setCursor(2,3);
      lcd.print("Please Wait....");
      digitalWrite(relayPinDispensing, LOW);
      delay(800);
      digitalWrite(relayPinDispensing,HIGH);
      delay(2000);
      lcd.clear();
      lcd.setCursor(2,0);
      lcd.print("You can now get");
      lcd.setCursor(0,1);
      lcd.print("your Alcohol Bottle");
      lcd.setCursor(3,2);
      lcd.print("Paid 20 pesos");
      lcd.setCursor(0,3);
      lcd.print("Balance: ");
      lcd.setCursor(10,3);
      lcd.print(card5Balance);
      delay(2000);
      dispensingRfidPaymentState = 0;
      dispensingState = 0;
    }
    else
    {
      card = content.substring(1);
      Serial.println(card5Balance);
      Serial.println("Please Recharge");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Insufficient Balance");
      lcd.setCursor(0,1);
      lcd.print("Please Recharge");
      lcd.setCursor(0,2);
      lcd.print("Card Balance");
      lcd.setCursor(0,3);
      lcd.print(card5Balance);
      delay(2000);
      dispensingRfidPaymentState = 0;
      dispensingState = 0;
    }
  }

  else if (digitalRead(resetBtn) == LOW){
    ticketingState = 0; 
    refillingState = 0; 
    chargingState = 0; 
    dispensingState = 0; 
    rechargeState = 0; 
    rfidPaymentState = 0;
    billPaymentState = 0;
    lcd.clear();
    lcd.setCursor(2,1);
    lcd.print("Processing Reset");
    lcd.setCursor(3,2);
    lcd.print("Please Wait...");
    delay(2000);
    }

  else   {
    lcd.clear();
    lcd.setCursor(3,0);
    lcd.print("Access Denied");
    lcd.setCursor(0,1);
    lcd.print("--------------------");
    lcd.setCursor(1,2);
    lcd.print("Unregistered RFID");
    Serial.println("Access denied");
    rfidPaymentState  = 0;
    dispensingState = 0;
    delay(2000);
  }
}

void dispensingBillPayment(){
  Serial.println(credit);
  Serial.println("INSERT BILL");
  lcd.clear();
  lcd.setCursor(3,0);
  lcd.print("-BILL Payment-");
  lcd.setCursor(4,1);
  lcd.print("Insert Bill");
  lcd.setCursor(5,2);
  lcd.print("20 Pesos");
  lcd.setCursor(1,3);
  lcd.print("<=====<=====<=====");
  delay(100);
  if (credit == 20){
    credit = 0;
    digitalWrite(relayPinDispensing, LOW);
    delay(800);
    digitalWrite(relayPinDispensing,HIGH);
    lcd.clear();
    lcd.setCursor(2,1);
    lcd.print("You can now get");
    lcd.setCursor(0,2);
    lcd.print("your Alcohol Bottle");
    dispensingBillPaymentState = 0;
    delay(3000);
    }
  else if (credit > 20){
    lcd.clear();
    lcd.setCursor(3,1);
    lcd.print("Invalid Amount");
    lcd.setCursor(3,2);
    lcd.print("20 Pesos Only");
    credit = 0;
    dispensingBillPaymentState = 0;
    delay(3000);
    }
  }
