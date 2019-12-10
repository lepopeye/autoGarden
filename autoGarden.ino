// version 11/12/2019 avec l'heure

// ----------------------
// ------- CONFIG -------
// ----------------------

String MOISTURE_NAME[] = {"S1:", "S2:", "S3:"};
String itemsMenu[] = {"Arroser", "Nb sonde", "%Hum 1", "%Hum 2", "%Hum 3", "--", "Niv Eau", "Lumiere"};

int duree_Arrosage = 5;

int itemsMenuMin[] =   {0, 1, 10, 10, 10, 0, 0, 0};
int itemsMenuValue[] = {1, 1, 20, 20, 20, 0, 10, 90};
int itemsMenuMax[] =   {1, 3, 60, 60, 60, 0, 100, 100};

int itemSondeHum[] = {itemsMenuValue[2], itemsMenuValue[3], itemsMenuValue[4]};

// ----------------------
// ----- CAPTEURS -------
// ----------------------

#define MAX_MOISTURE_LEVEL 700
#define MIN_MOISTURE_LEVEL 320

int pin_Hum[] = { 66, 67, 68};
int val_Hum[] = {0, 0, 0, 0};

// ----------------------
// ------ DS3231 --------
// ----------------------

#include "RTClib.h"
RTC_DS3231 RTC;
DateTime now;

// ----------------------
// ----- LUMIERE --------
// ----------------------

int pin_Lum = A7;

// ----------------------
// ---- ULTRA SON -------
// ----------------------

/* Constantes pour les broches */
const byte pin_uson_Trigger = 30; // Broche TRIGGER
const byte pin_uson_Echo = 31;    // Broche ECHO

/* Constantes pour le timeout */
const unsigned long MEASURE_TIMEOUT = 25000UL; // 25ms = ~8m à 340m/s

/* Vitesse du son dans l'air en mm/us */
const float SOUND_SPEED = 340.0 / 1000;


// ----------------------
// ----- RELAIS ---------
// ----------------------

int relay_Lum = 8;
int relay_Pompe_[] = {9, 10, 11};

// ----------------------
// ------- MENU ---------
// ----------------------

int PinCLK = 4;
int PinDT = 5;
int PinSW = 0;
int PinCLKLast = LOW;
int n = LOW;
int buttonState = 0;         // variable for reading the pushbutton status
int mode;

int clic = 0;

int saveSelect = 0;
int select = 0;
int selectMin = 0;
int selectMax = 8;

int nb_mesure = 0;

//delai entre 2 mesures

unsigned long delaiMesure = 300;
unsigned long saveDelaiMesure;

unsigned long delaiUpdateLCD = 1;
unsigned long saveDelaiUpdateLCD;

unsigned long delaiMenu = 10;
unsigned long saveDelaiMenu;

unsigned long delaiRetroEclairage = 90;
unsigned long saveDelaiRetroEclairage;

int menu;
int saveMenu = -1;
int itemMenu;
int item_max_menu = 7;
int tmp;

// capteur luminosité
int M_Lum;
int M_Hum;
int M_Eau;

// ----------------------
// -------- LCD ---------
// ----------------------

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display


// **********************
// ******* SETUP ********
// **********************

void setup() {
  Serial.begin(9600);

  // RTC
  RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));

  // RELAIS
  pinMode(relay_Lum, OUTPUT);

  digitalWrite(relay_Lum, HIGH);

  // relais pompes
  for (int i = 0; i < 4; i++) {
    pinMode(relay_Pompe_[i], OUTPUT);
    digitalWrite(relay_Pompe_[i], HIGH);
  }

  //LCD
  lcd.init();
  lcd.begin(20, 4);
  lcd.backlight();

  //Ultra son
  /* Initialise les broches */
  pinMode(pin_uson_Trigger, OUTPUT);
  digitalWrite(pin_uson_Trigger, LOW); // La broche TRIGGER doit être à LOW au repos
  pinMode(pin_uson_Echo, INPUT);

  //Menu
  pinMode (PinCLK, INPUT);
  pinMode (PinDT, INPUT);
  pinMode (PinSW, INPUT);

}


void loop() {

  int i;
  int colonne;
  int ligne;

  gestionMenu();

  // ----------------------
  // - SORTIE MENU TEMPO --
  // ----------------------

  if (millis() > saveDelaiMenu && mode > 0 || mode == 4 )
  {
    toHome();
  }

  // ----------------------
  // -------- CLIC --------
  // ----------------------

  if (clic == 1) {

    saveDelaiMenu = (delaiMenu * 1000 ) + millis();

    // ----------------------clic
    // - VALIDATION CONFIG --
    // ----------------------

    if (mode == 2)      // on enregistgre la config
    {
      itemsMenuValue[menu] = select;
      lcd.clear();
      lcd.setCursor(4, 0);
      lcd.print("SAUVEGARDE");
      lcd.setCursor(4, 2);
      lcd.print(itemsMenu[menu]);
      lcd.print(" = ");
      lcd.print(itemsMenuValue[menu]);
      delay(1800);
      itemSondeHum[0] = itemsMenuValue[2];
      itemSondeHum[1] = itemsMenuValue[3];
      itemSondeHum[2] = itemsMenuValue[4];

      mode = 4;
    }

    // ----------------------clic
    // ---- EDIT  CONFIG ----
    // ----------------------

    if (mode == 1)
    {
      select = itemsMenuValue[menu];
      selectMin = itemsMenuMin[menu];
      selectMax = itemsMenuMax[menu];
      lcd.clear();
      mode = 2;
    }

    // ----------------------clic
    // -- AFFICHAGE CONFIG --
    // ----------------------
    if (mode == 0)
    {
      lcd.clear();
      mode = 1;
      for ( i = 0; i <= item_max_menu / 2 ; i++) {
        lcd.setCursor(1, i);
        lcd.print(itemsMenu[i]);
        lcd.setCursor(12, i);
        lcd.print(itemsMenu[i + 4]);
      }
    }

    clic = 0;
    delay(500);
  } // FIN CLIC


  // ----------------------
  // -------- MODE --------
  // ----------------------

  if (mode == 1) {

    menu = select;

    if (menu != saveMenu )
    {

      if (saveMenu > 3)   {
        colonne = 11;
        ligne = saveMenu - 4;
      } else    {
        colonne = 0;
        ligne = saveMenu;
      }
      lcdDisplay(" ", colonne, ligne);

      if (menu > 3)   {
        colonne = 11;
        ligne = menu - 4;
      } else    {
        colonne = 0;
        ligne = menu;
      }

      lcdDisplay("#", colonne, ligne);
      saveMenu = menu;
    }
  }

  // ----------------------
  // ----- REGLAGE    -----
  // ----------------------

  if (mode == 2) {

    if (select != saveSelect || select == selectMin || select == selectMax)
    {
      lcdDisplay(String(itemsMenu[menu]), 7, 0);

      lcdDisplay("valeur  : ", 5, 1);
      lcdDisplay(String(select), 15, 1);

      lcdDisplay("Minimum : ", 5, 2);
      lcdDisplay(String(selectMin), 15, 2);
      lcdDisplay("Maximum : ", 5, 3);
      lcdDisplay(String(selectMax), 15, 3);

      saveSelect = select;
    }
  }

  // ----------------------
  // - AFFICHAGE NORMAL ---
  // ----------------------


  if (millis() > saveDelaiUpdateLCD && mode == 0) {

    now = RTC.now();

    afficher_heure();
    gestion_Led();

    saveDelaiUpdateLCD  = millis() + (delaiUpdateLCD * 1000);

    if ((saveDelaiMesure - millis()) / 1000 <= delaiMesure) {
      lcdDisplay("MES:   ", 12, 2);
      lcdDisplay(String((saveDelaiMesure - millis()) / 1000 ), 16, 2);
      lcdDisplay("s", 19, 2);
    }

    if (millis() > saveDelaiRetroEclairage ) {
      lcd.noBacklight();
    }
  }

  if (millis() > saveDelaiMesure && mode == 0) {

    saveDelaiMesure = millis() + (delaiMesure * 1000);

    M_Lum = mesure_Lum();
    M_Eau = mesure_Eau();
    M_Hum = mesure_Hum();

    nb_mesure++;
    lcdDisplay("nb mesures : ", 3, 3);
    lcdDisplay(String(nb_mesure), 17, 3);


    arrosage_auto();


  }
}


void toHome() {
  mode = 0;
  saveDelaiMesure = 0;
  select = -1;
  selectMin = 0;
  selectMax = 7;

  lcd.clear();
}

int mesure_Eau() {

  int temp;
  digitalWrite(pin_uson_Trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(pin_uson_Trigger, LOW);

  long measure = pulseIn(pin_uson_Echo, HIGH, MEASURE_TIMEOUT);
  int M_Niv_Eau = measure / 2.0 * SOUND_SPEED / 10;

  //  Serial.print(" Niveau eau en cm :  ");
  //  Serial.println(String(M_Niv_Eau));

  M_Eau = map(M_Niv_Eau, 10, 1, 0, 100);

  if (M_Eau > 100) {
    M_Eau = 100;
  }
  if (M_Eau < 0) {
    M_Eau = 0;
  }

  lcdDisplay("EAU:   ", 12, 0);
  lcdDisplay(String(M_Eau), 16, 0);
  lcdDisplay("%", 19, 0);

  return M_Eau;
}



int arrosage_auto()
{
  int val = 0;
  int i;
  int temp;

  for (i = 0; i < itemsMenuValue[1]; i++) {

    //    Serial.print(" sonde :  ");
    //    Serial.print(i);
    //    Serial.print(" val_Hum[i] :  ");
    //    Serial.print(val_Hum[i]);
    //    Serial.print("  < itemSondeHum[i] :  ");
    //    Serial.println(itemSondeHum[i]);
    if (val_Hum[i] < itemSondeHum[i])
    {
      arrosage(i); // arrosage = oui
    }
  }
}



int gestion_Led() {
  // ON
  if ((digitalRead(relay_Lum) == HIGH  && M_Lum < itemsMenuValue[7]) || (now.hour() >= 8 && digitalRead(relay_Lum) == HIGH ))  {
    digitalWrite(relay_Lum, LOW);
  }

  // OFF
  if ((digitalRead(relay_Lum) == LOW && M_Lum >= itemsMenuValue[7]) || ((now.hour() < 8 || now.hour() >= 22) && digitalRead(relay_Lum) == LOW ))  {
    digitalWrite(relay_Lum, HIGH);
  }
}



int mesure_Lum() {
  M_Lum = map(analogRead(pin_Lum), 0, 1024, 0, 100);

  lcdDisplay("LUM:", 12, 1);
  lcdDisplay(String(M_Lum), 16, 1);
  lcdDisplay("%", 19, 1);

  return M_Lum;
}



int mesure_Hum() {

  int temp;
  int i;

  for ( i = 0; i < itemsMenuValue[1]; i++) {
    temp = analogRead(pin_Hum[i]);

    val_Hum[i] = map(temp, MIN_MOISTURE_LEVEL, MAX_MOISTURE_LEVEL, 100, 0);

    lcdDisplay(MOISTURE_NAME[i], 0, i);
    lcdDisplay("%(", 5, i);
    lcdDisplay(String(val_Hum[i]), 3, i);

    lcdDisplay(String(itemSondeHum[i]), 7, i);
    lcdDisplay(")", 9, i);
  }
  return M_Hum;
}



int gestionMenu() {
  if (!(digitalRead(PinSW))) {
    if (clic == 0) {
      clic = 1;
    }
  }
  n = digitalRead(PinCLK);
  if ((PinCLKLast == LOW) && (n == HIGH)) {

    saveDelaiRetroEclairage = millis() + (delaiRetroEclairage * 1000);
    lcd.backlight();

    if (digitalRead(PinDT) == LOW) {
      select--;
    } else {
      select++;
    }
    if ( select < selectMin ) {
      select = selectMax;
    } else {
      select;
      if ( select > ( selectMax - 1 ) ) {
        select = selectMin;
      }
    }
    saveDelaiMenu = (delaiMenu * 1000) + millis();
  }
  PinCLKLast = n;
  buttonState = digitalRead(PinSW);
}



void lcdDisplay(String v, int colonne, int ligne) {
  lcd.setCursor(colonne, ligne);
  lcd.print(v);
}



int checkValue(int val, int level) {
  if (val > level) {
    return 1;
  } else {
    return 0;
  }
}


int arrosage(int relay)
{
  Serial.println("plouf");
  if (itemsMenuValue[0] == 1 && M_Eau >= itemsMenuValue[6])
  {
    Serial.println("glouglou");
    lcdDisplay("          ", 9, 3);
    lcdDisplay("*", 10, relay);
    lcdDisplay(" Arrosage ", 9, 3);
    digitalWrite(relay_Pompe_[relay], LOW);
    delay(duree_Arrosage * 1000);
    digitalWrite(relay_Pompe_[relay], HIGH);
    lcdDisplay("          ", 9, 3);
    lcdDisplay(" ", 10, relay);
  }
}


void afficher_heure() {


  lcd.setCursor(0, 3);

  if (now.day() < 10) {
    lcd.print(String(" "));
  }
  lcd.print(String(now.day(), DEC));
  lcd.print("/");
  if (now.month() < 10) {
    lcd.print(String(" "));
  }
  lcd.print(String(now.month(), DEC));
  lcd.print("/");
  lcd.print(String(now.year(), DEC));
  lcd.print(" ");
  if (now.hour() < 10) {
    lcd.print(String(" "));
  }
  lcd.print(String(now.hour(), DEC));
  lcd.print(":");
  if (now.minute() < 10) {
    lcd.print(String("0"));
  }
  lcd.print(String(now.minute(), DEC));
  lcd.print(":");
  if (now.second() < 10) {
    lcd.print(String("0"));
  }
  lcd.print(String(now.second(), DEC));
  lcd.print(" ");
}
