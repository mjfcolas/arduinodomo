#include <SoftwareSerial.h>

#define INPUT_SIZE 30
#define PARAM_SIZE 5
#define SET_PIN 10
int firstLoop = 0;
boolean doCallMeteo = false;
char meteoKey[PARAM_SIZE+1];
boolean doCallMeteo2 = false;
boolean doCallMeteo3 = false;
boolean doCallChauffage = false;
char chauffageOrder[PARAM_SIZE+1];
boolean doCallChauffageInfo = false;
int VW_MAX_MESSAGE_LEN = 200;

SoftwareSerial hc12(9,8);
char EOT = 4;

int vitesse = 2400;

void setup() {
  Serial.begin(2400);
  
  pinMode(SET_PIN,OUTPUT);
  digitalWrite(SET_PIN,LOW);
  delay(300);
  hc12.begin(2400);
  hc12.print("AT+B2400");
  delay(200);
  hc12.print("AT+C093");
  delay(200);
  hc12.print("AT+P8");
  delay(200);
  
  digitalWrite(SET_PIN,HIGH);// enter transparent mode

}

void loop() {

  
  //while(1){
    //hc12.print(1);
    //if (hc12.available())
    //{
      //Serial.write(hc12.read());
    //}
  //}
  firstLoop = 1;
  if(doCallMeteo){
    callMeteo();
  }
  if(doCallMeteo2){
    callMeteo2();
  }
  if(doCallMeteo3){
    callMeteo3();
  }
  if(doCallChauffage){
    callChauffage();
  }
  if(doCallChauffageInfo){
    callChauffageInfo();
  }
  if(Serial.available()){
    char input[INPUT_SIZE + 1];
    byte size = Serial.readBytes(input, INPUT_SIZE);
    Serial.flush();
    char* command = NULL;
    char* param = NULL;
    input[size] = 0;
    Serial.println(input);
    char* separator = strchr(input, ' ');
    *separator=0;
    command=input;
    param=separator + 1;
    if(strncmp(command, "METEO", 5) == 0 && strncmp(command, "METEO2", 6) != 0){
      doCallMeteo = true;
      strncpy(meteoKey, param, strlen(param));
      meteoKey[strlen(param)]=0;
    }
    if(strncmp(command, "CHAUFF", 6) == 0){
      doCallChauffage = true;
      strncpy(chauffageOrder, param, strlen(param));
      chauffageOrder[strlen(param)]=0;
    }
    if(strncmp(command, "METEO2", 6) == 0){
      doCallMeteo2 = true;
    }
    /*if(strcmp(command, "METEO3", 6) == 0){
      doCallMeteo3 = true;
    }*/
    if(strncmp(command, "CHINFO", 6) == 0){
      doCallChauffageInfo = true;
    }
      
  }
}

void readTimeout(unsigned long eotTimeoutMs, unsigned int MAXBUF, char* buffer){
  
  hc12.flush();
  for(int j = 0; j < MAXBUF; j++){
    buffer[j] = 0;
  }
  unsigned long initTime = millis();
  
  int dataAvailable = 0;
  while(!hc12.available() && millis() - initTime < eotTimeoutMs);
  if(hc12.available()){
    dataAvailable = 1;
  }
  
  int i = 0;
  char curCar = 0;
  while(dataAvailable && (byte)curCar != EOT && i < MAXBUF - 1 && millis() - initTime < eotTimeoutMs){
    if(hc12.available()){
      curCar = hc12.read();
      if(curCar != 1){   
        buffer[i++] = (char)curCar;
      }
      delay(10);  
    }
  }
  if(i >=1){
    buffer[--i] = '\0';
  }
  hc12.flush();
}

void printError(char* message){
  Serial.print("RECE ");
  Serial.print((char*) message);
  Serial.println(" NO_ANSWER"); 
}

void receptionMeteo() {
  char message[300] = "";

  // On attend de recevoir un message
  Serial.println("LOGA - Reception Meteo 1");
  readTimeout(9000, 300, message);
  if (message[0] != 0) {
    if(strncmp((char*)message, "RECE QT", 7) == 0){
      doCallMeteo = false;
      Serial.println("LOGA - Meteo recue");
      Serial.println((char*) message); // Affiche le message
      delay(2000);
    }
  }else{
    printError("QT");
  }
}

void receptionMeteo2() {
  char message[VW_MAX_MESSAGE_LEN];
  // On attend de recevoir un message
  Serial.println("LOGA - Reception Meteo 2");
  readTimeout(9000, 50, message);
  if (message[0] != 0) {
    if(strncmp((char*)message, "RECE N", 6) == 0){
      doCallMeteo2 = false;
      Serial.println("LOGA - Meteo 2 recue");
      // On copie le message, qu'il soit corrompu ou non
      Serial.println((char*) message); // Affiche le message
      delay(2000);
    }
  }else{
    printError("N");
  }
}

void receptionMeteo3() {
  char message[VW_MAX_MESSAGE_LEN];
  // On attend de recevoir un message
  Serial.println("LOGA - Reception Meteo 3");
  readTimeout(9000, VW_MAX_MESSAGE_LEN, message);
  if (message[0] != 0) {
    if(strncmp((char*)message, "RECE O", 6) == 0){
      doCallMeteo3 = false;
      Serial.println("Meteo 3 recue");
      // On copie le message, qu'il soit corrompu ou non
      Serial.println((char*) message); // Affiche le message
    }
  }else{
    printError("O");
  }
}

void receptionChauffage() {
  char message[VW_MAX_MESSAGE_LEN];
  // On attend de recevoir un message
  Serial.println("LOGA - Reception Chauffage");
  readTimeout(9000, VW_MAX_MESSAGE_LEN, message);
  if (message[0] != 0) {
    if(strncmp((char*)message, "RECE C", 6) == 0){
      doCallChauffage = false;
      Serial.println("LogA - Chauffage recu");
      Serial.println((char*) message); // Affiche le message
      delay(2000);
    }
  }else{
    printError("C");
  }
}

void receptionChauffageInfo() {
  char message[VW_MAX_MESSAGE_LEN];
  // On attend de recevoir un message
  Serial.println("LOGA - Reception Chauffage info");
  readTimeout(9000, VW_MAX_MESSAGE_LEN, message);
  if (message[0] != 0) {
    if(strncmp((char*)message, "RECE D", 6) == 0){
      doCallChauffageInfo = false;
      Serial.println("LogA - Info chauffage recue");
      Serial.println((char*) message); // Affiche le message
      delay(2000);
    }
  }else{
    printError("D");
  }
}

void callMeteo() {
  
  //digitalWrite(SET_PIN,LOW);
  //delay(500);
  //hc12.print("AT+C047");
  //delay(3000);
  //digitalWrite(SET_PIN,HIGH);// enter transparent mode  
  //hc12.flush();
  
  char message[8] = "CALL QT";
  strcat(message, "\0");
  Serial.println((char*) message);
  hc12.write(message);
  receptionMeteo();
}

void callMeteo2() {
  
  //pinMode(SET_PIN,OUTPUT);
  //digitalWrite(SET_PIN,LOW);
  //delay(100);
  //hc12.print("AT+C037");
  //delay(100);
  //digitalWrite(SET_PIN,HIGH);// enter transparent mode
  
  char message[8] = "CALL N";
  strcat(message, "\0");
  hc12.write(message);
  receptionMeteo2();
}

void callMeteo3() {
  char message[8] = "CALL O";
  strcat(message, "\0");
  Serial.println((char*) message);
  hc12.write(message);
  receptionMeteo3();
}

void callChauffage() {
  char message[9] = "CALL C ";
  strcat(message, chauffageOrder);
  strcat(message, "\0");
  Serial.println((char*) message);
  hc12.write(message);
  receptionChauffage();
}

void callChauffageInfo() {
  char message[8] = "CALL D";
  strcat(message, "\0");
  Serial.println((char*) message);
  hc12.write(message);
  receptionChauffageInfo();
}
