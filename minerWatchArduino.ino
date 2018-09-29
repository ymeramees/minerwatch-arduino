char receivedChar;
char lastChar;
bool newData = false;
int heartBeatTime = 300;  //Aeg, peale mida viimasest elus olemise teatest kaevurile restart tehakse, sekundites
int lastHeartBeat = 0;  //Sekundid alates viimasest elus olemise teatest, sekundites
int restartDelay = 500; //Aeg, kui kaua hoitakse restarti tehes pin sees, millisekundites

#define juhtKaevur 0

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(juhtKaevur, OUTPUT);
  pinMode(1, OUTPUT);  //Väljundiks
  pinMode(2, OUTPUT);  //Väljundiks
  pinMode(3, OUTPUT);  //Väljundiks
  // initialize timer1 
  //noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  OCR1A = 15625;            // compare match register 16MHz/256/1Hz
  TCCR1B |= (1 << WGM12);   // CTC mode
  //TCCR1B |= (1 << CS12);    // 256 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  // set prescaler to 1024 and start the timer
  //TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
  //interrupts();             // enable all interrupts

  digitalWrite(LED_BUILTIN, HIGH);
  Serial.begin(9600);
  Serial.println("MinerWatch ver. 0.1");
}

void loop() {
  // put your main code here, to run repeatedly:
  readSerial();
  if(receivedChar == 'e') {
    Serial.println("Sain, elus");
    lastHeartBeat = 0;  //Taimerile reset
    lastChar = 0;
    receivedChar = 0;
    newData = false;
  }else if(lastChar == 'r') {
    restart(receivedChar - 48);
    lastChar = 0;
    receivedChar = 0;
    newData = false;
  }

  if((TIFR1 & (1 << OCF1A))) {
    lastHeartBeat++;
    if(lastHeartBeat >= heartBeatTime) {  //Juhtiv kaevur ei vasta, teha restart
      restart(juhtKaevur);
      lastHeartBeat = 0;
    }
    if(digitalRead(LED_BUILTIN)) {
      digitalWrite(LED_BUILTIN, LOW);
    }else {
      digitalWrite(LED_BUILTIN, HIGH);
    }
    TIFR1 |= (1 << OCF1A); // reset the overflow flag
  }
}

void readSerial() {
  if (Serial.available() > 0) {
    if(newData) {
      lastChar = receivedChar;
    }
    if(lastChar == 'a'){  //Uus aeg, kui kaua peale viimast elusolemise signaali juhtkaevurile restart tehakse
      heartBeatTime = Serial.parseInt();
      lastChar = 0;
      receivedChar = 0;
      newData = false;
    }if(lastChar == 'd'){ //Uus restardi tegemise pini viivise aeg, millisekundites
      restartDelay = Serial.parseInt();
      lastChar = 0;
      receivedChar = 0;
      newData = false;
    }else{
      receivedChar = Serial.read();
      newData = true;
    }
 }
}

void restart(int no) {  //Lülitab küsitud pistiku reset pin'i
  digitalWrite(no, HIGH); //Reset sisse
  delay(restartDelay); //Paus
  digitalWrite(no, LOW);  //Reset välja
  
  Serial.print(no);
  Serial.println(", restart");
}

