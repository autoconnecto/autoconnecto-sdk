// =============================================================
// Generator_Monitoring_mqtt — Autoconnecto SDK example
//
// PURPOSE
//   Simulates diesel generator telemetry for the dashboard
//   "Generator Monitoring" widget (type: generatorMonitoring).
//
// DASHBOARD
//   Add widget: Generator Monitoring → bind device alias → this device.
//
// TELEMETRY KEYS (v1 contract — flat JSON on devices/{token}/telemetry)
//   gen_run_state, gen_mode, gen_on_load
//   gen_kw, gen_kva, gen_pf, gen_hz, gen_load_pct
//   gen_v_l1, gen_v_l2, gen_v_l3, gen_i_l1, gen_i_l2, gen_i_l3
//   gen_rpm, gen_oil_pressure, gen_coolant_temp, gen_fuel_level_pct
//   gen_battery_v, gen_run_hours, gen_alarm_count, gen_alarm_text
//
// CLIENT ATTRIBUTES (optional, slow-changing)
//   gen_rated_kva, gen_nominal_v, gen_nominal_hz
// =============================================================

#include <AutoconnectoSDK.h>

AutoconnectoSDK sdk;

enum GenPhase : uint8_t {
  PHASE_STOPPED = 0,
  PHASE_STARTING,
  PHASE_RUNNING,
  PHASE_FAULT,
};

GenPhase phase = PHASE_STOPPED;
unsigned long phaseStartedMs = 0;
float runHours = 1200.0f;
float fuelPct = 72.0f;
uint32_t tick = 0;

void setup() {
  Serial.begin(115200);
  delay(2000);

  SDKConfig config;
  config.wifiSSID = "YOUR_WIFI_SSID";
  config.wifiPassword = "YOUR_WIFI_PASSWORD";
  config.mqttHost = "mqtt.autoconnecto.in";
  config.mqttPort = 8883;
  config.wssPort = 8084;
  config.deviceToken = "YOUR_DEVICE_TOKEN";
  config.enableWS = true;
  config.enableMQTT = true;
  config.allowInsecureTLS = false;
  config.rootCA = AUTOCONNECTO_ROOT_CA;
  config.enableSerialLogs = true;

  sdk.begin(config);
  phaseStartedMs = millis();
  Serial.println("[SDK] Generator_Monitoring_mqtt started");
}

const char* runStateForPhase(GenPhase p) {
  switch (p) {
    case PHASE_STARTING: return "starting";
    case PHASE_RUNNING:  return "running";
    case PHASE_FAULT:    return "fault";
    default:             return "stopped";
  }
}

void sendRatedAttributes() {
  StaticJsonDocument<128> attrs;
  attrs["gen_rated_kva"] = 200;
  attrs["gen_nominal_v"] = 415;
  attrs["gen_nominal_hz"] = 50;
  sdk.sendClientAttributes(attrs);
}

void advancePhase() {
  const unsigned long elapsed = millis() - phaseStartedMs;

  if (phase == PHASE_STOPPED && elapsed > 8000) {
    phase = PHASE_STARTING;
    phaseStartedMs = millis();
    return;
  }
  if (phase == PHASE_STARTING && elapsed > 4000) {
    phase = PHASE_RUNNING;
    phaseStartedMs = millis();
    return;
  }
  if (phase == PHASE_RUNNING && elapsed > 45000) {
    phase = PHASE_STOPPED;
    phaseStartedMs = millis();
    return;
  }
  // Occasional fault demo (~every 6th run cycle)
  if (phase == PHASE_RUNNING && elapsed > 20000 && (tick % 6) == 5) {
    phase = PHASE_FAULT;
    phaseStartedMs = millis();
    return;
  }
  if (phase == PHASE_FAULT && elapsed > 12000) {
    phase = PHASE_STOPPED;
    phaseStartedMs = millis();
  }
}

void sendGeneratorTelemetry() {
  advancePhase();
  tick++;

  const bool running = phase == PHASE_RUNNING;
  const bool starting = phase == PHASE_STARTING;
  const bool fault = phase == PHASE_FAULT;
  const bool onLoad = running;

  const float loadPct = running ? 55.0f + (float)(tick % 20) : (starting ? 8.0f : 0.0f);
  const float kw = running ? (loadPct / 100.0f) * 180.0f : 0.0f;
  const float kva = running ? kw / 0.92f : 0.0f;
  const float baseV = running ? 238.0f : (starting ? 220.0f : 0.0f);

  if (running) {
    runHours += 10.0f / 3600.0f;
    fuelPct = max(5.0f, fuelPct - 0.02f);
  }

  StaticJsonDocument<768> tel;
  tel["gen_run_state"] = runStateForPhase(phase);
  tel["gen_mode"] = "auto";
  tel["gen_on_load"] = onLoad;

  tel["gen_kw"] = kw;
  tel["gen_kva"] = kva;
  tel["gen_pf"] = running ? 0.91f : 0.0f;
  tel["gen_hz"] = running ? 50.02f : (starting ? 49.5f : 0.0f);
  tel["gen_load_pct"] = loadPct;

  tel["gen_v_l1"] = baseV + (running ? 0.4f : 0.0f);
  tel["gen_v_l2"] = baseV - 0.2f;
  tel["gen_v_l3"] = baseV + 0.1f;
  tel["gen_i_l1"] = running ? 210.0f + (tick % 5) : 0.0f;
  tel["gen_i_l2"] = running ? 215.0f : 0.0f;
  tel["gen_i_l3"] = running ? 208.0f : 0.0f;

  tel["gen_rpm"] = running ? 1500.0f : (starting ? 900.0f : 0.0f);
  tel["gen_oil_pressure"] = running ? 4.1f + (tick % 3) * 0.05f : 0.0f;
  tel["gen_coolant_temp"] = running ? 82.0f + (tick % 4) : (starting ? 45.0f : 28.0f);
  tel["gen_fuel_level_pct"] = fuelPct;
  tel["gen_battery_v"] = 27.2f;
  tel["gen_run_hours"] = runHours;

  if (fault) {
    tel["gen_alarm_count"] = 1;
    tel["gen_alarm_text"] = "Simulated over-temperature";
  } else {
    tel["gen_alarm_count"] = 0;
    tel["gen_alarm_text"] = "";
  }

  sdk.sendTelemetry(tel);
  Serial.println("[TEL] Generator telemetry sent");
}

unsigned long lastTelemetry = 0;
unsigned long lastAttrs = 0;

void loop() {
  sdk.loop();

  const unsigned long now = millis();

  if (now - lastTelemetry > 10000) {
    lastTelemetry = now;
    sendGeneratorTelemetry();
  }

  if (now - lastAttrs > 60000) {
    lastAttrs = now;
    sendRatedAttributes();
    Serial.println("[ATTR] Rated generator attributes sent");
  }

  delay(10);
}
