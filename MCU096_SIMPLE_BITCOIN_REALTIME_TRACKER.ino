#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <U8g2lib.h>
#include <ArduinoJson.h>

// Reemplaza con tus credenciales de red
const char* ssid = "XXXXXXXXXXXXXXXX";
const char* password = "XXXXXXXXXXXX";

// URL de la API de CoinGecko para obtener el precio de Bitcoin en tiempo real
const char* bitcoinPriceURL = "https://api.coingecko.com/api/v3/simple/price?ids=bitcoin&vs_currencies=usd";

// Configuración de la pantalla OLED
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 12, /* data=*/ 14);

WiFiClientSecure client;

void setup() {
  Serial.begin(115200);
  Serial.println("Inicializando...");

  // Inicializar la pantalla OLED
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.sendBuffer();

  // Conectar a Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Conectado a Wi-Fi");

  // Obtener y mostrar el precio de Bitcoin
  String btcPrice = fetchBitcoinPrice();
  Serial.printf("Precio de Bitcoin: %s\n", btcPrice.c_str());

  // Mostrar el precio en la pantalla OLED
  displayPrice(btcPrice);
}

void loop() {
  // Obtener y mostrar el precio de Bitcoin
  String btcPrice = fetchBitcoinPrice();
  Serial.printf("Precio de Bitcoin: %s\n", btcPrice.c_str());

  // Mostrar el precio en la pantalla OLED
  displayPrice(btcPrice);

  delay(60000); // Actualizar cada 60 segundos
}

String fetchBitcoinPrice() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    client.setInsecure(); // Deshabilitar la verificación del certificado SSL (no recomendado para producción)
    http.begin(client, bitcoinPriceURL); // Usar la URL de la API de CoinGecko

    int httpCode = http.GET();
    Serial.printf("Código de respuesta HTTP: %d\n", httpCode);

    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println("Respuesta HTTP:");
      Serial.println(payload);

      // Analizar JSON para extraer el precio de Bitcoin
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, payload);

      if (error) {
        Serial.print("Error al analizar JSON: ");
        Serial.println(error.c_str());
        return "Error";
      }

      float btcPrice = doc["bitcoin"]["usd"];
      Serial.printf("Precio de BTC: %.2f\n", btcPrice);
      return String(btcPrice, 2); // Devuelve el precio como String con 2 decimales
    } else {
      Serial.printf("Error en la solicitud HTTP, código: %d\n", httpCode);
      return "Error";
    }

    http.end();
  } else {
    Serial.println("WiFi no conectado");
    return "No WiFi";
  }
}

void displayPrice(String price) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_commodore64_tr); // Elegir una fuente adecuada
  u8g2.drawStr(0, 14, "Precio BTC:");
  u8g2.setFont(u8g2_font_callite24_tr);
  u8g2.drawStr(0, 44, price.c_str());
  u8g2.drawStr(115, 44, "$");
  u8g2.sendBuffer();
}