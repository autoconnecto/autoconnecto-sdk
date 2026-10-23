// =============================================================
// Claim_https — HTTPS claim-code handoff (board onboarding)
//
// Replaces examples/esp32-claim (plain http, not board-verified).
// 1. Mint a claim code in the platform.
// 2. Flash this sketch with CLAIM_CODE in secrets.h.
// 3. Someone claims the code in the UI.
// 4. Device polls HTTPS handoff until it receives deviceToken,
//    stores it in NVS (Preferences), then you can switch to MQTT.
//
// POST https://api.autoconnecto.in/api/v1/device-claims/handoff
// Body: {"code":"acc_..."}
// =============================================================

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <AutoconnectoIsrgRoots.h>
#include "secrets.h"

Preferences prefs;

bool handoffOnce() {
  WiFiClientSecure client;
  client.setCACert(AUTOCONNECTO_ROOT_CA);

  HTTPClient http;
  if (!http.begin(client, "https://api.autoconnecto.in/api/v1/device-claims/handoff")) {
    Serial.println("[CLAIM] http begin failed");
    return false;
  }
  http.addHeader("Content-Type", "application/json");
  String body = String("{\"code\":\"") + CLAIM_CODE + "\"}";
  const int status = http.POST(body);
  String resp = http.getString();
  http.end();

  Serial.printf("[CLAIM] handoff HTTP %d\n", status);
  if (status != 200) {
    // 401 before claim: wait. 401 after one success: code is spent.
    return false;
  }

  // Minimal parse — look for "deviceToken":"...".
  int key = resp.indexOf("\"deviceToken\"");
  if (key < 0) {
    Serial.println("[CLAIM] no deviceToken in body");
    return false;
  }
  int q1 = resp.indexOf('"', key + 13);
  int q2 = resp.indexOf('"', q1 + 1);
  if (q1 < 0 || q2 < 0) return false;
  String token = resp.substring(q1 + 1, q2);
  if (token.length() < 8) return false;

  prefs.begin("ac_claim", false);
  prefs.putString("deviceToken", token);
  prefs.end();
  Serial.println("[CLAIM] token stored in NVS (not printed)");
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(1500);

  prefs.begin("ac_claim", true);
  String existing = prefs.getString("deviceToken", "");
  prefs.end();
  if (existing.length() >= 8) {
    Serial.println("[CLAIM] token already in NVS — done. Use it in your MQTT sketch.");
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  for (int i = 0; i < 60 && WiFi.status() != WL_CONNECTED; i++) delay(500);
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[CLAIM] Wi-Fi failed");
    return;
  }
  Serial.println("[CLAIM] Wi-Fi ok — polling handoff");
}

void loop() {
  prefs.begin("ac_claim", true);
  bool have = prefs.getString("deviceToken", "").length() >= 8;
  prefs.end();
  if (have) {
    delay(60000);
    return;
  }
  if (WiFi.status() == WL_CONNECTED) {
    if (handoffOnce()) {
      Serial.println("[CLAIM] success — stop polling");
    }
  }
  delay(15000);
}
