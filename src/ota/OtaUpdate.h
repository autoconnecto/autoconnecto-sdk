#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <functional>

// ThingsBoard-compatible OTA: shared fw_* attributes trigger HTTPS chunked download.
// Report progress with client attribute fw_state.

struct OtaConfig {
  String apiHost;
  String deviceToken;
  const char* rootCA = nullptr;
  bool allowInsecureTLS = false;
  uint32_t chunkSize = 16384;
  bool autoReboot = true;
};

using OtaClientAttributeFn =
  std::function<bool(const char* key, const char* value)>;

class AutoconnectoOta {
public:
  void begin(const OtaConfig& config, OtaClientAttributeFn sendClientAttr);

  void onSharedAttribute(const String& key, JsonVariant value);

  void loop();

  bool isBusy() const { return _busy; }

private:
  enum class Phase : uint8_t {
    Idle,
    Downloading,
    Verifying,
    Done,
    Failed,
  };

  OtaConfig _cfg;
  OtaClientAttributeFn _sendAttr;

  Phase _phase = Phase::Idle;
  bool _busy = false;

  String _title;
  String _version;
  size_t _fileSize = 0;
  String _checksum;
  String _checksumAlgo = "SHA256";

  size_t _chunkIndex = 0;
  size_t _bytesWritten = 0;
  bool _shaStarted = false;

  void resetTarget();
  bool metadataComplete() const;
  void startDownload();
  bool reportState(const char* state);
  bool downloadNextChunk();
  bool verifyChecksum();
  void fail(const char* reason);
};
