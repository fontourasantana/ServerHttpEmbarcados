#include <SPI.h>
#include <Ethernet.h>
#include <IRremote.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

IPAddress ip(10, 0, 0, 103);
IPAddress gateway(10, 0, 0, 1);
char server[] = "10.0.0.100";
EthernetClient client;
int contador = 0, statusCount = 0, aux = 0, auxCount = 0;
char statusCode[4] = "";
char bufferAux[255] = "";
char CODE[3], TOKEN[40];

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
int MAPPED_CONTROL_CODES[17] = {0xFF629D, // CIMA
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
                                0xFF52AD // #
                                };
int RECV_PIN = 2;
IRrecv irrecv(RECV_PIN);  
decode_results results;

int flag=0, SYSTEM_INFO[2] = {0, 0}, TEMP_CODE_READ=0;
void setup() {
    Serial.begin(9600);
    irrecv.enableIRIn();
    pinMode(RECV_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(RECV_PIN), function, RISING);
    while (!Serial);
    if (Ethernet.begin(mac) == 0) {
        Serial.println("Falha ao configurar Ethernet usando o DHCP");
        Ethernet.begin(mac, ip);
    } else
        Serial.println("Iniciado");
}

void post(int METHOD) {
    Serial.println("connecting...");
    String postData = String("code=" + String(CODE) + "&security_token=" + String(TOKEN));
    //Serial.println(postData);
    if (client.connect(server, 8080)) {
        Serial.println("connected");
        if(METHOD == 1) client.println("POST /api/check_in/ HTTP/1.1"); else client.println("POST /api/check_out/ HTTP/1.1");
        client.println("User-Agent: Arduino/1.0");
        client.println("Connection: close");
        client.println("Content-Type: application/x-www-form-urlencoded;");
        client.print("Content-Length: ");
        client.println(postData.length());
        client.println();
        client.println(postData);
    } else {
        Serial.println("connection failed");
    }
    flag=1;
    contador = 0; statusCount = 0; aux = 0; auxCount = 0;
}

void function() {
  if (irrecv.decode(&results)){
        //Serial.print("Valor lido : ");  
        //Serial.println(results.value, HEX);
        int CODE_READ = (results.value);
        Serial.println(CODE_READ, HEX);
        if(SYSTEM_INFO[0] == 0){
            if (CODE_READ == MAPPED_CONTROL_CODES[CODE_NUMBER_ONE]){
              Serial.println("Tecla 1 Pressionada");
              strcpy(CODE, "123");
              strcpy(TOKEN, "918237m12387da6sd876xcz765123*!SDSxasd1");
              post(1);
            }
            if (CODE_READ == MAPPED_CONTROL_CODES[CODE_NUMBER_TWO])
            {  
              Serial.println("Tecla 2 Pressionada");
              strcpy(CODE, "456");
              strcpy(TOKEN, "918237m12387da6sd876xcz765123*!SDSxasd2");
              post(1);
            }  
            if (CODE_READ == MAPPED_CONTROL_CODES[CODE_NUMBER_THREE])
            {
              Serial.println("Tecla 3 Pressionada");
              strcpy(CODE, "789");
              strcpy(TOKEN, "918237m12387da6sd876xcz765123*!SDSxasd3");
              post(1);
            }
            TEMP_CODE_READ = CODE_READ;
        } else {
            Serial.print("Sistema em uso, pelo código: ");
            Serial.println(SYSTEM_INFO[1], HEX);
            if (CODE_READ == SYSTEM_INFO[1]){
                strcpy(CODE, "123");
                strcpy(TOKEN, "918237m12387da6sd876xcz765123*!SDSxasd1");
                post(0);
            }
        }
        irrecv.resume(); //Le o próximo valor
  }
}

void loop(){
    if(flag){
        if (client.available()) {
            contador++;
            char c = client.read();
            if(contador >= 10 && contador <= 12) statusCode[statusCount++] = c;
            if(aux == 7) bufferAux[auxCount++] = c;
            if(c == '\n') aux++;
        }
        if (!client.connected()) {
            Serial.println();
            Serial.println("Desconectando.");
            client.stop();
            statusCode[statusCount] = '\0';
            bufferAux[0] = ' ';
            bufferAux[auxCount-1] = '\0';
            Serial.println(statusCode);
            if(strcmp("200", statusCode) == 0){
                if(strlen(bufferAux) > 1){ // Retornou nome do professor
                    Serial.println(String("Bem vindo" + String(bufferAux)));
                    SYSTEM_INFO[0] = 1;
                    SYSTEM_INFO[1] = TEMP_CODE_READ;
                } else { // Retornou nada, entretanto como código 200 logo registrou a saída do professor
                    SYSTEM_INFO[0] = 0;
                    SYSTEM_INFO[1] = 0;
                }
            }
            flag=0;
        }
    }
}