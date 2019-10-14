#include <OneWire.h>
#include <SoftwareSerial.h>

#define VW_MAX_MES
#define SET_PIN 7

/* Broche du bus 1-Wire */
const byte BROCHE_ONEWIRE = 9;
boolean sendInfos = false;
SoftwareSerial hc12(5, 3);

char EOT[1];
char EOL[1];

/* Code de retour de la fonction getTemperature() */
enum DS18B20_RCODES {
  READ_OK,  // Lecture ok
  NO_SENSOR_FOUND,  // Pas de capteur
  INVALID_ADDRESS,  // Adresse re?ue invalide
  INVALID_SENSOR  // Capteur invalide (pas un DS18B20)
};

/* Cr?ation de l'objet OneWire pour manipuler le bus 1-Wire */
OneWire ds(BROCHE_ONEWIRE);

/**
 * Fonction de lecture de la temp?rature via un capteur DS18B20.
 */
byte getTemperature(float *temperature, byte reset_search) {
  byte data[9], addr[8];
  // data[] : Donn?es lues depuis le scratchpad
  // addr[] : Adresse du module 1-Wire d?tect?
  
  /* Reset le bus 1-Wire ci n?cessaire (requis pour la lecture du premier capteur) */
  if (reset_search) {
    ds.reset_search();
  }
 
  /* Recherche le prochain capteur 1-Wire disponible */
  if (!ds.search(addr)) {
    // Pas de capteur
    Serial.println("NO SENSOR");
    return NO_SENSOR_FOUND;
  }
  
  /* V?rifie que l'adresse a ?t? correctement re?ue */
  if (OneWire::crc8(addr, 7) != addr[7]) {
    // Adresse invalide
    Serial.println("INVALID_ADDRESS");
    return INVALID_ADDRESS;
  }
 
  /* V?rifie qu'il s'agit bien d'un DS18B20 */
  if (addr[0] != 0x28) {
    // Mauvais type de capteur
    Serial.println("INVALID_SENSOR");
    return INVALID_SENSOR;
  }
 
  /* Reset le bus 1-Wire et s?lectionne le capteur */
  ds.reset();
  ds.select(addr);
  
  /* Lance une prise de mesure de temp?rature et attend la fin de la mesure */
  ds.write(0x44, 1);
  delay(800);
  
  /* Reset le bus 1-Wire, s?lectionne le capteur et envoie une demande de lecture du scratchpad */
  ds.reset();
  ds.select(addr);
  ds.write(0xBE);
 
 /* Lecture du scratchpad */
  for (byte i = 0; i < 9; i++) {
    data[i] = ds.read();
  }
   
  /* Calcul de la temp?rature en degr? Celsius */
  *temperature = ((data[1] << 8) | data[0]) * 0.0625; 
  
  // Pas d'erreur
  return READ_OK;
}

void readSerial(unsigned int MAXBUF, char* buffer){
    buffer[0] = 0;
    int i = 0;
    while(hc12.available() && i < MAXBUF){
      char curCar[1];
      curCar[0] = hc12.read();
      buffer[i++] = curCar[0];
      delay(2);
    }
  }  

void reinitBuf(char* buf, int bufsize)
{
  for(int i = 0 ; i < bufsize; i++){
    buf[i] = 0;
  }
}


/** Fonction setup() **/
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
  
  EOT[0] = 4;
  EOL[0] = '\n';
}

/** Fonction loop() **/
void loop() {
  
  //while(1){
    //hc12.print(1);
    //delay(30);
    //if (hc12.available())
    //{
    //  Serial.write(hc12.read());
    //}
  //}
  
  byte taille_messageRx = 8;
  char messageRx[taille_messageRx];
  readSerial(taille_messageRx, messageRx);
  if (messageRx[0] != 0) {
    Serial.println((char*)messageRx);
    if(strncmp((char*)messageRx, "CALL N", 6) == 0){
      Serial.println((char*) "Appel recu"); // Affiche le message
      sendInfos=true;
    }
  }
  
  if(sendInfos){
    delay(800);
    //TEMPERATURE
    float temperature;
    /* Lit la temp?rature ambiante ? ~1Hz */
    if (getTemperature(&temperature, true) != READ_OK) {
      Serial.println(F("Erreur de lecture du capteur temperature"));
      return;
    }
    
    char message[22] = "";
    reinitBuf(message, 22);
    char buf[10];
      
    strcat(message, "RECE N\n");
 
    reinitBuf(buf, 10);;
    strcat(message, "RECE T2 ");
    dtostrf(temperature, 2, 2, buf);
    strcat(message, buf);
    strcat(message, "\n");
    
    strcat(message, EOT);
    strcat(message, EOL);
    Serial.println(message);
    Serial.println("");
    hc12.flush();
    //Send bytes to trigger reception
    //for(int i = 0;  i <10; i++){
      hc12.write(message);
    //}
    sendInfos = false;
  }
}  



