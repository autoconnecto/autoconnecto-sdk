#include "OtaUpdate.h"

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Update.h>

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

}  // namespace

void AutoconnectoOta::begin(
  const OtaConfig& config,
  OtaClientAttributeFn sendClientAttr
) {
  _cfg = config;
  _sendAttr = sendClientAttr;
  resetTarget();
}

void AutoconnectoOta::resetTarget() {
  _phase = Phase::Idle;
  _busy = false;
  _title = "";
  _version = "";
  _fileSize = 0;
  _checksum = "";
  _chunkIndex = 0;
  _bytesWritten = 0;
  _shaStarted = false;
}

void AutoconnectoOta::onSharedAttribute(const String& key, JsonVariant value) {
  if (_busy) return;

  if (key == "fw_title") {
    _title = value.as<String>();
  } else if (key == "fw_version") {
    _version = value.as<String>();
  } else if (key == "fw_size") {
    _fileSize = value.as<size_t>();
  } else if (key == "fw_checksum") {
    _checksum = value.as<String>();
  } else if (key == "fw_checksum_algorithm") {
    _checksumAlgo = value.as<String>();
    _checksumAlgo.toUpperCase();
  } else {
    return;
  }

  if (metadataComplete()) {
    startDownload();
  }
}

bool AutoconnectoOta::metadataComplete() const {
  return _title.length() > 0 && _version.length() > 0 && _fileSize > 0 &&
         _checksum.length() > 0;
}

void AutoconnectoOta::startDownload() {
  _busy = true;
  _phase = Phase::Downloading;
  _chunkIndex = 0;
  _bytesWritten = 0;
  _shaStarted = false;

  Serial.printf(
    "[OTA] start title=%s version=%s size=%u\n",
    _title.c_str(),
    _version.c_str(),
    static_cast<unsigned>(_fileSize)
  );

  if (!Update.begin(_fileSize)) {
    fail("update_begin_failed");
    return;
  }

  reportState("DOWNLOADING");
}

bool AutoconnectoOta::reportState(const char* state) {
  if (!_sendAttr) return false;
  Serial.printf("[OTA] fw_state=%s\n", state);
  return _sendAttr("fw_state", state);
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
               _cfg.deviceToken + "/firmware?title=" + urlEncode(_title) +
               "&version=" + urlEncode(_version) + "&size=" +
               String(_cfg.chunkSize) + "&chunk=" + String(_chunkIndex);

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
  static mbedtls_sha256_context sha;

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
      mbedtls_sha256_init(&sha);
      mbedtls_sha256_starts(&sha, 0);
      _shaStarted = true;
    }
    mbedtls_sha256_update(&sha, buf, n);

    if (Update.write(buf, n) != n) {
      http.end();
      fail("flash_write_failed");
      return false;
    }

    _bytesWritten += n;
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
  static mbedtls_sha256_context sha;

  if (!_shaStarted) {
    fail("checksum_missing");
    return false;
  }

  uint8_t digest[32];
  mbedtls_sha256_finish(&sha, digest);
  mbedtls_sha256_free(&sha);
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

void AutoconnectoOta::fail(const char* reason) {
  _phase = Phase::Failed;
  _busy = false;
  reportState("FAILED");
  Update.abort();
  Serial.printf("[OTA] failed: %s\n", reason);
}

void AutoconnectoOta::loop() {
  if (!_busy) return;

  if (_phase == Phase::Downloading) {
    if (!downloadNextChunk()) return;

    if (_phase == Phase::Verifying) {
      if (!verifyChecksum()) return;

      reportState("UPDATING");
      if (!Update.end(true)) {
        fail("update_end_failed");
        return;
      }

      reportState("UPDATED");
      _phase = Phase::Done;
      _busy = false;

      if (_cfg.autoReboot) {
        delay(500);
        ESP.restart();
      }
    }
  }
}
