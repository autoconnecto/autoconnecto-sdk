#include "OtaUpdate.h"

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Update.h>
#include <LittleFS.h>

#include <mbedtls/sha256.h>

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

void AutoconnectoOta::onSoftwareReady(OtaSoftwareReadyFn fn) {
  _onSoftwareReady = fn;
}

void AutoconnectoOta::resetTarget() {
  _phase = Phase::Idle;
  _busy = false;
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

  // Starting a new package kind clears prior partial metadata.
  if (_title.length() || _version.length() || _fileSize || _checksum.length()) {
    if (kind != _kind) {
      resetTarget();
    }
  }
  _kind = kind;

  if (strcmp(field, "title") == 0) {
    _title = value;
  } else if (strcmp(field, "version") == 0) {
    _version = value;
  } else if (strcmp(field, "size") == 0) {
    _fileSize = static_cast<size_t>(atol(value.c_str()));
  } else if (strcmp(field, "checksum") == 0) {
    _checksum = value;
  } else if (strcmp(field, "checksum_algorithm") == 0) {
    _checksumAlgo = value;
    _checksumAlgo.toUpperCase();
  } else {
    return;
  }

  if (metadataComplete()) {
    startDownload();
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
    startDownload();
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
  if (!_sendAttr) return false;
  Serial.printf("[OTA] %s=%s\n", stateKey(), state);
  return _sendAttr(stateKey(), state);
}

bool AutoconnectoOta::downloadNextChunk() {
  if (_phase != Phase::Downloading) return false;

  WiFiClientSecure client;
  if (_cfg.allowInsecureTLS) {
    client.setInsecure();
  } else if (_cfg.rootCA) {
    client.setCACert(_cfg.rootCA);
  }

  HTTPClient http;
  String url = String("https://") + _cfg.apiHost + "/api/v1/" +
               _cfg.deviceToken + "/" + downloadPath() + "?title=" +
               urlEncode(_title) + "&version=" + urlEncode(_version) +
               "&size=" + String(_cfg.chunkSize) +
               "&chunk=" + String(_chunkIndex);

  if (!http.begin(client, url)) {
    fail("http_begin_failed");
    return false;
  }

  const int code = http.GET();
  if (code != 200) {
    Serial.printf("[OTA] HTTP GET %d url=%s\n", code, url.c_str());
    http.end();
    fail("http_get_failed");
    return false;
  }

  WiFiClient* stream = http.getStreamPtr();
  File softFile;
  if (_kind == OtaPackageKind::Software) {
    const char* path = _cfg.softwarePath ? _cfg.softwarePath : "/ota/package.bin";
    softFile = LittleFS.open(path, _chunkIndex == 0 ? "w" : "a");
    if (!softFile) {
      http.end();
      fail("software_write_open_failed");
      return false;
    }
  }

  while (http.connected() && _bytesWritten < _fileSize) {
    const size_t avail = stream->available();
    if (!avail) {
      if (!stream->connected()) break;
      delay(1);
      continue;
    }

    uint8_t buf[512];
    const size_t n = stream->readBytes(buf, min(avail, sizeof(buf)));
    if (!n) break;

    if (!_shaStarted) {
      mbedtls_sha256_init(&g_sha);
      mbedtls_sha256_starts(&g_sha, 0);
      _shaStarted = true;
    }
    mbedtls_sha256_update(&g_sha, buf, n);

    if (_kind == OtaPackageKind::Firmware) {
      if (Update.write(buf, n) != n) {
        http.end();
        fail("flash_write_failed");
        return false;
      }
    } else {
      if (softFile.write(buf, n) != n) {
        softFile.close();
        http.end();
        fail("software_write_failed");
        return false;
      }
    }

    _bytesWritten += n;
  }

  if (_kind == OtaPackageKind::Software) {
    softFile.close();
  }
  http.end();

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

  reportState("UPDATED");
  _phase = Phase::Done;
  _busy = false;

  if (_kind == OtaPackageKind::Firmware && _cfg.autoReboot) {
    delay(500);
    ESP.restart();
  }

  return true;
}

void AutoconnectoOta::fail(const char* reason) {
  _phase = Phase::Failed;
  _busy = false;
  reportState("FAILED");
  if (_kind == OtaPackageKind::Firmware) {
    Update.abort();
  }
  Serial.printf("[OTA] failed: %s\n", reason);
}

void AutoconnectoOta::loop() {
  if (!_busy) return;

  if (_phase == Phase::Downloading) {
    if (!downloadNextChunk()) return;

    if (_phase == Phase::Verifying) {
      if (!verifyChecksum()) return;
      finishApply();
    }
  }
}
