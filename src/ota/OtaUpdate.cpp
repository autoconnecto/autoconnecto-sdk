#include "../core/Esp32Only.h"
#if defined(AUTOCONNECTO_ESP32_CLIENT)

#include "OtaUpdate.h"

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Update.h>
#include <LittleFS.h>
#if defined(ESP32)
#include <Preferences.h>
#endif

#include <mbedtls/sha256.h>
extern "C" {
#include <esp_ota_ops.h>
}

namespace {
WiFiClientSecure* tlsClient = nullptr;
HTTPClient* httpClient = nullptr;
}

namespace {

String urlEncode(const String& s) {
  String out;
  const char* hex = "0123456789ABCDEF";
  for (size_t i = 0; i < s.length(); i++) {
    const char c = s[i];
    if (isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_' ||
        c == '.' || c == '~') {
      out += c;
    } else {
      out += '%';
      out += hex[(c >> 4) & 0xF];
      out += hex[c & 0xF];
    }
  }
  return out;
}

mbedtls_sha256_context g_sha;

}  // namespace

void AutoconnectoOta::begin(
  const OtaConfig& config,
  OtaClientAttributeFn sendClientAttr
) {
  _cfg = config;
  _sendAttr = sendClientAttr;
  resetTarget();
}

void AutoconnectoOta::onProgressReport(OtaProgressFn fn) {
  _progress = fn;
}

void AutoconnectoOta::onSoftwareReady(OtaSoftwareReadyFn fn) {
  _onSoftwareReady = fn;
}

void AutoconnectoOta::resetTarget() {
  _phase = Phase::Idle;
  _busy = false;
  _startRequested = false;
  _kind = OtaPackageKind::Firmware;
  _title = "";
  _version = "";
  _fileSize = 0;
  _checksum = "";
  _checksumAlgo = "SHA256";
  _chunkIndex = 0;
  _bytesWritten = 0;
  _shaStarted = false;
}

void AutoconnectoOta::clearPackageBytes() {
  _fileSize = 0;
  _checksum = "";
  _startRequested = false;
}

void AutoconnectoOta::rememberInstalled() {
#if defined(ESP32)
  Preferences prefs;
  if (!prefs.begin("ac_ota", false)) return;
  prefs.putString("title", _title);
  prefs.putString("ver", _version);
  prefs.putString("kind", _kind == OtaPackageKind::Software ? "sw" : "fw");
  prefs.end();
#else
  (void)0;
#endif
}

bool AutoconnectoOta::alreadyInstalled() {
#if defined(ESP32)
  Preferences prefs;
  if (!prefs.begin("ac_ota", true)) return false;
  const String title = prefs.getString("title", "");
  const String version = prefs.getString("ver", "");
  const String kind = prefs.getString("kind", "");
  prefs.end();
  const char* want = _kind == OtaPackageKind::Software ? "sw" : "fw";
  return title == _title && version == _version && kind == want;
#else
  return false;
#endif
}

bool AutoconnectoOta::acceptAttrKey(
  const String& key,
  OtaPackageKind* outKind,
  const char** outField
) const {
  if (key.startsWith("fw_")) {
    *outKind = OtaPackageKind::Firmware;
    *outField = key.c_str() + 3;
    return true;
  }
  if (key.startsWith("sw_")) {
    *outKind = OtaPackageKind::Software;
    *outField = key.c_str() + 3;
    return true;
  }
  return false;
}

void AutoconnectoOta::onSharedAttribute(const String& key, const String& value) {
  if (_busy) return;

  OtaPackageKind kind;
  const char* field = nullptr;
  if (!acceptAttrKey(key, &kind, &field)) return;

  if (_title.length() || _version.length() || _fileSize || _checksum.length()) {
    if (kind != _kind) {
      resetTarget();
    }
  }
  _kind = kind;

  if (strcmp(field, "title") == 0) {
    if (_title.length() && value != _title) clearPackageBytes();
    _title = value;
  } else if (strcmp(field, "version") == 0) {
    if (_version.length() && value != _version) clearPackageBytes();
    _version = value;
  } else if (strcmp(field, "size") == 0) {
    _fileSize = static_cast<size_t>(strtoul(value.c_str(), nullptr, 10));
  } else if (strcmp(field, "checksum") == 0) {
    _checksum = value;
  } else if (strcmp(field, "checksum_algorithm") == 0) {
    _checksumAlgo = value;
    _checksumAlgo.toUpperCase();
  } else {
    return;
  }

  if (metadataComplete()) {
    _startRequested = true;
  }
}

void AutoconnectoOta::onSharedAttribute(const String& key, float value) {
  if (_busy) return;

  OtaPackageKind kind;
  const char* field = nullptr;
  if (!acceptAttrKey(key, &kind, &field)) return;
  if (strcmp(field, "size") != 0) return;

  if (_title.length() || _version.length() || _fileSize || _checksum.length()) {
    if (kind != _kind) {
      resetTarget();
    }
  }
  _kind = kind;
  _fileSize = static_cast<size_t>(value);

  if (metadataComplete()) {
    _startRequested = true;
  }
}

bool AutoconnectoOta::metadataComplete() const {
  return _title.length() > 0 && _version.length() > 0 && _fileSize > 0 &&
         _checksum.length() > 0;
}

const char* AutoconnectoOta::stateKey() const {
  return _kind == OtaPackageKind::Software ? "sw_state" : "fw_state";
}

const char* AutoconnectoOta::downloadPath() const {
  return _kind == OtaPackageKind::Software ? "software" : "firmware";
}

void AutoconnectoOta::closeHttp() {
  if (httpClient) {
    httpClient->end();
    delete httpClient;
    httpClient = nullptr;
  }
  if (tlsClient) {
    tlsClient->stop();
    delete tlsClient;
    tlsClient = nullptr;
  }
}

void AutoconnectoOta::startDownload() {
  _busy = true;
  _phase = Phase::Downloading;
  _chunkIndex = 0;
  _bytesWritten = 0;
  _shaStarted = false;

  Serial.printf(
    "[OTA] start kind=%s title=%s version=%s size=%u\n",
    _kind == OtaPackageKind::Software ? "SOFTWARE" : "FIRMWARE",
    _title.c_str(),
    _version.c_str(),
    static_cast<unsigned>(_fileSize)
  );

  if (_kind == OtaPackageKind::Firmware) {
    if (!Update.begin(_fileSize)) {
      fail("update_begin_failed");
      return;
    }
  } else {
    if (!LittleFS.begin(true)) {
      fail("littlefs_mount_failed");
      return;
    }
    const char* path = _cfg.softwarePath ? _cfg.softwarePath : "/ota/package.bin";
    // Ensure parent directory exists (best-effort).
    String dir = path;
    const int slash = dir.lastIndexOf('/');
    if (slash > 0) {
      LittleFS.mkdir(dir.substring(0, slash));
    }
    File f = LittleFS.open(path, "w");
    if (!f) {
      fail("software_open_failed");
      return;
    }
    f.close();
  }

  reportState("DOWNLOADING");
}

bool AutoconnectoOta::reportState(const char* state) {
  Serial.printf(
    "[OTA] %s=%s title=%s version=%s\n",
    stateKey(),
    state,
    _title.c_str(),
    _version.c_str()
  );

  // Board evidence (v1.7.71): VERIFIED then UPDATING+UPDATED in the same ms
  // left the server stuck at UPDATING — later MQTT client-attr publishes were
  // dropped. Space reports and retry terminal states.
  const bool critical =
    strcmp(state, "VERIFIED") == 0 ||
    strcmp(state, "UPDATING") == 0 ||
    strcmp(state, "UPDATED") == 0 ||
    strcmp(state, "FAILED") == 0;
  const int attempts = critical ? 3 : 1;

  bool ok = false;
  for (int i = 0; i < attempts; i++) {
    if (_progress) {
      ok = _progress(stateKey(), state, _title.c_str(), _version.c_str());
    } else if (_sendAttr) {
      ok = _sendAttr(stateKey(), state);
    } else {
      return false;
    }
    // Let the MQTT/WSS client flush before the next state transition.
    delay(critical ? 200 : 50);
    if (ok) break;
    delay(150);
  }
  return ok;
}

bool AutoconnectoOta::downloadNextChunk() {
  if (_phase != Phase::Downloading) return false;

  if (!tlsClient) {
    tlsClient = new WiFiClientSecure();
    if (_cfg.allowInsecureTLS) {
      tlsClient->setInsecure();
    } else if (_cfg.rootCA) {
      tlsClient->setCACert(_cfg.rootCA);
    }
  }
  if (!httpClient) {
    httpClient = new HTTPClient();
    httpClient->setReuse(true);
  }

  String url = String("https://") + _cfg.apiHost + "/api/v1/" +
               _cfg.deviceToken + "/" + downloadPath() + "?title=" +
               urlEncode(_title) + "&version=" + urlEncode(_version) +
               "&size=" + String(_cfg.chunkSize) +
               "&chunk=" + String(_chunkIndex);

  if (!httpClient->begin(*tlsClient, url)) {
    fail("http_begin_failed");
    return false;
  }

  const int code = httpClient->GET();
  if (code != 200) {
    // Never print the full URL — it embeds the device token.
    Serial.printf(
      "[OTA] HTTP GET %d path=/%s title=%s version=%s chunk=%u\n",
      code,
      downloadPath(),
      _title.c_str(),
      _version.c_str(),
      static_cast<unsigned>(_chunkIndex)
    );
    httpClient->end();
    fail("http_get_failed");
    return false;
  }

  // One HTTP response is one chunk (Content-Length = this slice), not the
  // whole package. The TLS client is kept so the next chunk does not handshake again.
  const int contentLen = httpClient->getSize();
  const size_t chunkExpect =
    contentLen > 0
      ? static_cast<size_t>(contentLen)
      : static_cast<size_t>(_cfg.chunkSize);

  WiFiClient* stream = httpClient->getStreamPtr();
  File softFile;
  if (_kind == OtaPackageKind::Software) {
    const char* path = _cfg.softwarePath ? _cfg.softwarePath : "/ota/package.bin";
    softFile = LittleFS.open(path, _chunkIndex == 0 ? "w" : "a");
    if (!softFile) {
      httpClient->end();
      fail("software_write_open_failed");
      return false;
    }
  }

  size_t chunkGot = 0;
  unsigned long idleSince = millis();
  const unsigned long idleTimeoutMs = 8000;

  while (_bytesWritten < _fileSize && chunkGot < chunkExpect) {
    if (!httpClient->connected() && stream->available() == 0) break;

    const size_t avail = stream->available();
    if (!avail) {
      if (!stream->connected() && stream->available() == 0) break;
      if (millis() - idleSince > idleTimeoutMs) {
        Serial.printf(
          "[OTA] chunk idle timeout idx=%u got=%u expect=%u total=%u/%u\n",
          static_cast<unsigned>(_chunkIndex),
          static_cast<unsigned>(chunkGot),
          static_cast<unsigned>(chunkExpect),
          static_cast<unsigned>(_bytesWritten),
          static_cast<unsigned>(_fileSize)
        );
        break;
      }
      delay(1);
      yield();
      continue;
    }
    idleSince = millis();

    uint8_t buf[512];
    const size_t want = min(avail, min(sizeof(buf), chunkExpect - chunkGot));
    const size_t n = stream->readBytes(buf, want);
    if (!n) break;

    if (!_shaStarted) {
      mbedtls_sha256_init(&g_sha);
      mbedtls_sha256_starts(&g_sha, 0);
      _shaStarted = true;
    }
    mbedtls_sha256_update(&g_sha, buf, n);

    if (_kind == OtaPackageKind::Firmware) {
      if (Update.write(buf, n) != n) {
        httpClient->end();
        fail("flash_write_failed");
        return false;
      }
    } else {
      if (softFile.write(buf, n) != n) {
        softFile.close();
        httpClient->end();
        fail("software_write_failed");
        return false;
      }
    }

    chunkGot += n;
    _bytesWritten += n;
  }

  if (_kind == OtaPackageKind::Software) {
    softFile.close();
  }
  httpClient->end();

  // Empty body at past-EOF means the server has no more bytes.
  if (chunkGot == 0 && _bytesWritten >= _fileSize) {
    reportState("DOWNLOADED");
    _phase = Phase::Verifying;
    return true;
  }

  if (chunkGot == 0) {
    fail("chunk_empty");
    return false;
  }

  Serial.printf(
    "[OTA] chunk %u ok bytes=%u total=%u/%u\n",
    static_cast<unsigned>(_chunkIndex),
    static_cast<unsigned>(chunkGot),
    static_cast<unsigned>(_bytesWritten),
    static_cast<unsigned>(_fileSize)
  );

  if (_bytesWritten >= _fileSize) {
    reportState("DOWNLOADED");
    _phase = Phase::Verifying;
    return true;
  }

  _chunkIndex++;
  return true;
}

bool AutoconnectoOta::verifyChecksum() {
  if (!_shaStarted) {
    fail("checksum_missing");
    return false;
  }

  uint8_t digest[32];
  mbedtls_sha256_finish(&g_sha, digest);
  mbedtls_sha256_free(&g_sha);
  _shaStarted = false;

  if (_checksumAlgo != "SHA256") {
    fail("unsupported_checksum_algo");
    return false;
  }

  char hex[65];
  for (int i = 0; i < 32; i++) {
    sprintf(hex + (i * 2), "%02x", digest[i]);
  }
  hex[64] = 0;

  if (_checksum.equalsIgnoreCase(hex)) {
    reportState("VERIFIED");
    return true;
  }

  Serial.printf("[OTA] checksum expected=%s got=%s\n", _checksum.c_str(), hex);
  fail("checksum_mismatch");
  return false;
}

bool AutoconnectoOta::finishApply() {
  reportState("UPDATING");

  if (_kind == OtaPackageKind::Firmware) {
    if (!Update.end(true)) {
      fail("update_end_failed");
      return false;
    }
  } else {
    const char* path = _cfg.softwarePath ? _cfg.softwarePath : "/ota/package.bin";
    if (_onSoftwareReady) {
      _onSoftwareReady(String(path), _title, _version);
    }
  }

  // Extra gap before UPDATED — this is the report that closes the server row.
  delay(250);
  if (!reportState("UPDATED")) {
    Serial.println("[OTA] UPDATED report failed after retries; server may stay UPDATING");
  }
  const bool reboot = _kind == OtaPackageKind::Firmware && _cfg.autoReboot;
  closeHttp();
  rememberInstalled();
  resetTarget();

  if (reboot) {
    delay(500);
    ESP.restart();
  }

  return true;
}

void AutoconnectoOta::fail(const char* reason) {
  closeHttp();
  reportState("FAILED");
  if (_kind == OtaPackageKind::Firmware) {
    Update.abort();
  }
  Serial.printf("[OTA] failed: %s\n", reason);
  resetTarget();
}

void AutoconnectoOta::loop() {
  if (!_busy && _startRequested) {
    _startRequested = false;
    if (!metadataComplete()) return;
    if (alreadyInstalled()) {
      Serial.printf(
        "[OTA] skip installed title=%s version=%s\n",
        _title.c_str(),
        _version.c_str()
      );
      resetTarget();
      return;
    }
    startDownload();
  }

  if (!_busy) return;

  if (_phase == Phase::Downloading) {
    if (!downloadNextChunk()) return;

    if (_phase == Phase::Verifying) {
      if (!verifyChecksum()) return;
      finishApply();
    }
  }
}

bool AutoconnectoOta::markValidAfterHealthyCheck() {
  const esp_partition_t* running = esp_ota_get_running_partition();
  if (!running) return false;
  esp_ota_img_states_t state = ESP_OTA_IMG_UNDEFINED;
  if (esp_ota_get_state_partition(running, &state) != ESP_OK) return false;
  if (state == ESP_OTA_IMG_UNDEFINED) {
    Serial.printf("[OTA] boot state UNDEFINED on %s. A USB flash has nothing to confirm.\n", running->label);
    return false;
  }
  if (state != ESP_OTA_IMG_PENDING_VERIFY) {
    Serial.printf("[OTA] boot state %s on %s. Nothing to confirm.\n", bootStateName(), running->label);
    return state == ESP_OTA_IMG_VALID;
  }
  const esp_err_t err = esp_ota_mark_app_valid_cancel_rollback();
  Serial.printf("[OTA] mark valid on %s err=%d\n", running->label, (int)err);
  return err == ESP_OK;
}

bool AutoconnectoOta::rollbackIfUnhealthy() {
  const esp_partition_t* running = esp_ota_get_running_partition();
  if (!running) return false;
  esp_ota_img_states_t state = ESP_OTA_IMG_UNDEFINED;
  if (esp_ota_get_state_partition(running, &state) != ESP_OK) return false;
  if (state != ESP_OTA_IMG_PENDING_VERIFY) {
    Serial.printf(
      "[OTA] rollback skipped. state=%s slot=%s. Chip rollback exists only after an OTA install.\n",
      bootStateName(),
      running->label
    );
    return false;
  }
  Serial.printf("[OTA] mark invalid and reboot slot=%s\n", running->label);
  Serial.flush();
  esp_ota_mark_app_invalid_rollback_and_reboot();
  return true;
}

const char* AutoconnectoOta::bootPartitionLabel() const {
  const esp_partition_t* running = esp_ota_get_running_partition();
  return running ? running->label : "none";
}

const char* AutoconnectoOta::bootStateName() const {
  const esp_partition_t* running = esp_ota_get_running_partition();
  if (!running) return "none";
  esp_ota_img_states_t state = ESP_OTA_IMG_UNDEFINED;
  if (esp_ota_get_state_partition(running, &state) != ESP_OK) return "unknown";
  switch (state) {
    case ESP_OTA_IMG_NEW: return "NEW";
    case ESP_OTA_IMG_PENDING_VERIFY: return "PENDING_VERIFY";
    case ESP_OTA_IMG_VALID: return "VALID";
    case ESP_OTA_IMG_INVALID: return "INVALID";
    case ESP_OTA_IMG_ABORTED: return "ABORTED";
    case ESP_OTA_IMG_UNDEFINED: return "UNDEFINED";
    default: return "OTHER";
  }
}

#endif
