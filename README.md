# ESP32 紅外線偵測 → LED 通知系統

兩塊 **ESP32 CH9102** 透過 **ESP-NOW** 無線通訊（不需路由器）：
- **發送端**：HC-SR501 偵測到人體移動時送出訊號
- **接收端**：收到訊號後點亮紅色 LED

---

## 所需電子零件

| 零件 | 數量 | 說明 |
|------|------|------|
| ESP32 CH9102 開發板 | 2 | 您已具備 |
| HC-SR501 人體紅外線感測模組 | 1 | 您已具備 |
| 紅色 LED（5mm） | 1 | 您已具備 |
| 220 Ω 電阻 | 1 | 限流，保護 LED 與 GPIO |
| 杜邦線 | 若干 | 公對母、公對公 |
| 麵包板 | 1 | 建議使用，方便接線 |
| USB 傳輸線 | 2 | 供電與燒錄程式（依板子接頭選 Type-C 或 Micro-USB） |

### 選配

| 零件 | 說明 |
|------|------|
| 5V 電源供應器 | 若不想用 USB 供電，可改用穩壓 5V 模組 |
| HC-SR501 支架 | 調整偵測角度 |

---

## 接線說明

### 發送端（ESP32 + HC-SR501）

```
HC-SR501          ESP32 CH9102
────────          ────────────
VCC        ────►  5V（或 VIN，需 5V 供電）
GND        ────►  GND
OUT        ────►  GPIO 4
```

> **注意**
> - HC-SR501 建議使用 **5V** 供電，輸出訊號與 ESP32 3.3V GPIO 相容。
> - 模組上有兩顆可調電阻：**靈敏度**、**延遲時間**，可依環境微調。
> - 上電後約 **1 分鐘** 預熱，期間可能誤觸發，屬正常現象。

### 接收端（ESP32 + 紅色 LED）

```
紅色 LED 串聯 220Ω 電阻後接到 ESP32：

ESP32 GPIO 2 ──► 220Ω 電阻 ──► LED 長腳（正極 / 陽極）
LED 短腳（負極 / 陰極） ──► GND
```

> **極性**：LED 長腳為正極接電阻，短腳為負極接 GND。接反不會亮。
>
> 若暫時沒有 220Ω 電阻，可先用板載 LED（多數 ESP32 開發板 **GPIO 2** 已接內建 LED），但外接紅色 LED 較醒目。

### 電源

兩塊 ESP32 可各自用 USB 供電，**不必共地**（ESP-NOW 為無線通訊）。

---

## 軟體環境

1. 安裝 [Arduino IDE](https://www.arduino.cc/en/software) 或 PlatformIO
2. 安裝 ESP32 開發板支援：
   - Arduino IDE → **檔案 → 偏好設定 → 額外的開發板管理員網址** 加入：
     ```
     https://espressif.github.io/arduino-esp32/package_esp32_index.json
     ```
   - **工具 → 開發板 → 開發板管理員** 搜尋 **esp32** 並安裝
3. 開發板選擇：**ESP32 Dev Module**（或對應的 ESP32 CH9102 型號）
4. 兩塊板子都需燒錄程式，且建議使用相同 **WiFi 頻道**（程式內已設為頻道 1）

---

## 燒錄步驟

### 步驟 1：查詢接收端 MAC 位址

1. 將 **接收端 ESP32** 接上電腦
2. 開啟 `receiver_led/get_mac_address/get_mac_address.ino`
3. 燒錄後開啟 **序列埠監視器**（115200 baud）
4. 記下顯示的 MAC，例如：`AA:BB:CC:DD:EE:FF`

### 步驟 2：設定發送端

1. 開啟 `sender_motion/sender_motion.ino`
2. 將 `receiverMac[]` 改成步驟 1 記下的 MAC
3. 燒錄到 **發送端 ESP32**（接 HC-SR501 的那塊）

### 步驟 3：燒錄接收端

1. 開啟 `receiver_led/receiver_led.ino`
2. 燒錄到 **接收端 ESP32**（接紅色 LED 的那塊）

### 步驟 4：測試

1. 兩塊板子同時上電
2. 在 HC-SR501 前揮手或走過
3. 接收端紅色 LED 應亮約 **3 秒**（可在程式中調整）

---

## 專案結構

```
ESP32_Infrared detection/
├── README.md                          ← 本說明
├── sender_motion/
│   └── sender_motion.ino              ← 發送端（紅外線偵測）
└── receiver_led/
    ├── receiver_led.ino               ← 接收端（LED 通知）
    └── get_mac_address/
        └── get_mac_address.ino        ← 查詢 MAC 位址工具
```

---

## 常見問題

| 現象 | 可能原因 | 處理方式 |
|------|----------|----------|
| LED 不亮 | MAC 位址填錯 | 重新執行 get_mac_address 並更新發送端 |
| LED 不亮 | 兩板距離太遠或隔牆 | 拉近至約 10 m 內視距測試 |
| 一直誤觸發 | 靈敏度太高或預熱未完成 | 調低靈敏度、等待 1 分鐘預熱 |
| 發送失敗 | WiFi 頻道不一致 | 確認兩邊 `WIFI_CHANNEL` 皆為 1 |
| 序列埠看不到輸出 | 鮑率錯誤 | 設為 **115200** |

---

## 自訂參數

可在各 `.ino` 檔開頭修改：

| 參數 | 檔案 | 說明 |
|------|------|------|
| `MOTION_PIN` | sender_motion.ino | HC-SR501 輸出接腳（預設 GPIO 4） |
| `receiverMac[]` | sender_motion.ino | 接收端 MAC 位址 |
| `LED_PIN` | receiver_led.ino | LED 接腳（預設 GPIO 2） |
| `LED_ON_DURATION_MS` | receiver_led.ino | LED 亮燈時間（毫秒） |
| `WIFI_CHANNEL` | 兩邊皆有 | WiFi 頻道，兩邊必須相同 |
