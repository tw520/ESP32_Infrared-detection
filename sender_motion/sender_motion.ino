/*
 * 發送端：ESP32 CH9102 + HC-SR501 人體紅外線感測模組
 *
 * 功能：偵測到人體移動時，透過 ESP-NOW 無線傳送訊號給接收端 ESP32
 *
 * 接線：
 *   HC-SR501 VCC → ESP32 5V
 *   HC-SR501 GND → ESP32 GND
 *   HC-SR501 OUT → GPIO 4
 */

#include <WiFi.h>
#include <esp_now.h>

// ── 硬體接腳設定 ──────────────────────────────────────────
const int MOTION_PIN = 4;  // HC-SR501 的 OUT 訊號腳接到 GPIO 4

// ── ESP-NOW 通訊設定 ──────────────────────────────────────
// 【重要】請改成接收端 ESP32 的 MAC 位址（先用 get_mac_address.ino 查詢）
uint8_t receiverMac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// WiFi 頻道必須與接收端相同，否則 ESP-NOW 無法通訊
const int WIFI_CHANNEL = 1;

// ── 傳送的資料結構（發送端與接收端必須一致）──────────────
typedef struct {
  uint8_t  eventType;       // 事件類型：1 = 偵測到移動
  uint32_t timestamp;       // 發送當下的毫秒時間戳記
} MotionMessage;

MotionMessage outgoingData;  // 準備送出的訊息內容

// ── 防重複發送狀態 ────────────────────────────────────────
bool lastMotionState = false;  // 記錄上一次 HC-SR501 的輸出狀態

// ESP-NOW 發送完成後的回呼函式
void onDataSent(const uint8_t *mac, esp_now_send_status_t status) {
  // 在序列埠印出發送結果，方便除錯
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("[ESP-NOW] 訊號發送成功！");
  } else {
    Serial.println("[ESP-NOW] 訊號發送失敗，請檢查 MAC 與距離。");
  }
}

void setup() {
  // 啟動序列埠，鮑率 115200，用於除錯輸出
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("=== ESP32 紅外線偵測發送端啟動 ===");

  // 設定 HC-SR501 輸出腳為輸入模式（讀取高低電位）
  pinMode(MOTION_PIN, INPUT);

  // 將 ESP32 設為 WiFi Station 模式（ESP-NOW 需要 WiFi 射頻開啟）
  WiFi.mode(WIFI_STA);

  // 印出本機 MAC 位址，供參考
  Serial.print("本機 MAC 位址：");
  Serial.println(WiFi.macAddress());

  // 初始化 ESP-NOW 通訊協定
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW 初始化失敗！");
    // 初始化失敗則停止程式，避免誤判
    while (true) {
      delay(1000);
    }
  }

  // 註冊發送完成的回呼函式
  esp_now_register_send_cb(onDataSent);

  // 將接收端 MAC 加入 ESP-NOW 對等節點清單
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMac, 6);  // 複製接收端 MAC
  peerInfo.channel = WIFI_CHANNEL;              // 指定 WiFi 頻道
  peerInfo.encrypt = false;                     // 不加密（簡化設定）

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("加入接收端節點失敗，請確認 MAC 位址格式。");
    while (true) {
      delay(1000);
    }
  }

  Serial.println("發送端就緒，等待 HC-SR501 偵測人體移動...");
  Serial.println("（HC-SR501 上電後約需 1 分鐘預熱）");
}

// 透過 ESP-NOW 發送移動偵測訊號
void sendMotionAlert() {
  // 填入訊息內容
  outgoingData.eventType = 1;                    // 1 代表「偵測到移動」
  outgoingData.timestamp = millis();               // 記錄發送時間

  // 發送資料到接收端 MAC
  esp_err_t result = esp_now_send(receiverMac,
                                  (uint8_t *)&outgoingData,
                                  sizeof(outgoingData));

  if (result != ESP_OK) {
    Serial.println("[ESP-NOW] 發送函式呼叫失敗。");
  }
}

void loop() {
  // 讀取 HC-SR501 輸出：HIGH = 偵測到移動，LOW = 無移動
  bool currentMotionState = (digitalRead(MOTION_PIN) == HIGH);

  // 偵測「由 LOW 變 HIGH」的上升邊緣，代表新的移動事件
  // 這樣可避免 HC-SR501 持續輸出 HIGH 期間重複發送
  if (currentMotionState && !lastMotionState) {
    Serial.println("[HC-SR501] 偵測到人體移動！正在發送訊號...");
    sendMotionAlert();
  }

  // 更新上一次的狀態，供下次比對
  lastMotionState = currentMotionState;

  // 每 50 毫秒檢查一次，反應夠快且不致過度占用 CPU
  delay(50);
}
