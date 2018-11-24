#include <SPI.h>
#include <Ethernet.h>
#include <IRremote.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>
///
#define RECV_PIN 2
#define LED_PIN 31
#define LDR_PIN 5
#define POTENCIOMETER_PIN 4
///
#define CODE_CHECKIN 1
#define CODE_CHECKOUT 0
#define CODE_SYSTEM_ONLINE 1
#define CODE_SYSTEM_OFFLINE 0
#define SYSTEM_STATUS 0
#define SYSTEM_USER_IN_USE 1

// LCD
LiquidCrystal lcd(12, 11, 4, 5, 6, 7);

// CONFIGURAÇÕES PARA ESTABELECER CONEXÃO COM SERVIDOR
byte MAC[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
EthernetClient client;
IPAddress ip(10, 0, 0, 103);
IPAddress gateway(10, 0, 0, 1);
char server[] = "10.0.0.100";

// IR
int CODE_NUMBER_ONE = 5, CODE_NUMBER_TWO = 6, CODE_NUMBER_THREE = 7, CODE_ASTERIK = 15, CODE_HASHTAG = 16;
int MAPPED_CONTROL_CODES[17] = {
    0xFF629D, // CIMA
    0xFFC23D, // DIREITA
    0xFFA857, // BAIXO
    0xFF22DD, // ESQUERDA
    0xFF02FD, // OK
    0xFF6897, // 1
    0xFF9867, // 2
    0xFFB04F, // 3
    0xFF30CF, // 4
    0xFF18E7, // 5
    0xFF7A85, // 6
    0xFF10EF, // 7
    0xFF38C7, // 8
    0xFF5AA5, // 9
    0xFF4AB5, // 0
    0xFF42BD, // *
    0xFF52AD  // #
};

IRrecv irrecv(RECV_PIN);
decode_results results;

// CONFIGURAÇÕES
String returnOfRequisition, USER_LOGGED_CODE, USER_LOGGED_TOKEN, TEMP_USER_CODE, TEMP_USER_TOKEN;
String LIGHT_STATUS="OFF", AIR_STATUS="OFF";
bool requestConfirmed = false, brightnessConfiguration = false, temperatureConfiguration = false;
int SYSTEM_INFORMATION[2] = {0, 0}, TEMP_CODE_READ = 0, FLAG_CONTADOR = 0, MESSAGE_CODE;
int calibrationValue = 0, BRIGHTNESS_REFERENCE_VALUE = 0, TEMPERATURE_REFERENCE_VALUE = 0;
void setup()
{
    Serial.begin(9600);
    lcd.begin(16, 2);
    lcd.clear();
    // Configuração do TIMER1
    TCCR1A = 0;                //confira timer para operação normal
    TCCR1B = 0;                //limpa registrador
    TCNT1  = 0;                //zera temporizado
                                     // 65536-(16MHz/1024/1Hz) = 49911 = 0xC2F7
    OCR1A = 0x7A12;
    TCCR1B |= (1 << WGM12) | (1 << CS12) | (1 << CS10);
    irrecv.enableIRIn();
    pinMode(LED_PIN, OUTPUT);
    pinMode(RECV_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(RECV_PIN), inputTreatmentIR, RISING);
    messageBootTheSystem();
    loadReferenceValues();
    while (!Serial);
    if (Ethernet.begin(MAC) == 0)
    {
        Ethernet.begin(MAC, ip);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ERRO AO CONECTAR");
        lcd.setCursor(0, 1);
        lcd.print("COM A REDE");
    } else {
        if (!client.connect(server, 8080)) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("SERVIDOR OFFLINE");
            lcd.setCursor(0, 1);
            lcd.print("TENTE NOVAMENTE");
        } else {
            client.stop();
            messageToLogin();
        }
    }
}

ISR(TIMER1_COMPA_vect)
{
    if(FLAG_CONTADOR++ == 2){
        switch(MESSAGE_CODE){
            case 1:
                mainSystemMessageLoggedIn();
            break;
            case 2:
                messageToLogin();
            break;
        }
        TIMSK1 = 0x00;
    }
}

void messageToLogin()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ENTRE COM SUA");
    lcd.setCursor(0, 1);
    lcd.print("IDENTIFICACAO");
}

void messageBootTheSystem()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("INICIALIZANDO");
    lcd.setCursor(0, 1);
    lcd.print("SISTEMA");
    delay(2000);
}

void mainSystemMessageLoggedIn()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" SISTEMA ONLINE ");
    lcd.setCursor(0, 1);
    lcd.print(String("LUZ-"+ LIGHT_STATUS +" | AR-"+ AIR_STATUS));
}

void loadReferenceValues()
{
    BRIGHTNESS_REFERENCE_VALUE = EEPROM.read(0) * 4;
    TEMPERATURE_REFERENCE_VALUE = EEPROM.read(1) * 4;
}

void requestHttp(String CODE, String TOKEN, int METHOD_REQUEST)
{
    if (METHOD_REQUEST == CODE_CHECKIN)
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("VERIFICANDO");
        lcd.setCursor(0, 1);
        lcd.print("IDENTIFICACAO");
    }
    else
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("REGISTRANDO");
        lcd.setCursor(0, 1);
        lcd.print("SAIDA");
    }

    String postData = String("code=" + CODE + "&security_token=" + TOKEN);
    if (client.connect(server, 8080))
    {
        if (METHOD_REQUEST == CODE_CHECKIN)
            client.println("POST /api/check_in/ HTTP/1.1");
        else
            client.println("POST /api/check_out/ HTTP/1.1");
        client.println("User-Agent: Arduino/1.0");
        client.println("Connection: close");
        client.println("Content-Type: application/x-www-form-urlencoded;");
        client.print("Content-Length: ");
        client.println(postData.length());
        client.println();
        client.println(postData);
        requestConfirmed = true;
    }
    else
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("OCORREU UM ERRO");
        lcd.setCursor(0, 1);
        lcd.print("TENTE NOVAMENTE");
    }
}

void saveBrightnessAccuracy()
{
    EEPROM.write(0, calibrationValue/4);
    brightnessConfiguration = false;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("CALIBRACAO SALVA");
    lcd.setCursor(0, 1);
    lcd.print("COM SUCESSO");
    BRIGHTNESS_REFERENCE_VALUE = EEPROM.read(0) * 4;
    FLAG_CONTADOR = 0;
    MESSAGE_CODE = 1;
    TIMSK1 = 0x02;
}

void saveTemperatureAccuracy()
{
    EEPROM.write(1, calibrationValue/4);
    temperatureConfiguration = false;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("CALIBRACAO SALVA");
    lcd.setCursor(0, 1);
    lcd.print("COM SUCESSO");
    TEMPERATURE_REFERENCE_VALUE = EEPROM.read(1) * 4;
    FLAG_CONTADOR = 0;
    MESSAGE_CODE = 1;
    TIMSK1 = 0x02;
}

void inputTreatmentIR()
{
    noInterrupts();
    if (irrecv.decode(&results))
    {
        unsigned int CODE_READ = (results.value);
        bool CODE_VALID = false;
        String CODE, TOKEN;
        Serial.println(CODE_READ, HEX);
        if (SYSTEM_INFORMATION[SYSTEM_STATUS] == 0)
        {
            if (CODE_READ == MAPPED_CONTROL_CODES[CODE_NUMBER_ONE])
            {
                CODE = "123";
                TOKEN = "918237m12387da6sd876xcz765123*!SDSxasd1";
                CODE_VALID = true;
            }
            if (CODE_READ == MAPPED_CONTROL_CODES[CODE_NUMBER_TWO])
            {
                CODE = "456";
                TOKEN = "918237m12387da6sd876xcz765123*!SDSxasd2";
                CODE_VALID = true;
            }
            if (CODE_READ == MAPPED_CONTROL_CODES[CODE_NUMBER_THREE])
            {
                CODE = "789";
                TOKEN = "918237m12387da6sd876xcz765123*!SDSxasd3";
                CODE_VALID = true;
            }
            if (CODE_VALID)
            {
                TEMP_CODE_READ = CODE_READ;
                TEMP_USER_CODE = CODE;
                TEMP_USER_TOKEN = TOKEN;
                requestHttp(CODE, TOKEN, CODE_CHECKIN);
            }
        }
        else if (SYSTEM_INFORMATION[SYSTEM_STATUS] == 1)
        {
            Serial.print("Sistema em uso: ");
            Serial.println(String(USER_LOGGED_CODE + " - " + USER_LOGGED_TOKEN));
            if (CODE_READ == SYSTEM_INFORMATION[SYSTEM_USER_IN_USE])
            {
                CODE = USER_LOGGED_CODE;
                TOKEN = USER_LOGGED_TOKEN;
                requestHttp(CODE, TOKEN, CODE_CHECKOUT);
            }
            if (CODE_READ == MAPPED_CONTROL_CODES[CODE_ASTERIK])
            {
                if(!brightnessConfiguration)
                    brightnessConfiguration = true;
                else
                    saveBrightnessAccuracy();
            }
            if (CODE_READ == MAPPED_CONTROL_CODES[CODE_HASHTAG])
            {
                if(!temperatureConfiguration)
                    temperatureConfiguration = true;
                else
                    saveTemperatureAccuracy();
            }
        }
        irrecv.resume();
    }
    interrupts();
}

void enableSystem()
{
    SYSTEM_INFORMATION[SYSTEM_STATUS] = CODE_SYSTEM_ONLINE;
    SYSTEM_INFORMATION[SYSTEM_USER_IN_USE] = TEMP_CODE_READ;
    USER_LOGGED_CODE = TEMP_USER_CODE;
    USER_LOGGED_TOKEN = TEMP_USER_TOKEN;
    FLAG_CONTADOR = 0;
    MESSAGE_CODE = 1;
    TIMSK1 = 0x02;
}

void disableSystem()
{
    SYSTEM_INFORMATION[SYSTEM_STATUS] = CODE_SYSTEM_OFFLINE;
    SYSTEM_INFORMATION[SYSTEM_USER_IN_USE] = 0;
    USER_LOGGED_CODE = "";
    USER_LOGGED_TOKEN = "";
    digitalWrite(LED_PIN, LOW);
    LIGHT_STATUS="OFF";
    AIR_STATUS="OFF";
    FLAG_CONTADOR = 0;
    MESSAGE_CODE = 2;
    TIMSK1 = 0x02;
}

void loop()
{
    if (requestConfirmed)
    {
        if (client.available())
            returnOfRequisition.concat((char)client.read());
        if (!client.connected())
        {
            client.stop();
            long statusCode = getStatusCode(returnOfRequisition);
            String username = getUsername(returnOfRequisition);
            if (statusCode == 200)
            {
                if (username.length() > 0)
                { // Retornou nome do professor
                    Serial.println(String("Bem vindo " + username));
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print("BEM VINDO");
                    lcd.setCursor(0, 1);
                    lcd.print(username);
                    enableSystem();
                }
                else
                { // Retornou nada, entretanto como código 200 logo registrou a saída do professor
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print("SAIDA REGISTRADA");
                    lcd.setCursor(0, 1);
                    lcd.print("COM SUCESSO");
                    disableSystem();
                }
            }
            returnOfRequisition = "";
            requestConfirmed = false;
        }
    }
    if(brightnessConfiguration || temperatureConfiguration)
    {
        calibrationValue = analogRead(POTENCIOMETER_PIN);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(String("LEITURA: " + String(calibrationValue)));
        lcd.setCursor(0, 1);
        if(brightnessConfiguration)
            lcd.print("* - PARA SALVAR");
        else if(temperatureConfiguration)
            lcd.print("# - PARA SALVAR");
        delay(250);
    }
    if(SYSTEM_INFORMATION[SYSTEM_STATUS] == 1 && !brightnessConfiguration && !temperatureConfiguration && !requestConfirmed && !TIMSK1) {
        //float tensao = map(ldrValor, 0, 1023, 0, 5);
        int VALUE_READED = analogRead(LDR_PIN);
        if (VALUE_READED >= BRIGHTNESS_REFERENCE_VALUE && LIGHT_STATUS.equals("OFF")) {
            digitalWrite(LED_PIN, HIGH);
            LIGHT_STATUS = "ON";
            mainSystemMessageLoggedIn();
        } else if(VALUE_READED < BRIGHTNESS_REFERENCE_VALUE && LIGHT_STATUS.equals("ON")) {
            digitalWrite(LED_PIN, LOW);
            LIGHT_STATUS = "OFF";
            mainSystemMessageLoggedIn();
        }
        Serial.println(String(String(VALUE_READED) + " - " + String(BRIGHTNESS_REFERENCE_VALUE) + " - " + String(TEMPERATURE_REFERENCE_VALUE)));
        delay(1000);
    }
}

long getStatusCode(String str)
{
    String statusCode = str.substring(9, 12);
    return statusCode.toInt();
}

String removeUnnecessaryInformation(String response)
{
    int pos = response.lastIndexOf('\n') + 1, stringSize = response.length();
    return response.substring(pos, stringSize);
}

bool isReturnOfCheckoutRequest(String response)
{
    return (response.equals('0')) ? true : false;
}

String getUsernameFromInformation(String response)
{
    String username = response.substring(1, response.length() - 1);
    username.toUpperCase();
    return username;
}

String getUsername(String response)
{
    String informationReturned = removeUnnecessaryInformation(response);
    return (isReturnOfCheckoutRequest(informationReturned)) ? "" : getUsernameFromInformation(informationReturned);
}