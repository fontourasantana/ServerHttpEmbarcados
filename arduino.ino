#include <SPI.h>
#include <Ethernet.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

IPAddress ip(10, 0, 0, 103);
IPAddress gateway(10, 0, 0, 1);
char server[] = "10.0.0.100";
EthernetClient client;
int contador = 0, statusCount = 0, aux = 0, auxCount = 0;
char statusCode[4] = "";
char bufferAux[255] = "";
char CODE[] = "456", TOKEN[] = "918237m12387da6sd876xcz765123*!SDSxasd2";

void setup()
{
    Serial.begin(9600);
    while (!Serial)
        ;
    if (Ethernet.begin(mac) == 0)
    {
        Serial.println("Falha ao configurar Ethernet usando o DHCP");
        Ethernet.begin(mac, ip);
    }
    delay(1000);
    post();
}

void post()
{
    Serial.println("connecting...");
    String postData = String("code=" + String(CODE) + "&security_token=" + String(TOKEN));
    //Serial.println(postData);
    if (client.connect(server, 8080))
    {
        Serial.println("connected");
        client.println("POST /api/check_out/ HTTP/1.1");
        client.println("User-Agent: Arduino/1.0");
        client.println("Connection: close");
        client.println("Content-Type: application/x-www-form-urlencoded;");
        client.print("Content-Length: ");
        client.println(postData.length());
        client.println();
        client.println(postData);
    }
    else
    {
        Serial.println("connection failed");
    }
}

void loop()
{
    if (client.available())
    {
        contador++;
        char c = client.read();
        if (contador >= 10 && contador <= 12)
            statusCode[statusCount++] = c;
        if (aux == 7)
            bufferAux[auxCount++] = c;
        if (c == '\n')
            aux++;
        //Serial.print(c);
    }
    if (!client.connected())
    {
        Serial.println();
        Serial.println("Desconectando.");
        client.stop();
        statusCode[statusCount] = '\0';
        bufferAux[0] = ' ';
        bufferAux[auxCount - 1] = '\0';
        Serial.println(statusCode);
        if (strcmp("200", statusCode) == 0 && strlen(bufferAux) > 1)
            Serial.println(String("Bem vindo" + String(bufferAux)));
        while (true);
    }
}