#include <SoftwareSerial.h>

int pin1 = 3;
int pin2 = 15;
int pinrx = 16;
int pintx = 10;
int setPin = 9;
boolean isOn = true;
boolean firstRun = true;

SoftwareSerial hc12(pintx, pinrx);

char EOT[1];
char EOL[1];

void setup() {
  pinMode(pin1, OUTPUT);
  pinMode(pin2, OUTPUT);
  
  /* Initialisation du port s?rie */
  Serial.begin(2400);
  pinMode(setPin,OUTPUT);
  digitalWrite(setPin,LOW);
  delay(300);
  hc12.begin(2400);
  hc12.print("AT+B2400");
  delay(200);
  hc12.print("AT+C093");
  delay(200);
  hc12.print("AT+P8");
  delay(200);
  digitalWrite(setPin,HIGH);// enter transparent mode
  
  EOT[0] = 4;
  EOL[0] = '\n';
}

void readSerial(unsigned int MAXBUF, char* buffer){
  
    buffer[0] = 0;
    int i = 0;
    while(hc12.available() && i < MAXBUF){
      
      buffer[i++] = hc12.read();
      delay(10);
    }
  }    

void reinitBuf(char* buf, int bufsize)
{
  for(int i = 0 ; i < bufsize; i++){
    buf[i] = 0;
  }
}

// the loop routine runs over and over again forever:
void loop() {
  
  if(firstRun){
    firstRun = false;
    digitalWrite(pin1, LOW);
    digitalWrite(pin2, LOW);
  }
  
  char messageRx[8];
  byte taille_messageRx = 8;
  readSerial(taille_messageRx, messageRx);
  if (messageRx[0] != 0) {
    Serial.println((char*)messageRx);
    if(strncmp((char*)messageRx, "CALL C", 6) == 0){
      Serial.println((char*) "Appel recu"); // Affiche le message
      if(messageRx[7] == '1'){
        Serial.println("ON");
        digitalWrite(pin1, LOW);
        digitalWrite(pin2, LOW);
        isOn = true;
      }else if(messageRx[7] == '0'){
        Serial.println("OFF");
        digitalWrite(pin1, LOW);
        digitalWrite(pin2, HIGH);
        isOn = false;
      }
      delay(1500);
      int length = 7;
      char message[100] = "";
      reinitBuf(message, 100);
      strcat(message, "RECE C");
      //strcat(message, EOT);
      strcat(message, EOL);
      hc12.flush();
      for(int i = 0; i < 10; i++){
        hc12.write(message);
      }
      hc12.write(EOT);
    }else if(strncmp((char*)messageRx, "CALL D", 6) == 0){
      Serial.println((char*) "Envoyer mode"); // Affiche le message
      char message[100] = "";
      reinitBuf(message, 100);
      if(isOn){
        strcat(message, "RECE D 1");
      }else{
        strcat(message, "RECE D 0");
      }
      delay(1500);
      //strcat(message, EOT);
      strcat(message, EOL);
      Serial.print(message);
     
      hc12.flush();
      for(int i = 0; i < 10; i++){
        hc12.write(message);
      }
      hc12.write(EOT);
    }
  }
}
