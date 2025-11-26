/*
 * ============================================================================
 *    ╔═══════════════════════════════════════════════════════════════════╗
 *    ║     ALIMENTATION LINÉAIRE AUDIOPHILE "DUO PRO" - FIRMWARE V2.4.3  ║
 *    ║                    Made in France 🇫🇷                              ║
 *    ╚═══════════════════════════════════════════════════════════════════╝
 * ============================================================================
 * 
 * Version: 2.4.3
 * Date: Novembre 2025
 * 
 * CHANGELOG:
 * V2.4.0 : Architecture post-regulator adaptative
 * V2.4.1 : [FIX] validateDigipotRange calcul brut (sans constrain)
 *          [FIX] Limite courant adaptative selon V_OUT (thermique LM317)
 *          [DOC] F_SECTEUR 315mA T, C_MAIN 50V (voir circuit V2.4.1)
 * V2.4.2 : [FIX] updateEnergy() delta temps réel (bug Purist ×5)
 *          [TEST] test_digipot_conversion.cpp ajouté
 * V2.4.3 : [FIX] validateDigipotRange échec → blocage total + message OLED
 *          [DOC] PREMORTEM V3.10 (modules 9.8, 9.9, 12)
 * 
 * SPECS COURANT MAX CONTINU (limite thermique LM317):
 *   - V_OUT = 5-6V:   350mA max
 *   - V_OUT = 7-9V:   450mA max  
 *   - V_OUT = 10-15V: 500mA max
 * 
 * HARDWARE REQUIS V2.4.1+:
 *   - MCP41100 x2 sur feedback LM317
 *   - F_SECTEUR 315mA T (était 2A - CORRIGÉ)
 *   - C_MAIN 4700µF 50V (était 35V - CORRIGÉ)
 * 
 * ============================================================================
 */

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_INA219.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <EEPROM.h>

// ============================================================================
// BRANDING
// ============================================================================

#define PRODUCT_NAME    "LPS DUO PRO"
#define VERSION_STRING  "V2.4.3"
#define MADE_IN         "Made in France"

// ============================================================================
// CONFIGURATION HARDWARE
// ============================================================================

#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64
#define OLED_RESET      -1
#define OLED_ADDR_A     0x3C
#define OLED_ADDR_B     0x3D

#define YELLOW_ZONE_END   16
#define BLUE_ZONE_START   16

#define INA219_ADDR_A   0x40
#define INA219_ADDR_B   0x41

// ADC Pins
#define PIN_V_SENSE_A   A0
#define PIN_V_SENSE_B   A1
#define PIN_V_PRE_A     A2
#define PIN_V_PRE_B     A3
#define PIN_NTC         A6

// Digital Pins
#define PIN_ENC_A       2
#define PIN_ENC_B       3
#define PIN_ENC_SW      4
#define PIN_LED_GREEN   5
#define PIN_LED_RED     6
#define PIN_OUT_EN_A    7
#define PIN_OUT_EN_B    8

// SPI Digipots (NOUVEAU V2.3.0)
#define PIN_CS_DP_A     9
#define PIN_CS_DP_B     10
// MOSI = 11 (hardware SPI)
// SCK = 13 (hardware SPI)

// ============================================================================
// MULTI-LANGUE - DÉFINITIONS (NOUVEAU V2.3.0)
// ============================================================================

#define LANG_EN  0
#define LANG_FR  1
#define LANG_DE  2
#define NUM_LANGUAGES 3

// Index des strings
enum StringID {
    STR_LISTENING = 0,
    STR_STANDBY,
    STR_REST,
    STR_HIGH,
    STR_OVERVOLTAGE,
    STR_OVERCURRENT,
    STR_OVERTEMP,
    STR_BACKFEED,
    STR_PROTECTION,
    STR_FRESH,
    STR_GOOD,
    STR_WARM,
    STR_HOT,
    STR_QUALITY,
    STR_SESSION,
    STR_HEALTH,
    STR_OUTPUT,
    STR_OPTIMAL,
    STR_CORRECT,
    STR_DEGRADED,
    STR_WARMUP,
    STR_ENJOY,
    STR_IN_SESSION,
    STR_LONG_SESSION,
    STR_IDEAL_COND,
    STR_NORMAL_OP,
    STR_CHECK_VENT,
    STR_LET_COOL,
    STR_VOLTAGE_HIGH,
    STR_CURRENT_HIGH,
    STR_TEMP_CRIT,
    STR_REVERSE_CURR,
    STR_DISCONNECT,
    STR_CHECK_CABLES,
    STR_REDUCE_LOAD,
    STR_TURN_OFF,
    STR_CHECK_EQUIP,
    STR_TURN_ON,
    STR_CLICK_RETRY,
    STR_WAIT_5MIN,
    STR_AUTO_RESET,
    STR_CHECK_POLAR,
    STR_SETTING,
    STR_TURN_ADJ,
    STR_CLICK_SAVE,
    STR_LANGUAGE,
    NUM_STRINGS
};

// Strings EN
const char s00_EN[] PROGMEM = "LISTENING";
const char s01_EN[] PROGMEM = "STANDBY";
const char s02_EN[] PROGMEM = "REST";
const char s03_EN[] PROGMEM = "HIGH";
const char s04_EN[] PROGMEM = "OVERVOLTAGE";
const char s05_EN[] PROGMEM = "OVERCURRENT";
const char s06_EN[] PROGMEM = "OVERTEMP";
const char s07_EN[] PROGMEM = "BACKFEED";
const char s08_EN[] PROGMEM = "PROTECTION";
const char s09_EN[] PROGMEM = "FRESH";
const char s10_EN[] PROGMEM = "GOOD";
const char s11_EN[] PROGMEM = "WARM";
const char s12_EN[] PROGMEM = "HOT";
const char s13_EN[] PROGMEM = "QUALITY";
const char s14_EN[] PROGMEM = "SESSION";
const char s15_EN[] PROGMEM = "HEALTH";
const char s16_EN[] PROGMEM = "OUTPUT";
const char s17_EN[] PROGMEM = "OPTIMAL";
const char s18_EN[] PROGMEM = "CORRECT";
const char s19_EN[] PROGMEM = "DEGRADED";
const char s20_EN[] PROGMEM = "Warming up...";
const char s21_EN[] PROGMEM = "Enjoy!";
const char s22_EN[] PROGMEM = "Session active";
const char s23_EN[] PROGMEM = "Long session!";
const char s24_EN[] PROGMEM = "Ideal conditions";
const char s25_EN[] PROGMEM = "Normal operation";
const char s26_EN[] PROGMEM = "Check ventilation";
const char s27_EN[] PROGMEM = "Let cool down";
const char s28_EN[] PROGMEM = "Voltage too high";
const char s29_EN[] PROGMEM = "Current too high";
const char s30_EN[] PROGMEM = "Temp critical";
const char s31_EN[] PROGMEM = "Reverse current";
const char s32_EN[] PROGMEM = "Disconnect load";
const char s33_EN[] PROGMEM = "Check cables";
const char s34_EN[] PROGMEM = "Reduce load";
const char s35_EN[] PROGMEM = "Turn off device";
const char s36_EN[] PROGMEM = "Check equipment";
const char s37_EN[] PROGMEM = "Turn on again";
const char s38_EN[] PROGMEM = "Click = retry";
const char s39_EN[] PROGMEM = "Wait 5 minutes";
const char s40_EN[] PROGMEM = "Auto-reset <60C";
const char s41_EN[] PROGMEM = "Check polarity";
const char s42_EN[] PROGMEM = "SETTING";
const char s43_EN[] PROGMEM = "Turn=adjust";
const char s44_EN[] PROGMEM = "Click=save";
const char s45_EN[] PROGMEM = "LANGUAGE";

// Strings FR
const char s00_FR[] PROGMEM = "ECOUTE";
const char s01_FR[] PROGMEM = "VEILLE";
const char s02_FR[] PROGMEM = "REPOS";
const char s03_FR[] PROGMEM = "FORT";
const char s04_FR[] PROGMEM = "SURTENSION";
const char s05_FR[] PROGMEM = "SURCHARGE";
const char s06_FR[] PROGMEM = "SURCHAUFFE";
const char s07_FR[] PROGMEM = "RETOUR";
const char s08_FR[] PROGMEM = "PROTECTION";
const char s09_FR[] PROGMEM = "FRAIS";
const char s10_FR[] PROGMEM = "BON";
const char s11_FR[] PROGMEM = "TIEDE";
const char s12_FR[] PROGMEM = "CHAUD";
const char s13_FR[] PROGMEM = "QUALITE";
const char s14_FR[] PROGMEM = "SESSION";
const char s15_FR[] PROGMEM = "SANTE";
const char s16_FR[] PROGMEM = "SORTIE";
const char s17_FR[] PROGMEM = "OPTIMAL";
const char s18_FR[] PROGMEM = "CORRECT";
const char s19_FR[] PROGMEM = "DEGRADE";
const char s20_FR[] PROGMEM = "Mise en chauffe...";
const char s21_FR[] PROGMEM = "Bonne ecoute!";
const char s22_FR[] PROGMEM = "Session en cours";
const char s23_FR[] PROGMEM = "Longue session!";
const char s24_FR[] PROGMEM = "Conditions ideales";
const char s25_FR[] PROGMEM = "Fonctionnement OK";
const char s26_FR[] PROGMEM = "Verifier aeration";
const char s27_FR[] PROGMEM = "Laisser refroidir";
const char s28_FR[] PROGMEM = "Tension trop haute";
const char s29_FR[] PROGMEM = "Courant trop haut";
const char s30_FR[] PROGMEM = "Temp. critique";
const char s31_FR[] PROGMEM = "Courant inverse";
const char s32_FR[] PROGMEM = "Debrancher charge";
const char s33_FR[] PROGMEM = "Verifier cables";
const char s34_FR[] PROGMEM = "Reduire charge";
const char s35_FR[] PROGMEM = "Eteindre appareil";
const char s36_FR[] PROGMEM = "Verifier materiel";
const char s37_FR[] PROGMEM = "Rallumer";
const char s38_FR[] PROGMEM = "Clic = reessayer";
const char s39_FR[] PROGMEM = "Attendre 5 minutes";
const char s40_FR[] PROGMEM = "Auto-reset <60C";
const char s41_FR[] PROGMEM = "Verifier polarite";
const char s42_FR[] PROGMEM = "REGLAGE";
const char s43_FR[] PROGMEM = "Tourner=ajuster";
const char s44_FR[] PROGMEM = "Clic=sauver";
const char s45_FR[] PROGMEM = "LANGUE";

// Strings DE
const char s00_DE[] PROGMEM = "HOREN";
const char s01_DE[] PROGMEM = "STANDBY";
const char s02_DE[] PROGMEM = "RUHE";
const char s03_DE[] PROGMEM = "HOCH";
const char s04_DE[] PROGMEM = "UEBERSPANN";
const char s05_DE[] PROGMEM = "UEBERSTROM";
const char s06_DE[] PROGMEM = "UEBERHITZ";
const char s07_DE[] PROGMEM = "RUECKSTR";
const char s08_DE[] PROGMEM = "SCHUTZ";
const char s09_DE[] PROGMEM = "KUHL";
const char s10_DE[] PROGMEM = "GUT";
const char s11_DE[] PROGMEM = "WARM";
const char s12_DE[] PROGMEM = "HEISS";
const char s13_DE[] PROGMEM = "QUALITAT";
const char s14_DE[] PROGMEM = "SITZUNG";
const char s15_DE[] PROGMEM = "STATUS";
const char s16_DE[] PROGMEM = "AUSGANG";
const char s17_DE[] PROGMEM = "OPTIMAL";
const char s18_DE[] PROGMEM = "KORREKT";
const char s19_DE[] PROGMEM = "SCHLECHT";
const char s20_DE[] PROGMEM = "Aufwarmen...";
const char s21_DE[] PROGMEM = "Viel Spass!";
const char s22_DE[] PROGMEM = "Sitzung aktiv";
const char s23_DE[] PROGMEM = "Lange Sitzung!";
const char s24_DE[] PROGMEM = "Ideale Bedingung";
const char s25_DE[] PROGMEM = "Normalbetrieb";
const char s26_DE[] PROGMEM = "Luftung prufen";
const char s27_DE[] PROGMEM = "Abkuhlen lassen";
const char s28_DE[] PROGMEM = "Spannung zu hoch";
const char s29_DE[] PROGMEM = "Strom zu hoch";
const char s30_DE[] PROGMEM = "Temp. kritisch";
const char s31_DE[] PROGMEM = "Ruckstrom";
const char s32_DE[] PROGMEM = "Last trennen";
const char s33_DE[] PROGMEM = "Kabel prufen";
const char s34_DE[] PROGMEM = "Last reduzieren";
const char s35_DE[] PROGMEM = "Gerat ausschalten";
const char s36_DE[] PROGMEM = "Gerat prufen";
const char s37_DE[] PROGMEM = "Wieder einschalten";
const char s38_DE[] PROGMEM = "Klick = Neustart";
const char s39_DE[] PROGMEM = "5 Minuten warten";
const char s40_DE[] PROGMEM = "Auto-Reset <60C";
const char s41_DE[] PROGMEM = "Polaritat prufen";
const char s42_DE[] PROGMEM = "EINSTELLUNG";
const char s43_DE[] PROGMEM = "Drehen=anpassen";
const char s44_DE[] PROGMEM = "Klick=speichern";
const char s45_DE[] PROGMEM = "SPRACHE";

// Tables de pointeurs
const char* const strings_EN[] PROGMEM = {
    s00_EN, s01_EN, s02_EN, s03_EN, s04_EN, s05_EN, s06_EN, s07_EN,
    s08_EN, s09_EN, s10_EN, s11_EN, s12_EN, s13_EN, s14_EN, s15_EN,
    s16_EN, s17_EN, s18_EN, s19_EN, s20_EN, s21_EN, s22_EN, s23_EN,
    s24_EN, s25_EN, s26_EN, s27_EN, s28_EN, s29_EN, s30_EN, s31_EN,
    s32_EN, s33_EN, s34_EN, s35_EN, s36_EN, s37_EN, s38_EN, s39_EN,
    s40_EN, s41_EN, s42_EN, s43_EN, s44_EN, s45_EN
};

const char* const strings_FR[] PROGMEM = {
    s00_FR, s01_FR, s02_FR, s03_FR, s04_FR, s05_FR, s06_FR, s07_FR,
    s08_FR, s09_FR, s10_FR, s11_FR, s12_FR, s13_FR, s14_FR, s15_FR,
    s16_FR, s17_FR, s18_FR, s19_FR, s20_FR, s21_FR, s22_FR, s23_FR,
    s24_FR, s25_FR, s26_FR, s27_FR, s28_FR, s29_FR, s30_FR, s31_FR,
    s32_FR, s33_FR, s34_FR, s35_FR, s36_FR, s37_FR, s38_FR, s39_FR,
    s40_FR, s41_FR, s42_FR, s43_FR, s44_FR, s45_FR
};

const char* const strings_DE[] PROGMEM = {
    s00_DE, s01_DE, s02_DE, s03_DE, s04_DE, s05_DE, s06_DE, s07_DE,
    s08_DE, s09_DE, s10_DE, s11_DE, s12_DE, s13_DE, s14_DE, s15_DE,
    s16_DE, s17_DE, s18_DE, s19_DE, s20_DE, s21_DE, s22_DE, s23_DE,
    s24_DE, s25_DE, s26_DE, s27_DE, s28_DE, s29_DE, s30_DE, s31_DE,
    s32_DE, s33_DE, s34_DE, s35_DE, s36_DE, s37_DE, s38_DE, s39_DE,
    s40_DE, s41_DE, s42_DE, s43_DE, s44_DE, s45_DE
};

const char* const* const all_strings[] PROGMEM = {
    strings_EN, strings_FR, strings_DE
};

// ============================================================================
// ICÔNES 8x8 EN PROGMEM
// ============================================================================

const uint8_t PROGMEM icon_volt[] = {
    0b00111100, 0b01000010, 0b10100101, 0b10100101,
    0b10100101, 0b10011001, 0b01000010, 0b00111100
};

const uint8_t PROGMEM icon_current[] = {
    0b00000000, 0b00000100, 0b00000110, 0b11111111,
    0b11111111, 0b00000110, 0b00000100, 0b00000000
};

const uint8_t PROGMEM icon_temp[] = {
    0b00001100, 0b00010010, 0b00010010, 0b00010010,
    0b00010010, 0b00100101, 0b00100101, 0b00011110
};

const uint8_t PROGMEM icon_power[] = {
    0b00001100, 0b00011000, 0b00110000, 0b01111110,
    0b00001100, 0b00011000, 0b00110000, 0b01000000
};

const uint8_t PROGMEM icon_rail_A[] = {
    0b00011000, 0b00100100, 0b01000010, 0b01000010,
    0b01111110, 0b01000010, 0b01000010, 0b00000000
};

const uint8_t PROGMEM icon_rail_B[] = {
    0b01111100, 0b01000010, 0b01000010, 0b01111100,
    0b01000010, 0b01000010, 0b01111100, 0b00000000
};

const uint8_t PROGMEM icon_energy[] = {
    0b00111100, 0b00100100, 0b11111111, 0b10000001,
    0b10111101, 0b10111101, 0b10000001, 0b11111111
};

const uint8_t PROGMEM icon_warning[] = {
    0b00001000, 0b00001000, 0b00010100, 0b00010100,
    0b00100010, 0b00101010, 0b01000001, 0b01111111
};

const uint8_t PROGMEM icon_clock[] = {
    0b00111100, 0b01000010, 0b10001001, 0b10001001,
    0b10001111, 0b10000001, 0b01000010, 0b00111100
};

const uint8_t PROGMEM icon_quality[] = {
    0b00011000, 0b00100100, 0b01000010, 0b10000001,
    0b01000010, 0b00100100, 0b00011000, 0b00000000
};

const uint8_t PROGMEM icon_settings[] = {
    0b00011000, 0b01111110, 0b11011011, 0b11111111,
    0b11111111, 0b11011011, 0b01111110, 0b00011000
};

// ============================================================================
// CONSTANTES
// ============================================================================

const float V_DIV_RATIO = 4.03f;
const float V_REF = 5.0f;
const float ADC_RESOLUTION = 1023.0f;

// Seuils protection
const float V_OUT_MAX = 16.0f;
const float V_OUT_RESET = 15.0f;
const float V_PRE_MAX = 17.5f;
const float V_PRE_RESET = 16.5f;
const float BACKFEED_THRESHOLD = -20.0f;

// Limites courant adaptatives selon V_OUT (thermique LM317) V2.4.1
// I_max = f(V_OUT) pour éviter surchauffe LM317 à basse tension
const float I_MAX_LOW_V = 350.0f;    // V_OUT 5-6V: 350mA max
const float I_MAX_MID_V = 450.0f;    // V_OUT 7-9V: 450mA max
const float I_MAX_HIGH_V = 500.0f;   // V_OUT 10-15V: 500mA max
const float I_OUT_HYSTERESIS = 30.0f; // Hystérésis reset

// Seuils température
const float TEMP_WARNING = 70.0f;
const float TEMP_SHUTDOWN = 85.0f;
const float TEMP_RESET = 60.0f;
const float TEMP_MIN_VALID = -40.0f;
const float TEMP_MAX_VALID = 150.0f;

// NTC
const float NTC_R25 = 10000.0f;
const float NTC_BETA = 3950.0f;
const float NTC_R_SERIES = 10000.0f;
const float KELVIN_OFFSET = 273.15f;

// Sweep tuning V2.4.0 - Architecture post-regulator adaptative
// MCP41100 sur feedback LM317 (wiper à 1.25V, dans specs)
const float V_OUT_MIN = 5.0f;       // Wiper = 0
const float V_OUT_MAX_SET = 15.0f;  // Wiper = 255
const uint8_t DIGIPOT_STEPS = 255;

// Constantes feedback LM317 (NOUVEAU V2.4.0)
const float R_FIXED_FB = 1100.0f;   // R_FIXED feedback (Ω)
const float R1_FB = 240.0f;         // R1 ADJ→GND (Ω)
const float R_SHUNT_FB = 2000.0f;   // R_SHUNT parallèle digipot (Ω)
const float R_DIGIPOT_FULL = 100000.0f;  // MCP41100 100kΩ
const float V_REF_LM317 = 1.25f;    // Tension référence LM317
const float V_HEADROOM = 2.0f;      // V_PRE - V_OUT constant

// Délais
const unsigned long OCP_DELAY_MS = 100;
const unsigned long OVP_DELAY_MS = 50;
const unsigned long DISPLAY_INTERVAL_MS = 200;
const unsigned long ENERGY_CALC_INTERVAL_MS = 1000;
const unsigned long PURIST_MEASURE_INTERVAL_MS = 200;
const unsigned long SETTING_TIMEOUT_MS = 10000;

// Lissage
const float SMOOTH_FACTOR = 0.3f;
const float STABILITY_SMOOTH = 0.9f;

// EEPROM
const int EEPROM_ENERGY_A = 0;
const int EEPROM_ENERGY_B = 4;
const int EEPROM_UPTIME = 8;
const int EEPROM_LANGUAGE = 12;
const int EEPROM_V_SET_A = 13;
const int EEPROM_V_SET_B = 14;

// Pages
const uint8_t NUM_PAGES = 5;

// MCP41100 SPI commands
const uint8_t MCP41_CMD_WRITE = 0x11;  // Write to potentiometer

// ============================================================================
// OBJETS
// ============================================================================

Adafruit_SSD1306 oled_A(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_SSD1306 oled_B(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_INA219 ina219_A(INA219_ADDR_A);
Adafruit_INA219 ina219_B(INA219_ADDR_B);

// ============================================================================
// VARIABLES
// ============================================================================

bool oled_A_ok = false;
bool oled_B_ok = false;
bool ina219_A_ok = false;
bool ina219_B_ok = false;

float v_out_A = 0.0f, v_out_B = 0.0f;
float i_out_A = 0.0f, i_out_B = 0.0f;
float v_pre_A = 0.0f, v_pre_B = 0.0f;
float temp_radiator = 25.0f;
bool ntc_fault = false;

float smooth_i_A = 0.0f;
float smooth_i_B = 0.0f;
float prev_i_A = 0.0f, prev_i_B = 0.0f;
float stability_score = 100.0f;

float energy_Wh_A = 0.0f;
float energy_Wh_B = 0.0f;
unsigned long uptime_seconds = 0;
unsigned long last_energy_calc = 0;
unsigned long last_eeprom_save = 0;

bool ovp_A_triggered = false, ocp_A_triggered = false;
bool ovp_B_triggered = false, ocp_B_triggered = false;
bool ovp_pre_A_triggered = false, ovp_pre_B_triggered = false;
bool otp_triggered = false;
bool backfeed_A_detected = false, backfeed_B_detected = false;
bool temp_warning_active = false;

unsigned long ocp_A_start = 0, ocp_B_start = 0;
unsigned long ovp_A_start = 0, ovp_B_start = 0;
unsigned long ovp_pre_A_start = 0, ovp_pre_B_start = 0;
unsigned long otp_start = 0;
unsigned long last_display_update = 0;
unsigned long last_purist_measure = 0;

bool purist_mode = false;

uint8_t display_page = 0;
volatile int encoder_delta = 0;

// Multi-langue (NOUVEAU V2.3.0)
uint8_t current_language = LANG_EN;

// Sweep tuning (NOUVEAU V2.3.0)
uint8_t v_set_A = 128;  // Position digipot A (défaut ~10V)
uint8_t v_set_B = 128;  // Position digipot B
bool setting_mode_A = false;
bool setting_mode_B = false;
unsigned long setting_mode_start = 0;
uint8_t temp_v_set = 0;

// Solutions guidées (NOUVEAU V2.3.0)
uint8_t fault_page = 0;
const uint8_t MAX_FAULT_PAGES = 4;

// Buffer pour getString
char str_buffer[24];

// ============================================================================
// PROTOTYPES
// ============================================================================

void initHardware();
void initI2CDevices();
void initSPI();
void readAllMeasures();
float readVoltageADC(uint8_t pin);
float readTemperatureNTC();
void checkProtections();
void updateDisplays();

// Multi-langue
const char* getString(uint8_t str_id);
void loadLanguage();
void saveLanguage();
void cycleLanguage();

// Digipots
void setDigipot(uint8_t cs_pin, uint8_t value);
void applyVoltageSettings();
void loadVoltageSettings();
void saveVoltageSettings();
float digipotToVoltage(uint8_t pos);
uint8_t voltageToDigipot(float v);

// Écrans
void displayOutputScreen(Adafruit_SSD1306 &display, char rail, float v, float i, float *smooth_i);
void displayQualityScreen(Adafruit_SSD1306 &display);
void displaySessionScreen(Adafruit_SSD1306 &display);
void displayHealthScreen(Adafruit_SSD1306 &display);
void displaySettingScreen(Adafruit_SSD1306 &display, char rail, uint8_t current_set);
void displayFaultScreen(Adafruit_SSD1306 &display, char rail, uint8_t fault_type, float value);
void displayLanguageScreen(Adafruit_SSD1306 &display);

void drawVerticalBar(Adafruit_SSD1306 &display, uint8_t x, uint8_t y, uint8_t w, uint8_t h, float percent);
void drawBargraph(Adafruit_SSD1306 &display, uint8_t x, uint8_t y, uint8_t w, uint8_t h, float percent);
void drawThermometer(Adafruit_SSD1306 &display, uint8_t x, uint8_t y, uint8_t h, float temp, float t_warn, float t_max);
void drawSlider(Adafruit_SSD1306 &display, uint8_t x, uint8_t y, uint8_t w, uint8_t pos);

void setOutputEnable(char rail, bool enable);
void setLED(bool green, bool red);
void enterPuristMode();
void exitPuristMode();
bool anyFaultActive();
uint8_t getActiveFaultType(char rail);
void enterIdleMode();
void safeDelay(unsigned long ms);
void animatedSplash();
void updateEnergy();
void updateStabilityScore();
void saveEnergyToEEPROM();
void loadEnergyFromEEPROM();
void encoderISR();
int8_t readEncoderDelta();
const char* formatUptime(unsigned long seconds);
const char* formatEnergy(float wh);

// ============================================================================
// FONCTIONS MULTI-LANGUE
// ============================================================================

const char* getString(uint8_t str_id) {
    if (str_id >= NUM_STRINGS) return "";
    
    const char* const* lang_table;
    switch (current_language) {
        case LANG_FR: lang_table = strings_FR; break;
        case LANG_DE: lang_table = strings_DE; break;
        default:      lang_table = strings_EN; break;
    }
    
    strcpy_P(str_buffer, (char*)pgm_read_ptr(&lang_table[str_id]));
    return str_buffer;
}

void loadLanguage() {
    uint8_t lang = EEPROM.read(EEPROM_LANGUAGE);
    if (lang < NUM_LANGUAGES) {
        current_language = lang;
    } else {
        current_language = LANG_EN;
    }
}

void saveLanguage() {
    EEPROM.update(EEPROM_LANGUAGE, current_language);
}

void cycleLanguage() {
    current_language = (current_language + 1) % NUM_LANGUAGES;
    saveLanguage();
}

// ============================================================================
// FONCTIONS DIGIPOTS SPI
// ============================================================================

void initSPI() {
    pinMode(PIN_CS_DP_A, OUTPUT);
    pinMode(PIN_CS_DP_B, OUTPUT);
    digitalWrite(PIN_CS_DP_A, HIGH);
    digitalWrite(PIN_CS_DP_B, HIGH);
    
    SPI.begin();
    SPI.setClockDivider(SPI_CLOCK_DIV4);  // 4MHz @ 16MHz CPU
    SPI.setDataMode(SPI_MODE0);
    SPI.setBitOrder(MSBFIRST);
}

void setDigipot(uint8_t cs_pin, uint8_t value) {
    digitalWrite(cs_pin, LOW);
    SPI.transfer(MCP41_CMD_WRITE);
    SPI.transfer(value);
    digitalWrite(cs_pin, HIGH);
}

void applyVoltageSettings() {
    setDigipot(PIN_CS_DP_A, v_set_A);
    setDigipot(PIN_CS_DP_B, v_set_B);
}

void loadVoltageSettings() {
    uint8_t val_A = EEPROM.read(EEPROM_V_SET_A);
    uint8_t val_B = EEPROM.read(EEPROM_V_SET_B);
    
    // Validation (éviter valeurs corrompues)
    v_set_A = (val_A == 0xFF) ? 128 : val_A;  // Défaut ~10V si EEPROM vierge
    v_set_B = (val_B == 0xFF) ? 128 : val_B;
}

void saveVoltageSettings() {
    EEPROM.update(EEPROM_V_SET_A, v_set_A);
    EEPROM.update(EEPROM_V_SET_B, v_set_B);
}

float digipotToVoltage(uint8_t pos) {
    // V2.4.0: Architecture post-regulator adaptative
    // MCP41100 sur feedback LM317 (wiper à 1.25V, dans specs)
    
    // R_wiper = pos × (100kΩ / 255)
    float r_wiper = pos * (R_DIGIPOT_FULL / DIGIPOT_STEPS);
    
    // R_eff = (R_wiper × R_SHUNT) / (R_wiper + R_SHUNT)
    float r_eff = 0.0f;
    if (r_wiper > 0) {
        r_eff = (r_wiper * R_SHUNT_FB) / (r_wiper + R_SHUNT_FB);
    }
    
    // R2 = R_FIXED + R_eff
    float r2 = R_FIXED_FB + r_eff;
    
    // V_PRE = 1.25 × (1 + R2/R1)
    float v_pre = V_REF_LM317 * (1.0f + r2 / R1_FB);
    
    // V_OUT = V_PRE - headroom
    float v_out = v_pre - V_HEADROOM;
    
    return constrain(v_out, V_OUT_MIN, V_OUT_MAX_SET);
}

uint8_t voltageToDigipot(float v_out) {
    // V2.4.0: Inverse de digipotToVoltage
    
    v_out = constrain(v_out, V_OUT_MIN, V_OUT_MAX_SET);
    
    // V_PRE = V_OUT + headroom
    float v_pre = v_out + V_HEADROOM;
    
    // R2 = (V_PRE / 1.25 - 1) × R1
    float r2 = (v_pre / V_REF_LM317 - 1.0f) * R1_FB;
    
    // R_eff = R2 - R_FIXED
    float r_eff = r2 - R_FIXED_FB;
    if (r_eff < 0) r_eff = 0;
    
    // R_wiper = (R_eff × R_SHUNT) / (R_SHUNT - R_eff)
    float denom = R_SHUNT_FB - r_eff;
    if (denom <= 0) return 255;
    float r_wiper = (r_eff * R_SHUNT_FB) / denom;
    
    // pos = R_wiper / (100k / 255)
    int pos = (int)(r_wiper / (R_DIGIPOT_FULL / DIGIPOT_STEPS) + 0.5f);
    
    return (uint8_t)constrain(pos, 0, 255);
}

// Headroom dynamique pour monitoring (V2.4.0)
float getActualHeadroom(char rail) {
    float v_pre = (rail == 'A') ? v_pre_A : v_pre_B;
    float v_out = (rail == 'A') ? v_out_A : v_out_B;
    return v_pre - v_out;
}

// Limite courant adaptative selon V_OUT (V2.4.1)
// Protège thermique LM317 à basse tension de sortie
float getAdaptiveCurrentLimit(float v_out_target) {
    if (v_out_target < 7.0f) {
        return I_MAX_LOW_V;   // 5-6V: 350mA max
    } else if (v_out_target < 10.0f) {
        return I_MAX_MID_V;   // 7-9V: 450mA max
    } else {
        return I_MAX_HIGH_V;  // 10-15V: 500mA max
    }
}

// Validation plage digipot au boot (V2.4.1 - calcul brut sans constrain)
bool validateDigipotRange() {
    // Wiper = 0: calcul brut
    float r_wiper_0 = 0;
    float r_eff_0 = 0;
    float r2_0 = R_FIXED_FB + r_eff_0;
    float v_pre_0 = V_REF_LM317 * (1.0f + r2_0 / R1_FB);
    float v_out_0 = v_pre_0 - V_HEADROOM;
    
    // Wiper = 255: calcul brut
    float r_wiper_255 = 255 * (R_DIGIPOT_FULL / DIGIPOT_STEPS);
    float r_eff_255 = (r_wiper_255 * R_SHUNT_FB) / (r_wiper_255 + R_SHUNT_FB);
    float r2_255 = R_FIXED_FB + r_eff_255;
    float v_pre_255 = V_REF_LM317 * (1.0f + r2_255 / R1_FB);
    float v_out_255 = v_pre_255 - V_HEADROOM;
    
    // Vérifier plage brute (tolérance ±10%)
    bool min_ok = (v_out_0 >= 4.5f) && (v_out_0 <= 5.5f);
    bool max_ok = (v_out_255 >= 14.5f) && (v_out_255 <= 16.0f);
    
    return min_ok && max_ok;
}

// ============================================================================
// INTERRUPTION ENCODEUR
// ============================================================================

void encoderISR() {
    static uint8_t last_state = 0;
    uint8_t state = (digitalRead(PIN_ENC_A) << 1) | digitalRead(PIN_ENC_B);
    const int8_t enc_table[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
    encoder_delta += enc_table[(last_state << 2) | state];
    last_state = state;
}

int8_t readEncoderDelta() {
    cli();
    int8_t delta = encoder_delta;
    encoder_delta = 0;
    sei();
    return delta;
}

// ============================================================================
// SETUP
// ============================================================================

void setup() {
    wdt_enable(WDTO_1S);
    
    initHardware();
    initSPI();
    
    Wire.begin();
    Wire.setClock(400000);
    initI2CDevices();
    
    loadEnergyFromEEPROM();
    loadLanguage();
    loadVoltageSettings();
    
    applyVoltageSettings();
    
    // V2.4.2: Validation plage digipot au boot - BLOCAGE si échec
    if (!validateDigipotRange()) {
        // Erreur calibration critique - Désactiver sorties et bloquer
        setOutputEnable('A', false);
        setOutputEnable('B', false);
        
        // Afficher erreur sur OLED si disponible
        if (oled_A_ok) {
            oled_A.clearDisplay();
            oled_A.setTextSize(1);
            oled_A.setCursor(0, 20);
            oled_A.print(F("ERREUR CALIBRATION"));
            oled_A.setCursor(0, 35);
            oled_A.print(F("Verif R_FIXED/R1/R_SHUNT"));
            oled_A.display();
        }
        
        // Boucle infinie - LED rouge clignotante
        while (1) {
            digitalWrite(PIN_LED_RED, HIGH);
            delay(200);
            wdt_reset();
            digitalWrite(PIN_LED_RED, LOW);
            delay(200);
            wdt_reset();
        }
    }
    
    attachInterrupt(digitalPinToInterrupt(PIN_ENC_A), encoderISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_ENC_B), encoderISR, CHANGE);
    
    animatedSplash();
    
    setOutputEnable('A', true);
    setOutputEnable('B', true);
    
    last_energy_calc = millis();
}

// ============================================================================
// LOOP
// ============================================================================

void loop() {
    wdt_reset();
    
    int8_t delta = readEncoderDelta();
    bool enc_click = false;
    static bool enc_was_pressed = false;
    static unsigned long enc_press_start = 0;
    
    // Détection clic encodeur
    if (digitalRead(PIN_ENC_SW) == LOW) {
        if (!enc_was_pressed) {
            enc_was_pressed = true;
            enc_press_start = millis();
        }
    } else {
        if (enc_was_pressed) {
            unsigned long press_duration = millis() - enc_press_start;
            if (press_duration < 500) {
                enc_click = true;  // Clic court
            }
            enc_was_pressed = false;
        }
    }
    
    // Appui long: Purist mode (3s) ou changement langue (5s sur page santé)
    if (enc_was_pressed && (millis() - enc_press_start > 3000)) {
        if (display_page == 4 && (millis() - enc_press_start > 5000)) {
            // Changement langue
            cycleLanguage();
            enc_was_pressed = false;
            enc_press_start = 0;
        } else if (millis() - enc_press_start > 3000 && !purist_mode) {
            enterPuristMode();
            enc_was_pressed = false;
            enc_press_start = 0;
        }
    }
    
    // MODE SETTING (réglage tension)
    if (setting_mode_A || setting_mode_B) {
        // Timeout
        if (millis() - setting_mode_start > SETTING_TIMEOUT_MS) {
            setting_mode_A = false;
            setting_mode_B = false;
        }
        
        // Ajustement avec encodeur
        if (delta != 0) {
            if (setting_mode_A) {
                int16_t new_val = (int16_t)temp_v_set + delta;
                temp_v_set = constrain(new_val, 0, 255);
            } else {
                int16_t new_val = (int16_t)temp_v_set + delta;
                temp_v_set = constrain(new_val, 0, 255);
            }
            setting_mode_start = millis();  // Reset timeout
        }
        
        // Validation avec clic
        if (enc_click) {
            if (setting_mode_A) {
                v_set_A = temp_v_set;
                setDigipot(PIN_CS_DP_A, v_set_A);
                saveVoltageSettings();
                setting_mode_A = false;
            } else {
                v_set_B = temp_v_set;
                setDigipot(PIN_CS_DP_B, v_set_B);
                saveVoltageSettings();
                setting_mode_B = false;
            }
        }
        
        // Affichage mode setting
        if (setting_mode_A && oled_A_ok) {
            displaySettingScreen(oled_A, 'A', temp_v_set);
        }
        if (setting_mode_B && oled_B_ok) {
            displaySettingScreen(oled_B, 'B', temp_v_set);
        }
        
        return;  // Pas d'autre traitement en mode setting
    }
    
    // MODE FAUTE - Navigation solutions guidées
    if (anyFaultActive()) {
        if (delta != 0) {
            if (delta > 0) {
                fault_page = (fault_page + 1) % MAX_FAULT_PAGES;
            } else {
                fault_page = (fault_page == 0) ? (MAX_FAULT_PAGES - 1) : (fault_page - 1);
            }
        }
        
        // Clic = retry (reset fautes soft)
        if (enc_click && fault_page == 3) {
            if (ocp_A_triggered || backfeed_A_detected) {
                ocp_A_triggered = false;
                backfeed_A_detected = false;
                setOutputEnable('A', true);
            }
            if (ocp_B_triggered || backfeed_B_detected) {
                ocp_B_triggered = false;
                backfeed_B_detected = false;
                setOutputEnable('B', true);
            }
            fault_page = 0;
        }
    } else {
        fault_page = 0;
        
        // Navigation pages normale
        if (delta != 0) {
            if (delta > 0) {
                display_page = (display_page + 1) % NUM_PAGES;
            } else {
                display_page = (display_page == 0) ? (NUM_PAGES - 1) : (display_page - 1);
            }
        }
        
        // Clic court sur page 0 ou 1 = mode réglage
        if (enc_click) {
            if (display_page == 0) {
                setting_mode_A = true;
                temp_v_set = v_set_A;
                setting_mode_start = millis();
            } else if (display_page == 1) {
                setting_mode_B = true;
                temp_v_set = v_set_B;
                setting_mode_start = millis();
            }
        }
    }
    
    // MODE PURIST
    if (purist_mode) {
        if (millis() - last_purist_measure >= PURIST_MEASURE_INTERVAL_MS) {
            readAllMeasures();
            checkProtections();
            updateEnergy();
            last_purist_measure = millis();
        }
        
        if (enc_click) {
            exitPuristMode();
        }
        
        enterIdleMode();
        return;
    }
    
    // MODE NORMAL
    readAllMeasures();
    checkProtections();
    updateStabilityScore();
    
    if (millis() - last_energy_calc >= ENERGY_CALC_INTERVAL_MS) {
        updateEnergy();
        last_energy_calc = millis();
    }
    
    if (millis() - last_eeprom_save >= 300000UL) {
        saveEnergyToEEPROM();
        last_eeprom_save = millis();
    }
    
    if (millis() - last_display_update >= DISPLAY_INTERVAL_MS) {
        updateDisplays();
        last_display_update = millis();
    }
    
    // LED status
    if (temp_warning_active && !anyFaultActive()) {
        static bool blink = false;
        blink = !blink;
        setLED(!blink, blink);
    } else {
        setLED(!anyFaultActive(), anyFaultActive());
    }
}

// ============================================================================
// HARDWARE INIT
// ============================================================================

void initHardware() {
    pinMode(PIN_V_SENSE_A, INPUT);
    pinMode(PIN_V_SENSE_B, INPUT);
    pinMode(PIN_V_PRE_A, INPUT);
    pinMode(PIN_V_PRE_B, INPUT);
    
    pinMode(PIN_ENC_A, INPUT_PULLUP);
    pinMode(PIN_ENC_B, INPUT_PULLUP);
    pinMode(PIN_ENC_SW, INPUT_PULLUP);
    
    pinMode(PIN_LED_GREEN, OUTPUT);
    pinMode(PIN_LED_RED, OUTPUT);
    pinMode(PIN_OUT_EN_A, OUTPUT);
    pinMode(PIN_OUT_EN_B, OUTPUT);
    
    digitalWrite(PIN_OUT_EN_A, LOW);
    digitalWrite(PIN_OUT_EN_B, LOW);
    setLED(false, false);
}

void initI2CDevices() {
    // OLED A
    if (oled_A.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR_A)) {
        oled_A_ok = true;
        oled_A.clearDisplay();
        oled_A.display();
    }
    
    // OLED B
    if (oled_B.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR_B)) {
        oled_B_ok = true;
        oled_B.clearDisplay();
        oled_B.display();
    }
    
    // INA219 A
    Wire.beginTransmission(INA219_ADDR_A);
    if (Wire.endTransmission() == 0) {
        ina219_A.begin();
        ina219_A.setCalibration_32V_1A();
        ina219_A_ok = true;
    }
    
    // INA219 B
    Wire.beginTransmission(INA219_ADDR_B);
    if (Wire.endTransmission() == 0) {
        ina219_B.begin();
        ina219_B.setCalibration_32V_1A();
        ina219_B_ok = true;
    }
}

// ============================================================================
// MESURES
// ============================================================================

void readAllMeasures() {
    v_out_A = readVoltageADC(PIN_V_SENSE_A) * V_DIV_RATIO;
    v_out_B = readVoltageADC(PIN_V_SENSE_B) * V_DIV_RATIO;
    v_pre_A = readVoltageADC(PIN_V_PRE_A) * V_DIV_RATIO;
    v_pre_B = readVoltageADC(PIN_V_PRE_B) * V_DIV_RATIO;
    
    if (ina219_A_ok) {
        float raw_i_A = ina219_A.getCurrent_mA();
        i_out_A = (raw_i_A < 0.0f) ? 0.0f : raw_i_A;
        smooth_i_A = smooth_i_A * (1.0f - SMOOTH_FACTOR) + i_out_A * SMOOTH_FACTOR;
    }
    
    if (ina219_B_ok) {
        float raw_i_B = ina219_B.getCurrent_mA();
        i_out_B = (raw_i_B < 0.0f) ? 0.0f : raw_i_B;
        smooth_i_B = smooth_i_B * (1.0f - SMOOTH_FACTOR) + i_out_B * SMOOTH_FACTOR;
    }
    
    temp_radiator = readTemperatureNTC();
}

float readVoltageADC(uint8_t pin) {
    uint32_t sum = 0;
    for (uint8_t i = 0; i < 4; i++) {
        sum += analogRead(pin);
    }
    return (sum / 4.0f) * V_REF / ADC_RESOLUTION;
}

float readTemperatureNTC() {
    float v_ntc = analogRead(PIN_NTC) * V_REF / ADC_RESOLUTION;
    
    if (v_ntc < 0.1f) {
        ntc_fault = true;
        return 150.0f;
    }
    if (v_ntc > 4.9f) {
        ntc_fault = true;
        return 150.0f;
    }
    
    float r_ntc = NTC_R_SERIES * v_ntc / (V_REF - v_ntc);
    
    if (r_ntc <= 0) {
        ntc_fault = true;
        return 150.0f;
    }
    
    float steinhart = log(r_ntc / NTC_R25) / NTC_BETA;
    steinhart += 1.0f / (25.0f + KELVIN_OFFSET);
    float temp = (1.0f / steinhart) - KELVIN_OFFSET;
    
    if (temp < TEMP_MIN_VALID || temp > TEMP_MAX_VALID) {
        ntc_fault = true;
        return 150.0f;
    }
    
    ntc_fault = false;
    return temp;
}

// ============================================================================
// PROTECTIONS
// ============================================================================

void checkProtections() {
    // OVP Rail A
    if (v_out_A > V_OUT_MAX && !ovp_A_triggered) {
        if (ovp_A_start == 0) ovp_A_start = millis();
        if (millis() - ovp_A_start > OVP_DELAY_MS) {
            ovp_A_triggered = true;
            setOutputEnable('A', false);
        }
    } else {
        ovp_A_start = 0;
    }
    
    // OVP Rail B
    if (v_out_B > V_OUT_MAX && !ovp_B_triggered) {
        if (ovp_B_start == 0) ovp_B_start = millis();
        if (millis() - ovp_B_start > OVP_DELAY_MS) {
            ovp_B_triggered = true;
            setOutputEnable('B', false);
        }
    } else {
        ovp_B_start = 0;
    }
    
    // OVP Pre A
    if (v_pre_A > V_PRE_MAX && !ovp_pre_A_triggered) {
        if (ovp_pre_A_start == 0) ovp_pre_A_start = millis();
        if (millis() - ovp_pre_A_start > OVP_DELAY_MS) {
            ovp_pre_A_triggered = true;
            setOutputEnable('A', false);
        }
    } else {
        ovp_pre_A_start = 0;
    }
    
    // OVP Pre B
    if (v_pre_B > V_PRE_MAX && !ovp_pre_B_triggered) {
        if (ovp_pre_B_start == 0) ovp_pre_B_start = millis();
        if (millis() - ovp_pre_B_start > OVP_DELAY_MS) {
            ovp_pre_B_triggered = true;
            setOutputEnable('B', false);
        }
    } else {
        ovp_pre_B_start = 0;
    }
    
    // OCP Rail A - Limite adaptative selon V_OUT (V2.4.1)
    float i_limit_A = getAdaptiveCurrentLimit(digipotToVoltage(v_set_A));
    if (ina219_A_ok && i_out_A > i_limit_A && !ocp_A_triggered) {
        if (ocp_A_start == 0) ocp_A_start = millis();
        if (millis() - ocp_A_start > OCP_DELAY_MS) {
            ocp_A_triggered = true;
            setOutputEnable('A', false);
        }
    } else {
        ocp_A_start = 0;
    }
    
    // OCP Rail B - Limite adaptative selon V_OUT (V2.4.1)
    float i_limit_B = getAdaptiveCurrentLimit(digipotToVoltage(v_set_B));
    if (ina219_B_ok && i_out_B > i_limit_B && !ocp_B_triggered) {
        if (ocp_B_start == 0) ocp_B_start = millis();
        if (millis() - ocp_B_start > OCP_DELAY_MS) {
            ocp_B_triggered = true;
            setOutputEnable('B', false);
        }
    } else {
        ocp_B_start = 0;
    }
    
    // Backfeed detection
    if (i_out_A < BACKFEED_THRESHOLD && !backfeed_A_detected) {
        backfeed_A_detected = true;
        setOutputEnable('A', false);
    }
    if (i_out_B < BACKFEED_THRESHOLD && !backfeed_B_detected) {
        backfeed_B_detected = true;
        setOutputEnable('B', false);
    }
    
    // OTP
    if (temp_radiator > TEMP_SHUTDOWN && !otp_triggered) {
        if (otp_start == 0) otp_start = millis();
        if (millis() - otp_start > OVP_DELAY_MS) {
            otp_triggered = true;
            setOutputEnable('A', false);
            setOutputEnable('B', false);
        }
    } else if (temp_radiator < TEMP_RESET && otp_triggered) {
        otp_triggered = false;
        if (!ovp_A_triggered && !ocp_A_triggered && !ovp_pre_A_triggered && !backfeed_A_detected) {
            setOutputEnable('A', true);
        }
        if (!ovp_B_triggered && !ocp_B_triggered && !ovp_pre_B_triggered && !backfeed_B_detected) {
            setOutputEnable('B', true);
        }
    } else {
        otp_start = 0;
    }
    
    // Warning température
    temp_warning_active = (temp_radiator > TEMP_WARNING && !otp_triggered);
}

bool anyFaultActive() {
    return ovp_A_triggered || ocp_A_triggered || ovp_pre_A_triggered || backfeed_A_detected ||
           ovp_B_triggered || ocp_B_triggered || ovp_pre_B_triggered || backfeed_B_detected ||
           otp_triggered;
}

// Type de faute: 0=OVP, 1=OCP, 2=OTP, 3=Backfeed, 4=OVP_PRE
uint8_t getActiveFaultType(char rail) {
    if (rail == 'A') {
        if (ovp_A_triggered) return 0;
        if (ocp_A_triggered) return 1;
        if (ovp_pre_A_triggered) return 4;
        if (backfeed_A_detected) return 3;
    } else {
        if (ovp_B_triggered) return 0;
        if (ocp_B_triggered) return 1;
        if (ovp_pre_B_triggered) return 4;
        if (backfeed_B_detected) return 3;
    }
    if (otp_triggered) return 2;
    return 255;
}

// ============================================================================
// AFFICHAGE
// ============================================================================

void updateDisplays() {
    bool fault_A = ovp_A_triggered || ocp_A_triggered || ovp_pre_A_triggered || backfeed_A_detected || otp_triggered;
    bool fault_B = ovp_B_triggered || ocp_B_triggered || ovp_pre_B_triggered || backfeed_B_detected || otp_triggered;
    
    // Écran A
    if (oled_A_ok) {
        if (fault_A) {
            uint8_t ft = getActiveFaultType('A');
            float fv = (ft == 0 || ft == 4) ? v_out_A : (ft == 1 ? i_out_A : temp_radiator);
            displayFaultScreen(oled_A, 'A', ft, fv);
        } else {
            switch (display_page) {
                case 0: displayOutputScreen(oled_A, 'A', v_out_A, i_out_A, &smooth_i_A); break;
                case 1: displayOutputScreen(oled_A, 'B', v_out_B, i_out_B, &smooth_i_B); break;
                case 2: displayQualityScreen(oled_A); break;
                case 3: displaySessionScreen(oled_A); break;
                case 4: displayHealthScreen(oled_A); break;
            }
        }
    }
    
    // Écran B (miroir ou complémentaire)
    if (oled_B_ok) {
        if (fault_B) {
            uint8_t ft = getActiveFaultType('B');
            float fv = (ft == 0 || ft == 4) ? v_out_B : (ft == 1 ? i_out_B : temp_radiator);
            displayFaultScreen(oled_B, 'B', ft, fv);
        } else {
            switch (display_page) {
                case 0: displayOutputScreen(oled_B, 'B', v_out_B, i_out_B, &smooth_i_B); break;
                case 1: displayOutputScreen(oled_B, 'A', v_out_A, i_out_A, &smooth_i_A); break;
                case 2: displayQualityScreen(oled_B); break;
                case 3: displaySessionScreen(oled_B); break;
                case 4: displayHealthScreen(oled_B); break;
            }
        }
    }
}

void displayOutputScreen(Adafruit_SSD1306 &display, char rail, float v, float i, float *smooth_i) {
    display.clearDisplay();
    
    // Header jaune
    display.drawBitmap(2, 4, (rail == 'A') ? icon_rail_A : icon_rail_B, 8, 8, SSD1306_WHITE);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(14, 4);
    display.print(getString(STR_OUTPUT));
    display.print(F(" "));
    display.print(rail);
    
    // Température en haut à droite
    display.setCursor(90, 4);
    display.print((int)temp_radiator);
    display.print((char)176);
    display.print(F("C"));
    
    // Numéro de page
    display.setCursor(118, 4);
    display.print(display_page + 1);
    
    // Tension grande (zone bleue)
    display.setTextSize(3);
    display.setCursor(10, 20);
    if (v < 10.0f) display.print(F(" "));
    display.print(v, 2);
    display.setTextSize(1);
    display.setCursor(110, 28);
    display.print(F("V"));
    
    // État écoute
    display.setTextSize(1);
    display.setCursor(0, 44);
    if (i > 400.0f) {
        display.print(F("!"));
        display.print(getString(STR_HIGH));
    } else if (i > 100.0f) {
        display.print(getString(STR_LISTENING));
    } else if (i > 20.0f) {
        display.print(getString(STR_STANDBY));
    } else if (i > 5.0f) {
        display.print(getString(STR_REST));
    } else {
        display.print(getString(STR_REST));
    }
    
    // Courant
    display.setCursor(60, 44);
    display.print((int)*smooth_i);
    display.print(F("mA"));
    
    // Barre verticale charge
    float i_percent = constrain(*smooth_i / 500.0f, 0.0f, 1.0f);
    drawVerticalBar(display, 115, 20, 10, 36, i_percent);
    
    // Hint réglage
    display.setCursor(0, 56);
    display.print(F("Clic="));
    display.print(getString(STR_SETTING));
    
    display.display();
}

void displayQualityScreen(Adafruit_SSD1306 &display) {
    display.clearDisplay();
    
    // Header
    display.drawBitmap(2, 4, icon_quality, 8, 8, SSD1306_WHITE);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(14, 4);
    display.print(getString(STR_QUALITY));
    display.setCursor(118, 4);
    display.print(F("3"));
    
    // Score grande
    float quality = stability_score;
    if (temp_radiator > TEMP_WARNING) quality -= 20.0f;
    if (smooth_i_A > 400.0f || smooth_i_B > 400.0f) quality -= 10.0f;
    quality = constrain(quality, 0.0f, 100.0f);
    
    display.setTextSize(3);
    display.setCursor(20, 22);
    display.print((int)quality);
    display.setTextSize(2);
    display.print(F("%"));
    
    // Label qualité
    display.setTextSize(1);
    display.setCursor(90, 28);
    if (quality >= 90.0f) {
        display.print(getString(STR_OPTIMAL));
    } else if (quality >= 70.0f) {
        display.print(getString(STR_GOOD));
    } else if (quality >= 50.0f) {
        display.print(getString(STR_CORRECT));
    } else {
        display.print(getString(STR_DEGRADED));
    }
    
    // Indicateurs
    display.setCursor(0, 48);
    display.print(F("Stab: "));
    display.print((int)stability_score);
    display.print(F("%"));
    
    display.setCursor(70, 48);
    display.print(F("Temp: "));
    if (temp_radiator < 45.0f) {
        display.print(getString(STR_GOOD));
    } else if (temp_radiator < TEMP_WARNING) {
        display.print(getString(STR_WARM));
    } else {
        display.print(getString(STR_HOT));
    }
    
    display.display();
}

void displaySessionScreen(Adafruit_SSD1306 &display) {
    display.clearDisplay();
    
    // Header
    display.drawBitmap(2, 4, icon_clock, 8, 8, SSD1306_WHITE);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(14, 4);
    display.print(getString(STR_SESSION));
    display.setCursor(118, 4);
    display.print(F("4"));
    
    // Temps
    display.setTextSize(2);
    display.setCursor(10, 24);
    display.print(formatUptime(uptime_seconds));
    
    // Message contextuel
    display.setTextSize(1);
    display.setCursor(0, 48);
    if (uptime_seconds < 300) {
        display.print(getString(STR_WARMUP));
    } else if (uptime_seconds < 1800) {
        display.print(getString(STR_ENJOY));
    } else if (uptime_seconds < 7200) {
        display.print(getString(STR_IN_SESSION));
    } else {
        display.print(getString(STR_LONG_SESSION));
    }
    
    // Énergie
    display.setCursor(0, 58);
    display.print(formatEnergy(energy_Wh_A + energy_Wh_B));
    display.print(F(" Wh total"));
    
    display.display();
}

void displayHealthScreen(Adafruit_SSD1306 &display) {
    display.clearDisplay();
    
    // Header
    display.drawBitmap(2, 4, icon_temp, 8, 8, SSD1306_WHITE);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(14, 4);
    display.print(getString(STR_HEALTH));
    display.setCursor(118, 4);
    display.print(F("5"));
    
    // Thermomètre graphique
    drawThermometer(display, 10, 20, 38, temp_radiator, TEMP_WARNING, TEMP_SHUTDOWN);
    
    // Température grande
    display.setTextSize(2);
    display.setCursor(50, 24);
    display.print((int)temp_radiator);
    display.print((char)176);
    display.print(F("C"));
    
    // Label température
    display.setTextSize(1);
    display.setCursor(50, 42);
    if (temp_radiator < 30.0f) {
        display.print(getString(STR_FRESH));
    } else if (temp_radiator < 45.0f) {
        display.print(getString(STR_GOOD));
    } else if (temp_radiator < TEMP_WARNING) {
        display.print(getString(STR_WARM));
    } else {
        display.print(getString(STR_HOT));
    }
    
    // Conseil
    display.setCursor(0, 52);
    if (otp_triggered) {
        display.print(getString(STR_LET_COOL));
    } else if (temp_radiator > TEMP_WARNING) {
        display.print(getString(STR_CHECK_VENT));
    } else if (temp_radiator > 45.0f) {
        display.print(getString(STR_NORMAL_OP));
    } else {
        display.print(getString(STR_IDEAL_COND));
    }
    
    // V2.4.0: Affichage headroom max (debug thermique LT3045)
    display.setCursor(85, 42);
    display.print(F("H:"));
    float h_max = max(getActualHeadroom('A'), getActualHeadroom('B'));
    display.print(h_max, 1);
    display.print(F("V"));
    
    // Hint langue
    display.setCursor(0, 62);
    display.print(F("5s="));
    display.print(getString(STR_LANGUAGE));
    
    display.display();
}

// ============================================================================
// ÉCRAN RÉGLAGE TENSION (NOUVEAU V2.3.0)
// ============================================================================

void displaySettingScreen(Adafruit_SSD1306 &display, char rail, uint8_t current_set) {
    display.clearDisplay();
    
    // Header
    display.drawBitmap(2, 4, icon_settings, 8, 8, SSD1306_WHITE);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(14, 4);
    display.print(getString(STR_SETTING));
    display.print(F(" "));
    display.print(rail);
    
    // Tension calculée
    float v_target = digipotToVoltage(current_set);
    display.setTextSize(3);
    display.setCursor(15, 20);
    if (v_target < 10.0f) display.print(F(" "));
    display.print(v_target, 2);
    display.setTextSize(1);
    display.setCursor(110, 28);
    display.print(F("V"));
    
    // Slider graphique
    drawSlider(display, 10, 46, 108, current_set);
    
    // Labels min/max
    display.setCursor(10, 56);
    display.print(F("5V"));
    display.setCursor(100, 56);
    display.print(F("15V"));
    
    // Instructions
    display.setCursor(40, 56);
    display.print(getString(STR_TURN_ADJ));
    
    display.display();
}

void drawSlider(Adafruit_SSD1306 &display, uint8_t x, uint8_t y, uint8_t w, uint8_t pos) {
    // Barre de fond
    display.drawRect(x, y, w, 6, SSD1306_WHITE);
    
    // Position curseur
    uint8_t cursor_x = x + (uint8_t)((uint32_t)pos * (w - 6) / 255);
    display.fillRect(cursor_x, y - 1, 6, 8, SSD1306_WHITE);
    
    // Marqueur 50%
    uint8_t mid_x = x + w / 2;
    display.drawPixel(mid_x, y + 7, SSD1306_WHITE);
}

// ============================================================================
// ÉCRAN FAUTE AVEC SOLUTIONS GUIDÉES (NOUVEAU V2.3.0)
// ============================================================================

void displayFaultScreen(Adafruit_SSD1306 &display, char rail, uint8_t fault_type, float value) {
    static bool invert = false;
    invert = !invert;
    
    display.clearDisplay();
    
    // Header clignotant
    if (invert) {
        display.fillRect(0, 0, 128, YELLOW_ZONE_END, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
    } else {
        display.setTextColor(SSD1306_WHITE);
    }
    
    display.drawBitmap(2, 4, icon_warning, 8, 8, invert ? SSD1306_BLACK : SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(14, 4);
    display.print(getString(STR_PROTECTION));
    
    // Page faute
    display.setCursor(110, 4);
    display.print(fault_page + 1);
    display.print(F("/4"));
    
    display.setTextColor(SSD1306_WHITE);
    
    // Contenu selon page
    if (fault_page == 0) {
        // Page 0: Diagnostic
        display.setTextSize(2);
        display.setCursor(0, 20);
        
        switch (fault_type) {
            case 0: display.print(getString(STR_OVERVOLTAGE)); break;
            case 1: display.print(getString(STR_OVERCURRENT)); break;
            case 2: display.print(getString(STR_OVERTEMP)); break;
            case 3: display.print(getString(STR_BACKFEED)); break;
            case 4: display.print(getString(STR_OVERVOLTAGE)); break;
        }
        
        display.setTextSize(1);
        display.setCursor(0, 40);
        display.print(getString(STR_OUTPUT));
        display.print(F(" "));
        display.print(rail);
        display.print(F(": "));
        
        if (fault_type == 0 || fault_type == 4) {
            display.print(value, 1);
            display.print(F("V"));
        } else if (fault_type == 1) {
            display.print((int)value);
            display.print(F("mA"));
        } else if (fault_type == 2) {
            display.print((int)value);
            display.print((char)176);
            display.print(F("C"));
        }
        
        display.setCursor(0, 56);
        display.print(F("> "));
        switch (fault_type) {
            case 0: case 4: display.print(getString(STR_VOLTAGE_HIGH)); break;
            case 1: display.print(getString(STR_CURRENT_HIGH)); break;
            case 2: display.print(getString(STR_TEMP_CRIT)); break;
            case 3: display.print(getString(STR_REVERSE_CURR)); break;
        }
        
    } else if (fault_page == 1) {
        // Page 1: Étape 1
        display.setTextSize(1);
        display.setCursor(0, 20);
        display.print(F("Etape 1:"));
        display.setTextSize(2);
        display.setCursor(0, 32);
        
        switch (fault_type) {
            case 0: case 4: display.print(getString(STR_TURN_OFF)); break;
            case 1: display.print(getString(STR_DISCONNECT)); break;
            case 2: display.print(getString(STR_WAIT_5MIN)); break;
            case 3: display.print(getString(STR_DISCONNECT)); break;
        }
        
    } else if (fault_page == 2) {
        // Page 2: Étape 2
        display.setTextSize(1);
        display.setCursor(0, 20);
        display.print(F("Etape 2:"));
        display.setTextSize(2);
        display.setCursor(0, 32);
        
        switch (fault_type) {
            case 0: case 4: display.print(getString(STR_CHECK_EQUIP)); break;
            case 1: display.print(getString(STR_CHECK_CABLES)); break;
            case 2: display.print(getString(STR_CHECK_VENT)); break;
            case 3: display.print(getString(STR_CHECK_POLAR)); break;
        }
        
    } else if (fault_page == 3) {
        // Page 3: Action finale
        display.setTextSize(1);
        display.setCursor(0, 20);
        display.print(F("Etape 3:"));
        display.setTextSize(2);
        display.setCursor(0, 32);
        
        if (fault_type == 2) {
            display.print(getString(STR_AUTO_RESET));
        } else if (fault_type == 0 || fault_type == 4) {
            display.print(getString(STR_TURN_ON));
        } else {
            display.print(getString(STR_CLICK_RETRY));
        }
    }
    
    display.display();
}

// ============================================================================
// GRAPHIQUES
// ============================================================================

void drawVerticalBar(Adafruit_SSD1306 &display, uint8_t x, uint8_t y, uint8_t w, uint8_t h, float percent) {
    display.drawRect(x, y, w, h, SSD1306_WHITE);
    
    uint8_t fill_h = (uint8_t)(percent * (h - 4));
    if (fill_h > 0) {
        uint8_t fill_y = y + h - 2 - fill_h;
        
        // Zone normale (0-70%)
        uint8_t normal_h = min(fill_h, (uint8_t)((h - 4) * 0.7f));
        display.fillRect(x + 2, y + h - 2 - normal_h, w - 4, normal_h, SSD1306_WHITE);
        
        // Zone warning (70-100%) - rayée
        if (fill_h > normal_h) {
            uint8_t warn_h = fill_h - normal_h;
            for (uint8_t i = 0; i < warn_h; i += 2) {
                display.drawFastHLine(x + 2, fill_y + i, w - 4, SSD1306_WHITE);
            }
        }
    }
}

void drawBargraph(Adafruit_SSD1306 &display, uint8_t x, uint8_t y, uint8_t w, uint8_t h, float percent) {
    display.drawRect(x, y, w, h, SSD1306_WHITE);
    
    uint8_t fill_w = (uint8_t)(percent * (w - 4));
    if (fill_w > 0) {
        uint8_t normal_w = min(fill_w, (uint8_t)((w - 4) * 0.7f));
        display.fillRect(x + 2, y + 2, normal_w, h - 4, SSD1306_WHITE);
        
        if (fill_w > normal_w) {
            uint8_t warn_w = fill_w - normal_w;
            for (uint8_t i = 0; i < warn_w; i += 2) {
                display.drawFastVLine(x + 2 + normal_w + i, y + 2, h - 4, SSD1306_WHITE);
            }
        }
    }
}

void drawThermometer(Adafruit_SSD1306 &display, uint8_t x, uint8_t y, uint8_t h, float temp, float t_warn, float t_max) {
    uint8_t w = 10;
    uint8_t bulb_r = 6;
    
    display.drawRect(x + 2, y, w - 4, h - bulb_r, SSD1306_WHITE);
    display.drawCircle(x + w/2, y + h - bulb_r, bulb_r, SSD1306_WHITE);
    
    float fill_ratio = constrain(temp / 100.0f, 0.0f, 1.0f);
    int16_t fill_h = (int16_t)(fill_ratio * (h - bulb_r - 4));
    fill_h = constrain(fill_h, (int16_t)0, (int16_t)(h - bulb_r - 4));
    
    display.fillRect(x + 4, y + h - bulb_r - fill_h - 2, w - 8, fill_h, SSD1306_WHITE);
    display.fillCircle(x + w/2, y + h - bulb_r, bulb_r - 2, SSD1306_WHITE);
    
    // Marqueurs
    int16_t y_warn = y + h - bulb_r - 2 - (int16_t)(t_warn / 100.0f * (h - bulb_r - 4));
    int16_t y_otp = y + h - bulb_r - 2 - (int16_t)(t_max / 100.0f * (h - bulb_r - 4));
    
    display.drawFastHLine(x + w + 2, y_warn, 4, SSD1306_WHITE);
    display.drawFastHLine(x + w + 2, y_otp, 4, SSD1306_WHITE);
}

// ============================================================================
// CONTRÔLE SORTIES
// ============================================================================

void setOutputEnable(char rail, bool enable) {
    if (rail == 'A') {
        digitalWrite(PIN_OUT_EN_A, enable ? HIGH : LOW);
    } else if (rail == 'B') {
        digitalWrite(PIN_OUT_EN_B, enable ? HIGH : LOW);
    }
}

void setLED(bool green, bool red) {
    digitalWrite(PIN_LED_GREEN, green ? HIGH : LOW);
    digitalWrite(PIN_LED_RED, red ? HIGH : LOW);
}

// ============================================================================
// MODE PURIST
// ============================================================================

void enterPuristMode() {
    purist_mode = true;
    last_purist_measure = millis();
    
    saveEnergyToEEPROM();
    
    if (oled_A_ok) {
        oled_A.clearDisplay();
        oled_A.setTextSize(1);
        oled_A.setCursor(0, 4);
        oled_A.print(F("MODE PURIST"));
        oled_A.setTextSize(2);
        oled_A.setCursor(20, 26);
        oled_A.print(F("SILENCE"));
        oled_A.setTextSize(1);
        oled_A.setCursor(0, 50);
        oled_A.print(F("Ecrans OFF"));
        oled_A.setCursor(0, 58);
        oled_A.print(F("Clic = reprendre"));
        oled_A.display();
        safeDelay(1200);
        oled_A.clearDisplay();
        oled_A.display();
        oled_A.ssd1306_command(SSD1306_DISPLAYOFF);
    }
    if (oled_B_ok) {
        oled_B.clearDisplay();
        oled_B.setTextSize(2);
        oled_B.setCursor(20, 26);
        oled_B.print(F("PURIST"));
        oled_B.display();
        safeDelay(1200);
        oled_B.clearDisplay();
        oled_B.display();
        oled_B.ssd1306_command(SSD1306_DISPLAYOFF);
    }
    
    setLED(false, false);
    
    power_timer1_disable();
    power_timer2_disable();
    // Note: SPI reste activé mais digipots gardent leur valeur
}

void exitPuristMode() {
    power_timer1_enable();
    power_timer2_enable();
    
    purist_mode = false;
    
    if (oled_A_ok) oled_A.ssd1306_command(SSD1306_DISPLAYON);
    if (oled_B_ok) oled_B.ssd1306_command(SSD1306_DISPLAYON);
    
    if (!ovp_A_triggered && !ocp_A_triggered && !ovp_pre_A_triggered && 
        !backfeed_A_detected && !otp_triggered) {
        setOutputEnable('A', true);
    }
    
    if (!ovp_B_triggered && !ocp_B_triggered && !ovp_pre_B_triggered && 
        !backfeed_B_detected && !otp_triggered) {
        setOutputEnable('B', true);
    }
}

// ============================================================================
// UTILITAIRES
// ============================================================================

void safeDelay(unsigned long ms) {
    while (ms > 100) {
        delay(100);
        wdt_reset();
        ms -= 100;
    }
    if (ms > 0) delay(ms);
    wdt_reset();
}

void enterIdleMode() {
    set_sleep_mode(SLEEP_MODE_IDLE);
    cli();
    sleep_enable();
    sei();
    sleep_cpu();
    sleep_disable();
}

void updateStabilityScore() {
    float delta_i_A = abs(i_out_A - prev_i_A);
    float delta_i_B = abs(i_out_B - prev_i_B);
    float instability = (delta_i_A + delta_i_B) / 10.0f;
    
    stability_score = stability_score * STABILITY_SMOOTH + 
                      (100.0f - constrain(instability, 0.0f, 100.0f)) * (1.0f - STABILITY_SMOOTH);
    
    prev_i_A = i_out_A;
    prev_i_B = i_out_B;
}

// V2.4.2: Corrigé - calcul delta temps réel (bug Purist ×5)
static unsigned long last_energy_update = 0;

void updateEnergy() {
    unsigned long now = millis();
    unsigned long delta_ms = now - last_energy_update;
    last_energy_update = now;
    
    // Éviter division par zéro au premier appel
    if (delta_ms == 0 || delta_ms > 10000) {
        delta_ms = 1000;  // Défaut 1s
    }
    
    float delta_hours = delta_ms / 3600000.0f;
    
    float p_A = v_out_A * (i_out_A / 1000.0f);  // Watts
    float p_B = v_out_B * (i_out_B / 1000.0f);
    
    energy_Wh_A += p_A * delta_hours;
    energy_Wh_B += p_B * delta_hours;
    
    // Uptime: incrémenter seulement les secondes entières
    static unsigned long uptime_accumulator_ms = 0;
    uptime_accumulator_ms += delta_ms;
    while (uptime_accumulator_ms >= 1000) {
        uptime_seconds++;
        uptime_accumulator_ms -= 1000;
    }
}

void saveEnergyToEEPROM() {
    EEPROM.put(EEPROM_ENERGY_A, energy_Wh_A);
    EEPROM.put(EEPROM_ENERGY_B, energy_Wh_B);
    EEPROM.put(EEPROM_UPTIME, uptime_seconds);
}

void loadEnergyFromEEPROM() {
    EEPROM.get(EEPROM_ENERGY_A, energy_Wh_A);
    EEPROM.get(EEPROM_ENERGY_B, energy_Wh_B);
    EEPROM.get(EEPROM_UPTIME, uptime_seconds);
    
    if (isnan(energy_Wh_A) || energy_Wh_A < 0) energy_Wh_A = 0;
    if (isnan(energy_Wh_B) || energy_Wh_B < 0) energy_Wh_B = 0;
}

static char uptime_buffer[12];
const char* formatUptime(unsigned long seconds) {
    unsigned long h = seconds / 3600;
    unsigned long m = (seconds % 3600) / 60;
    
    if (h > 99) h = 99;
    
    snprintf(uptime_buffer, sizeof(uptime_buffer), "%luh%02lum", h, m);
    return uptime_buffer;
}

static char energy_buffer[10];
const char* formatEnergy(float wh) {
    if (wh < 100.0f) {
        dtostrf(wh, 4, 1, energy_buffer);
    } else if (wh < 1000.0f) {
        dtostrf(wh, 4, 0, energy_buffer);
    } else {
        dtostrf(wh / 1000.0f, 4, 2, energy_buffer);
        strcat(energy_buffer, "k");
    }
    return energy_buffer;
}

// ============================================================================
// SPLASH
// ============================================================================

void animatedSplash() {
    if (!oled_A_ok && !oled_B_ok) {
        safeDelay(1500);
        return;
    }
    
    for (uint8_t r = 4; r <= 18; r += 2) {
        wdt_reset();
        
        if (oled_A_ok) {
            oled_A.clearDisplay();
            oled_A.drawCircle(64, 36, r, SSD1306_WHITE);
            if (r > 8) oled_A.drawCircle(64, 36, r - 4, SSD1306_WHITE);
            oled_A.display();
        }
        if (oled_B_ok) {
            oled_B.clearDisplay();
            oled_B.drawCircle(64, 36, r, SSD1306_WHITE);
            if (r > 8) oled_B.drawCircle(64, 36, r - 4, SSD1306_WHITE);
            oled_B.display();
        }
        delay(40);
    }
    
    wdt_reset();
    
    if (oled_A_ok) {
        oled_A.clearDisplay();
        oled_A.setTextSize(1);
        oled_A.setTextColor(SSD1306_WHITE);
        oled_A.setCursor(35, 4);
        oled_A.print(F("RAIL A"));
        
        oled_A.drawCircle(64, 36, 18, SSD1306_WHITE);
        oled_A.drawCircle(64, 36, 14, SSD1306_WHITE);
        oled_A.setCursor(55, 33);
        oled_A.print(F("LPS"));
        
        oled_A.setCursor(28, 56);
        oled_A.print(F("DUO PRO "));
        oled_A.print(VERSION_STRING);
        oled_A.display();
    }
    
    if (oled_B_ok) {
        oled_B.clearDisplay();
        oled_B.setTextSize(1);
        oled_B.setTextColor(SSD1306_WHITE);
        oled_B.setCursor(35, 4);
        oled_B.print(F("RAIL B"));
        
        oled_B.drawCircle(64, 36, 18, SSD1306_WHITE);
        oled_B.drawCircle(64, 36, 14, SSD1306_WHITE);
        oled_B.setCursor(55, 33);
        oled_B.print(F("LPS"));
        
        oled_B.setCursor(28, 56);
        oled_B.print(F("DUO PRO "));
        oled_B.print(VERSION_STRING);
        oled_B.display();
    }
    
    safeDelay(1500);
}

// ============================================================================
// FIN FIRMWARE V2.4.3
// ============================================================================
