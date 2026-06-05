/*
 * 接收端：ESP32 CH9102 + 紅色 LED
 *
 * 功能：收到 ESP-NOW 移動偵測訊號後，點亮紅色 LED 一段時間作為通知
 *
 * 接線：
 *   GPIO 2 → 220Ω 電阻 → LED 正極（長腳）
 *   LED 負極（短腳） → GND
 */

#include <WiFi.h>
#include <esp_now.h>

// ── 硬體接腳設定 ──────────────────────────────────────────
const int LED_PIN = 2;  // 紅色 LED 接到 GPIO 2（多數 ESP32 板載 LED 也在此腳）

// WiFi 頻道必須與發送端相同
const int WIFI_CHANNEL = 1;

// LED 收到訊號後維持亮燈的時間（毫秒），預設 3 秒
const unsigned long LED_ON_DURATION_MS = 3000;

// ── 接收的資料結構（必須與發送端一致）────────────────────
typedef struct {
  uint8_t  eventType;   // 事件類型：1 = 偵測到移動
  uint32_t timestamp;   // 發送端的時間戳記
} MotionMessage;

// ── LED 控制狀態 ──────────────────────────────────────────
bool ledIsOn = false;                  // 目前 LED 是否亮著
unsigned long ledOffTime = 0;          // LED 預計關閉的時間點

// ESP-NOW 收到資料時的回呼函式（在 WiFi 中斷上下文執行，應盡量簡短）
void onDataReceived(const uint8_t *mac, const uint8_t *data, int dataLen) {
  // 確認收到的資料長度正確
  if (dataLen != sizeof(MotionMessage)) {
    Serial.println("[ESP-NOW] 收到資料長度不符，忽略。");
    return;
  }

  // 將收到的位元組轉成 MotionMessage 結構
  MotionMessage incomingData;
  memcpy(&incomingData, data, sizeof(incomingData));

  // 只處理「移動偵測」事件（eventType == 1）
  if (incomingData.eventType == 1) {
    // 印出發送端的 MAC 與時間戳記
    Serial.print("[ESP-NOW] 收到移動訊號！來源 MAC：");
    for (int i = 0; i < 6; i++) {
      if (i > 0) Serial.print(":");
      // 以兩位十六進位格式印出 MAC 每一位
      if (mac[i] < 0x10) Serial.print("0");
      Serial.print(mac[i], HEX);
    }
    Serial.print("，時間戳記：");
    Serial.println(incomingData.timestamp);

    // 點亮 LED
    digitalWrite(LED_PIN, HIGH);
    ledIsOn = true;
    // 計算幾毫秒後要關閉 LED
    ledOffTime = millis() + LED_ON_DURATION_MS;

    Serial.print("[LED] 紅色 LED 已點亮，將於 ");
    Serial.print(LED_ON_DURATION_MS / 1000);
    Serial.println(" 秒後熄滅。");
  }
}

void setup() {
  // 啟動序列埠供除錯使用
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("=== ESP32 LED 通知接收端啟動 ===");

  // 設定 LED 腳位為輸出，初始狀態為熄滅
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // 設為 WiFi Station 模式以啟用 ESP-NOW
  WiFi.mode(WIFI_STA);

  // 印出本機 MAC（發送端需要填入此位址）
  Serial.print("本機 MAC 位址：");
  Serial.println(WiFi.macAddress());
  Serial.println("（請將此 MAC 填入 sender_motion.ino 的 receiverMac[]）");

  // 初始化 ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW 初始化失敗！");
    while (true) {
      delay(1000);
    }
  }

  // 註冊資料接收回呼函式
  esp_now_register_recv_cb(onDataReceived);

  Serial.println("接收端就緒，等待移動偵測訊號...");
}

void loop() {
  // 若 LED 正在亮燈，且已到達預定關閉時間，則熄滅 LED
  if (ledIsOn && millis() >= ledOffTime) {
    digitalWrite(LED_PIN, LOW);
    ledIsOn = false;
    Serial.println("[LED] 紅色 LED 已熄滅。");
  }

  // 短暫延遲，降低 CPU 占用
  delay(10);
}
