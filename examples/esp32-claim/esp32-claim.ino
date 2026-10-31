// Claim-code handoff (legacy plain HTTP). Prefer examples/Claim_https for TLS.
// The QR and this sketch hold the code, not the device token.
// Not board-verified. Poll until a person claims the code, then store the token.
//
// Prefer examples/Claim_https (stores the token, does not print it).
// POST https://api.autoconnecto.in/api/v1/device-claims/handoff
// Body: {"code":"acc_..."}
// 201 (any 2xx): deviceId and deviceToken. Do not print them.
// 401 before claim: the code has not been claimed yet. Wait and try again.
// 401 after the token was returned once: the code is dead.

#include <WiFi.h>
#include <HTTPClient.h>
#include "secrets.h"

const char *API_BASE = "https://api.autoconnecto.in";

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) delay(500);
}

void loop() {
  HTTPClient http;
  http.begin(String(API_BASE) + "/api/v1/device-claims/handoff");
  http.addHeader("Content-Type", "application/json");
  int status = http.POST(String("{\"code\":\"") + CLAIM_CODE + "\"}");
  String resp = http.getString();
  http.end();
  Serial.printf("handoff HTTP %d\n", status);
  if (status >= 200 && status < 300) {
    Serial.println("handoff ok — token not printed");
    while (true) delay(1000);
  }
  if (resp.indexOf("deviceToken") < 0 && resp.indexOf(CLAIM_CODE) < 0) {
    const int key = resp.indexOf("\"message\"");
    const int q1 = key >= 0 ? resp.indexOf('"', key + 9) : -1;
    const int q2 = q1 >= 0 ? resp.indexOf('"', q1 + 1) : -1;
    if (q1 >= 0 && q2 > q1 && q2 - q1 <= 160) {
      Serial.println(resp.substring(q1 + 1, q2));
    }
  }
  delay(15000);
}
