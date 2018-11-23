#include <SPI.h>
#include <Ethernet.h>
#include <IRremote.h>
#define RECV_PIN 2
#define CODE_CHECKIN 1
#define CODE_CHECKOUT 0
#define SYSTEM_STATUS 0
#define SYSTEM_USER_IN_USE 1
//#include <LiquidCrystal.h>

// LCD
//LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

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
int CODE_NUMBER_ONE = 5, CODE_NUMBER_TWO = 6, CODE_NUMBER_THREE = 7;
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
bool requestConfirmed = false;
int SYSTEM_INFORMATION[2] = {0, 0}, TEMP_CODE_READ = 0;

void setup()
{
    //    lcd.begin(16, 2);
    Serial.begin(9600);
    irrecv.enableIRIn();
    pinMode(RECV_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(RECV_PIN), inputTreatmentIR, RISING);
    while (!Serial)
        ;
    if (Ethernet.begin(MAC) == 0)
    {
        Serial.println("Falha ao configurar Ethernet usando o DHCP");
        Ethernet.begin(MAC, ip);
    }
    else
        Serial.println("SISTEMA INICIALIZADO COM SUCESSO !");
}

void requestHttp(String CODE, String TOKEN, int METHOD_REQUEST)
{
    if (METHOD_REQUEST == CODE_CHECKIN)
        Serial.println("Verificando Identificação");
    else
        Serial.println("Registrando Saída");

    // lcd.clear();
    // lcd.setCursor(0, 0);
    // lcd.print("VERIFICANDO");
    // lcd.setCursor(0, 1);
    // lcd.print("IDENTIFICACAO");

    // lcd.clear();
    // lcd.setCursor(0, 0);
    // lcd.print("REGISTRANDO");
    // lcd.setCursor(0, 1);
    // lcd.print("SAIDA");
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
        // lcd.clear();
        // lcd.setCursor(0, 0);
        // lcd.print("OCORREU UM ERRO");
        // lcd.setCursor(0, 1);
        // lcd.print("TENTE NOVAMENTE");
    }
}

void inputTreatmentIR()
{
    if (irrecv.decode(&results))
    {
        noInterrupts();
        int CODE_READ = (results.value);
        String CODE, TOKEN;
        Serial.println(CODE_READ, HEX);
        if (SYSTEM_INFORMATION[SYSTEM_STATUS] == 0 && CODE_READ != 0xFFFFFFFF)
        {
            if (CODE_READ == MAPPED_CONTROL_CODES[CODE_NUMBER_ONE])
            {
                CODE = "123";
                TOKEN = "918237m12387da6sd876xcz765123*!SDSxasd1";
            }
            if (CODE_READ == MAPPED_CONTROL_CODES[CODE_NUMBER_TWO])
            {
                CODE = "456";
                TOKEN = "918237m12387da6sd876xcz765123*!SDSxasd2";
            }
            if (CODE_READ == MAPPED_CONTROL_CODES[CODE_NUMBER_THREE])
            {
                CODE = "789";
                TOKEN = "918237m12387da6sd876xcz765123*!SDSxasd3";
            }
            TEMP_CODE_READ = CODE_READ;
            TEMP_USER_CODE = CODE;
            TEMP_USER_TOKEN = TOKEN;
            requestHttp(CODE, TOKEN, CODE_CHECKIN);
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
                    // lcd.clear();
                    // lcd.setCursor(0, 0);
                    // lcd.print("BEM VINDO");
                    // lcd.setCursor(0, 1);
                    // lcd.print(username);
                    SYSTEM_INFORMATION[SYSTEM_STATUS] = 1;
                    SYSTEM_INFORMATION[SYSTEM_USER_IN_USE] = TEMP_CODE_READ;
                    USER_LOGGED_CODE = TEMP_USER_CODE;
                    USER_LOGGED_TOKEN = TEMP_USER_TOKEN;
                }
                else
                { // Retornou nada, entretanto como código 200 logo registrou a saída do professor
                    // lcd.clear();
                    // lcd.setCursor(0, 0);
                    // lcd.print("SAIDA REGISTRADA");
                    // lcd.setCursor(0, 1);
                    // lcd.print("COM SUCESSO");
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