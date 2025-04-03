#define BLYNK_TEMPLATE_ID "TMPL6Sa9VdprT"
#define BLYNK_TEMPLATE_NAME "SmartPlantPot"
#define BLYNK_AUTH_TOKEN "53H5FdD6-H4C_K7ks67E8u7g2SORAplK"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

// Định nghĩa Virtual Pins trên Blynk
#define VIRTUAL_TEMP V0
#define VIRTUAL_HUMID V1                      
#define VIRTUAL_MOIST V2                      
#define VIRTUAL_LED V3
#define VIRTUAL_PUMP V4

char ssid[32] = "";
char password[32] = "";
char auth[] = BLYNK_AUTH_TOKEN;

// Biến cảm biến và bơm
int temperature, humidity, moisture;
bool pumpState = false;
unsigned long lastPumpTime = 0;
unsigned long lastReadTime = 0;
#define PUMP_DELAY 5000  // 5 giây
#define LED_PIN 4
#define DHTPIN 5
#define DHTTYPE DHT11
#define PUMP_PIN 18
const int moisturePin = 34;
DHT dht(DHTPIN, DHTTYPE);

// Hàm nhập WiFi từ Serial Monitor
void getWiFiCredentials() {
    Serial.println("\nNhập tên WiFi: ");
    while (!Serial.available()) { }
    String ssidInput = Serial.readStringUntil('\n');
    ssidInput.trim();
    ssidInput.toCharArray(ssid, sizeof(ssid));

    Serial.println("Nhập mật khẩu WiFi: ");
    while (!Serial.available()) { }
    String passwordInput = Serial.readStringUntil('\n');
    passwordInput.trim();
    passwordInput.toCharArray(password, sizeof(password));
}

// Kết nối WiFi
void connectToWiFi() {
    Serial.print("Đang kết nối WiFi...");
    WiFi.begin(ssid, password);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✅ Kết nối WiFi thành công!");
        Serial.print("📡 IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\n❌ Kết nối WiFi thất bại. Khởi động lại ESP32...");
        WiFi.disconnect();
        delay(1000);
        ESP.restart();
    }
}

// Kiểm tra kết nối WiFi & Blynk
void checkWiFiConnection() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("⚠️ Mất kết nối WiFi! Đang thử lại...");
        WiFi.disconnect();
        WiFi.reconnect();
        int retry = 0;
        while (WiFi.status() != WL_CONNECTED && retry < 20) {
            delay(500);
            Serial.print(".");
            retry++;
        }
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\n✅ Kết nối WiFi lại thành công!");
        } else {
            Serial.println("\n❌ Không thể kết nối lại. Chờ 30 giây rồi thử lại...");
            delay(30000);
        }
    }
    if (!Blynk.connected()) {
        Serial.println("⚠️ Mất kết nối Blynk! Đang thử lại...");
        Blynk.begin(auth, ssid, password);
    }
}

// Điều khiển máy bơm
void controlPump() {
    if (moisture < 25 && !pumpState) {
        digitalWrite(PUMP_PIN, HIGH);
        pumpState = true;
        lastPumpTime = millis();
        Serial.println("🟢 Bật máy bơm - Đất rất khô!");
    } 
    else if (moisture > 65 && pumpState) {
        digitalWrite(PUMP_PIN, LOW);
        pumpState = false;
        lastPumpTime = millis();
        Serial.println("🔴 Tắt máy bơm - Đất đủ ẩm!");
    }
    Blynk.virtualWrite(VIRTUAL_PUMP, pumpState ? 1 : 0);
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    pinMode(PUMP_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    dht.begin();

    getWiFiCredentials();
    connectToWiFi();
    Blynk.begin(auth, ssid, password);
}

void loop() {
    Blynk.run();
    checkWiFiConnection();

    if (millis() - lastReadTime > 5000) {
        lastReadTime = millis();

        moisture = map(analogRead(moisturePin), 4095, 0, 0, 100);
        temperature = dht.readTemperature();
        humidity = dht.readHumidity();

        Serial.printf("🌡 Nhiệt độ: %d°C\n💧 Độ ẩm không khí: %d%%\n🌱 Độ ẩm đất: %d%%\n", 
                      temperature, humidity, moisture);

        controlPump();
        Blynk.virtualWrite(VIRTUAL_TEMP, temperature);
        Blynk.virtualWrite(VIRTUAL_HUMID, humidity);
        Blynk.virtualWrite(VIRTUAL_MOIST, moisture);
    }
}
