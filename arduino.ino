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
#define SYSTEM_STATUS 0
#define SYSTEM_USER_IN_USE 1

// LCD
LiquidCrystal lcd(12, 11, 4, 5, 6, 7);

/////
byte MAC[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
EthernetClient client;
IPAddress ip(10, 0, 0, 103);
IPAddress gateway(10, 0, 0, 1);
char server[] = "10.0.0.100";
///
//char CODE[3], TOKEN[40];

// CIMA - FF629D
// DIREITA - FFC23D
// BAIXO - FFA857
// ESQUERDA - FF22DD
// OK - FF02FD
// 1 - FF6897
// 2 - FF9867
// 3 - FFB04F
// 4 - FF30CF
// 5 - FF18E7
// 6 - FF7A85
// 7 - FF10EF
// 8 - FF38C7
// 9 - FF5AA5
// 0 - FF4AB5
// * - FF42BD
// # - FF52AD

// IR
int CODE_NUMBER_ONE = 5, CODE_NUMBER_TWO = 6, CODE_NUMBER_THREE = 7, CODE_ASTERIK = 15;
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
bool requestConfirmed = false, brightnessConfiguration = false;
int SYSTEM_INFORMATION[2] = {0, 0}, TEMP_CODE_READ = 0;
int calibrationValue = 0, BRIGHTNESS_VALUE_REFFER = 0;
void setup()
{
    Serial.begin(9600);
    lcd.begin(16, 2);
    irrecv.enableIRIn();
    pinMode(LED_PIN, OUTPUT);
    pinMode(RECV_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(RECV_PIN), inputTreatmentIR, RISING);
    messageBootTheSystem();
    loadReferenceValues();
    while (!Serial);
    if (Ethernet.begin(MAC) == 0)
    {
        Serial.println("Falha ao configurar Ethernet usando o DHCP");
        Ethernet.begin(MAC, ip);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ERRO AO CONECTAR");
        lcd.setCursor(0, 1);
        lcd.print("COM O SERVIDOR");
    }
    else
    {
        Serial.println("SISTEMA INICIALIZADO COM SUCESSO !");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ENTRE COM SUA");
        lcd.setCursor(0, 1);
        lcd.print("IDENTIFICACAO");
    }
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

void loadReferenceValues()
{
    BRIGHTNESS_VALUE_REFFER = EEPROM.read(0) * 4;
}

void requestHttp(String CODE, String TOKEN, int METHOD_REQUEST)
{
    if (METHOD_REQUEST == CODE_CHECKIN)
    {
        Serial.println("Verificando Identificação");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("VERIFICANDO");
        lcd.setCursor(0, 1);
        lcd.print("IDENTIFICACAO");
    }
    else
    {
        Serial.println("Registrando Saída");
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
        Serial.println("Erro na verificação da identificação");
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
    BRIGHTNESS_VALUE_REFFER = EEPROM.read(0) * 4;
}

void inputTreatmentIR()
{
    if (irrecv.decode(&results))
    {
        noInterrupts();
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
        }
        irrecv.resume();
        interrupts();
    }
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
                    SYSTEM_INFORMATION[SYSTEM_STATUS] = 1;
                    SYSTEM_INFORMATION[SYSTEM_USER_IN_USE] = TEMP_CODE_READ;
                    USER_LOGGED_CODE = TEMP_USER_CODE;
                    USER_LOGGED_TOKEN = TEMP_USER_TOKEN;
                }
                else
                { // Retornou nada, entretanto como código 200 logo registrou a saída do professor
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print("SAIDA REGISTRADA");
                    lcd.setCursor(0, 1);
                    lcd.print("COM SUCESSO");
                    Serial.println("Saída Registrada Com Sucesso");
                    SYSTEM_INFORMATION[SYSTEM_STATUS] = 0;
                    SYSTEM_INFORMATION[SYSTEM_USER_IN_USE] = 0;
                    USER_LOGGED_CODE = "";
                    USER_LOGGED_TOKEN = "";
                }
            }
            returnOfRequisition = "";
            requestConfirmed = false;
        }
    }
    if(brightnessConfiguration)
    {
        calibrationValue = analogRead(POTENCIOMETER_PIN);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(String("LEITURA: " + String(calibrationValue)));
        lcd.setCursor(0, 1);
        lcd.print("* - PARA SALVAR");
        delay(500);
    }
    if(!brightnessConfiguration && !requestConfirmed) {
        //float tensao = map(ldrValor, 0, 1023, 0, 5);
        int VALUE_READED = analogRead(LDR_PIN);
        if (VALUE_READED >= BRIGHTNESS_VALUE_REFFER) digitalWrite(LED_PIN, HIGH); else digitalWrite(LED_PIN, LOW);
        Serial.println(String(String(VALUE_READED) + " - " + String(BRIGHTNESS_VALUE_REFFER)));
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