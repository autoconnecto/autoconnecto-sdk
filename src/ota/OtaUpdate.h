#pragma once

#include <Arduino.h>
#include <functional>

// ThingsBoard-compatible OTA:
//   FOTA — shared fw_* attributes → HTTPS /firmware chunks → esp_ota flash → fw_state
//   SOTA — shared sw_* attributes → HTTPS /software chunks → LittleFS file → sw_state

enum class OtaPackageKind : uint8_t {
  Firmware = 0,
  Software = 1,
};

struct OtaConfig {
  String apiHost;
  String deviceToken;
  const char* rootCA = nullptr;
  bool allowInsecureTLS = false;
  uint32_t chunkSize = 16384;
  /** FOTA only: reboot after successful flash. */
  bool autoReboot = true;
  /** SOTA only: destination path on LittleFS (created/truncated on each update). */
  const char* softwarePath = "/ota/package.bin";
};

using OtaClientAttributeFn =
  std::function<bool(const char* key, const char* value)>;

/** Called after SOTA file is verified on LittleFS (before sw_state=UPDATED). */
using OtaSoftwareReadyFn = std::function<void(
  const String& path,
  const String& title,
  const String& version
)>;

class AutoconnectoOta {
public:
  void begin(const OtaConfig& config, OtaClientAttributeFn sendClientAttr);

  /** Optional: handle a verified software package (config, script, asset). */
  void onSoftwareReady(OtaSoftwareReadyFn fn);

  /** String SHARED attrs (fw_* or sw_*). */
  void onSharedAttribute(const String& key, const String& value);

  /** Numeric SHARED attrs (fw_size / sw_size when sent as JSON number). */
  void onSharedAttribute(const String& key, float value);

  void loop();

  bool isBusy() const { return _busy; }

  OtaPackageKind activeKind() const { return _kind; }

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
  OtaSoftwareReadyFn _onSoftwareReady;

  Phase _phase = Phase::Idle;
  bool _busy = false;
  OtaPackageKind _kind = OtaPackageKind::Firmware;

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
  bool acceptAttrKey(const String& key, OtaPackageKind* outKind, const char** outField) const;
  void startDownload();
  bool reportState(const char* state);
  const char* stateKey() const;
  const char* downloadPath() const;
  bool downloadNextChunk();
  bool verifyChecksum();
  bool finishApply();
  void fail(const char* reason);
};
