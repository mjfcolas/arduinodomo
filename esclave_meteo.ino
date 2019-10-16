#include <OneWire.h>
#include <SFE_BMP180.h>

#include <SoftwareSerial.h>
#include <DHT.h>


#define VW_MAX_MESSAGE_LEN 200
#define SET_PIN 10
#define ALTITUDE 68.0 // Altitude Appartement

#define DHTTYPE DHT22
#define DHTPIN 3

/* Broche du bus 1-Wire */
const byte BROCHE_ONEWIRE = 5;
boolean sendInfos = false;

SoftwareSerial hc12(9, 7);
DHT dht(DHTPIN, DHTTYPE);
SFE_BMP180 pressure;

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

byte getPression(double *Pout, double *p0out){
  char status;
  double T,P,p0,a;
  //Debut mesure temp
  status = pressure.startTemperature();
  
  if (status != 0)
  {
    delay(status);
    status = pressure.getTemperature(T);
    {
      //Debut mesure Pression
      status = pressure.startPressure(3);
      if (status != 0)
      {
        delay(status);
        status = pressure.getPressure(P,T);
        if (status != 0)
        {
          *Pout = P;
          p0 = pressure.sealevel(P,ALTITUDE);
          *p0out = p0;
          return 0;
        }
      }
    }
  }
  return -1;
}

void readSerial(unsigned int MAXBUF, char* buffer){
  
    buffer[0] = 0;
    int i = 0;
    while(hc12.available() && i < MAXBUF){
      
      buffer[i++] = hc12.read();
      delay(10);
    }
  }    

/** Fonction setup() **/
void setup() {

  /* Initialisation du port s?rie */
  Serial.begin(2400);
  
  int i = 0;
  dht.begin();
    
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
  
  if (pressure.begin())
    Serial.println("BMP180 init success");
  else
  {
    Serial.println("BMP180 init fail\n\n");
  }
  
  EOT[0] = 4;
  EOL[0] = '\n';
}


void reinitBuf(char* buf, int bufsize)
{
  for(int i = 0 ; i < bufsize; i++){
    buf[i] = 0;
  }
}

/** Fonction loop() **/
void loop() {
  byte taille_messageRx = 8;
  char messageRx[taille_messageRx];
  readSerial(taille_messageRx, messageRx);
  if (messageRx[0] != 0) {
    Serial.println((char*)messageRx);
    if(strncmp((char*)messageRx, "CALL QT", 7) == 0){
      Serial.println((char*) "Appel recu"); // Affiche le message
      sendInfos=true;
    }
  }
  
  if(sendInfos){
    delay(800);
    //TEMPERATURE
    float temperature = 0;
    /* Lit la temp?rature ambiante ? ~1Hz */
    if (getTemperature(&temperature, true) != READ_OK) {
      Serial.println(F("Erreur de lecture du capteur temp?rature"));
      return;
    }
  
    //PRESSION
    double P,p0;
    boolean pressionOk = true;
    if (getPression(&P, &p0) != 0) {
      Serial.println(F("Erreur de lecture du capteur pression"));
      pressionOk = false;
    }
    
    //HYGRO
    float hum = dht.readHumidity();;
    boolean hygroOk = true;
    if (isnan(hum)) {
      hygroOk = false;
      Serial.println(F("Failed to read from DHT sensor!"));
    }          
      
    char message[100] = "";
    reinitBuf(message, 100);
    char buf[10];
      
    strcat(message, "RECE QT\n");
 
    reinitBuf(buf, 10);;
    strcat(message, "RECE T ");
    dtostrf(temperature, 2, 2, buf);
    strcat(message, buf);
    strcat(message, "\n");
    
    if(pressionOk){
      reinitBuf(buf, 10);
      strcat(message, "RECE AP ");
      dtostrf(P, 4, 1, buf);
      strcat(message, buf);
      strcat(message, "\n");
    
      reinitBuf(buf, 10);
      strcat(message, "RECE RP ");
      dtostrf(p0, 4, 1, buf);
      strcat(message, buf);
      strcat(message, "\n");
    }
    
    if(hygroOk){
      reinitBuf(buf, 10);
      strcat(message, "RECE HH ");
      dtostrf(hum, 4, 2, buf);
      strcat(message, buf);
    }
    strcat(message, EOT);
    strcat(message, "\n");
    Serial.print(message);
    Serial.println("");
    hc12.flush();
    hc12.write(message);
    sendInfos = false;
  }
}  
