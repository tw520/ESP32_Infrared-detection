/*
 * 工具程式：查詢 ESP32 的 MAC 位址
 *
 * 用途：燒錄到「接收端」ESP32 後，從序列埠監視器讀取 MAC，
 *       再填入 sender_motion.ino 的 receiverMac[] 陣列。
 *
 * 使用完畢後，請改燒錄 receiver_led.ino 正式程式。
 */

#include <WiFi.h>

void setup() {
  // 啟動序列埠
  Serial.begin(115200);
  delay(1000);

  // 設為 Station 模式以取得 WiFi MAC
  WiFi.mode(WIFI_STA);

  Serial.println();
  Serial.println("========================================");
  Serial.println("  ESP32 MAC 位址查詢工具");
  Serial.println("========================================");
  Serial.println();

  // 印出可讀格式的 MAC（例如 AA:BB:CC:DD:EE:FF）
  Serial.print("MAC 位址（可讀格式）：");
  Serial.println(WiFi.macAddress());

  Serial.println();
  Serial.println("請將下方陣列複製到 sender_motion.ino 的 receiverMac[]：");
  Serial.println();

  // 取得 MAC 的 6 個位元組
  uint8_t mac[6];
  WiFi.macAddress(mac);

  // 以程式碼格式印出，方便直接複製貼上
  Serial.print("uint8_t receiverMac[] = {");
  for (int i = 0; i < 6; i++) {
    Serial.print("0x");
    // 補零，確保兩位十六進位
    if (mac[i] < 0x10) Serial.print("0");
    Serial.print(mac[i], HEX);
    if (i < 5) Serial.print(", ");
  }
  Serial.println("};");

  Serial.println();
  Serial.println("複製完成後，請燒錄 receiver_led.ino 到本板。");
  Serial.println("========================================");
}

void loop() {
  // 此工具只需執行一次，迴圈保持空白
  delay(1000);
}
