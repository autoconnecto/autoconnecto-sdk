/**
 * Add secrets.h.example to every sketch folder and strip inline credentials from .ino files.
 * Run: node scripts/apply-secrets-h.mjs
 */
import fs from "fs";
import path from "path";
import { fileURLToPath } from "url";

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const examplesRoot = path.join(__dirname, "..", "examples");

const STANDARD_EXAMPLE = `#pragma once

// Copy to secrets.h (gitignored) and fill in real values.
// Never commit secrets.h.

#ifndef WIFI_SSID
#define WIFI_SSID "YOUR_WIFI_SSID"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#endif

#ifndef DEVICE_TOKEN
#define DEVICE_TOKEN "YOUR_DEVICE_TOKEN"
#endif
`;

const LTE_EXAMPLE = `#pragma once

// Copy to secrets.h (gitignored) and fill in real values.
// Never commit secrets.h.

#ifndef LTE_APN
#define LTE_APN "YOUR_CARRIER_APN"
#endif

#ifndef DEVICE_TOKEN
#define DEVICE_TOKEN "YOUR_DEVICE_TOKEN"
#endif
`;

const CLAIM_EXAMPLE = `#pragma once

#ifndef WIFI_SSID
#define WIFI_SSID "YOUR_WIFI_SSID"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#endif

#ifndef CLAIM_CODE
#define CLAIM_CODE "acc_replace_me"
#endif
`;

const GATEWAY_EXAMPLE = `#pragma once

#ifndef WIFI_SSID
#define WIFI_SSID "YOUR_WIFI_SSID"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#endif

#ifndef GATEWAY_TOKEN
#define GATEWAY_TOKEN "YOUR_GATEWAY_DEVICE_TOKEN"
#endif

#ifndef CHILD_DEVICE_ID
#define CHILD_DEVICE_ID "YOUR_CHILD_DEVICE_UUID"
#endif
`;

const FLEET_EXAMPLE = `#pragma once

#ifndef WIFI_SSID
#define WIFI_SSID "YOUR_WIFI_SSID"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#endif

// Optional: override API host (defaults in sketch if unset).
// #ifndef API_HOST
// #define API_HOST "api.autoconnecto.in"
// #endif

#ifndef FLEET_TOKEN_1
#define FLEET_TOKEN_1 "REPLACE_WITH_TOKEN_1"
#endif
#ifndef FLEET_TOKEN_2
#define FLEET_TOKEN_2 "REPLACE_WITH_TOKEN_2"
#endif
#ifndef FLEET_TOKEN_3
#define FLEET_TOKEN_3 "REPLACE_WITH_TOKEN_3"
#endif
#ifndef FLEET_TOKEN_4
#define FLEET_TOKEN_4 "REPLACE_WITH_TOKEN_4"
#endif
#ifndef FLEET_TOKEN_5
#define FLEET_TOKEN_5 "REPLACE_WITH_TOKEN_5"
#endif
#ifndef FLEET_TOKEN_6
#define FLEET_TOKEN_6 "REPLACE_WITH_TOKEN_6"
#endif
#ifndef FLEET_TOKEN_7
#define FLEET_TOKEN_7 "REPLACE_WITH_TOKEN_7"
#endif
#ifndef FLEET_TOKEN_8
#define FLEET_TOKEN_8 "REPLACE_WITH_TOKEN_8"
#endif
#ifndef FLEET_TOKEN_9
#define FLEET_TOKEN_9 "REPLACE_WITH_TOKEN_9"
#endif
#ifndef FLEET_TOKEN_10
#define FLEET_TOKEN_10 "REPLACE_WITH_TOKEN_10"
#endif
`;

function ensureInclude(src) {
  if (/#include\s+"secrets\.h"/.test(src)) return src;
  // After the last #include ... line near the top
  const lines = src.split(/\r?\n/);
  let lastInclude = -1;
  for (let i = 0; i < Math.min(lines.length, 80); i++) {
    if (/^\s*#include\s+/.test(lines[i])) lastInclude = i;
    if (lastInclude >= 0 && i > lastInclude + 5 && !/^\s*#include/.test(lines[i]) && lines[i].trim() !== "") {
      break;
    }
  }
  if (lastInclude < 0) {
    return `#include "secrets.h"\n\n` + src;
  }
  lines.splice(lastInclude + 1, 0, `#include "secrets.h"`);
  return lines.join("\n");
}

function stripStaticWifiDevice(src) {
  return src
    .replace(
      /^\s*static const char\*\s*WIFI_SSID\s*=\s*"[^"]*"\s*;\s*\r?\n/gm,
      ""
    )
    .replace(
      /^\s*static const char\*\s*WIFI_PASSWORD\s*=\s*"[^"]*"\s*;\s*\r?\n/gm,
      ""
    )
    .replace(
      /^\s*static const char\*\s*DEVICE_TOKEN\s*=\s*"[^"]*"\s*;\s*\r?\n/gm,
      ""
    )
    .replace(
      /^\s*const char\s*\*\s*WIFI_SSID\s*=\s*"[^"]*"\s*;\s*\r?\n/gm,
      ""
    )
    .replace(
      /^\s*const char\s*\*\s*WIFI_PASSWORD\s*=\s*"[^"]*"\s*;\s*\r?\n/gm,
      ""
    )
    .replace(
      /^\s*const char\s*\*\s*CLAIM_CODE\s*=\s*"[^"]*"\s*;\s*\r?\n/gm,
      ""
    )
    .replace(
      /^\s*static const char\*\s*GATEWAY_TOKEN\s*=\s*"[^"]*"\s*;\s*\r?\n/gm,
      ""
    )
    .replace(
      /^\s*static const char\*\s*CHILD_DEVICE_ID\s*=\s*"[^"]*"\s*;\s*\r?\n/gm,
      ""
    );
}

function patchSdkConfig(src) {
  return src
    .replace(
      /config\.wifiSSID\s*=\s*"[^"]*"\s*;/g,
      "config.wifiSSID = WIFI_SSID;"
    )
    .replace(
      /config\.wifiPassword\s*=\s*"[^"]*"\s*;/g,
      "config.wifiPassword = WIFI_PASSWORD;"
    )
    .replace(
      /config\.deviceToken\s*=\s*"[^"]*"\s*;/g,
      "config.deviceToken = DEVICE_TOKEN;"
    )
    .replace(
      /config\.lteApn\s*=\s*"[^"]*"\s*;/g,
      "config.lteApn = LTE_APN;"
    );
}

function walkInoDirs(dir, out = []) {
  for (const ent of fs.readdirSync(dir, { withFileTypes: true })) {
    if (!ent.isDirectory()) continue;
    if (ent.name === "integrations" || ent.name === "CompileTest") continue;
    const full = path.join(dir, ent.name);
    const inos = fs.readdirSync(full).filter((f) => f.endsWith(".ino"));
    if (inos.length) out.push({ dir: full, name: ent.name, inos });
    else walkInoDirs(full, out);
  }
  return out;
}

const folders = walkInoDirs(examplesRoot);
const report = [];

for (const { dir, name, inos } of folders) {
  let exampleBody = STANDARD_EXAMPLE;
  if (name.includes("lte_ppp") || name.includes("Lte")) exampleBody = LTE_EXAMPLE;
  if (name === "Claim_https" || name === "esp32-claim") exampleBody = CLAIM_EXAMPLE;
  if (name === "GatewayRelay_http") exampleBody = GATEWAY_EXAMPLE;
  if (name === "FleetSimulator_10_machines_http") exampleBody = FLEET_EXAMPLE;
  // DeepSleep / Offline / Board already have examples — rewrite to stay consistent
  if (name === "Claim_https") {
    // keep CLAIM_CODE variant (already CLAIM_EXAMPLE)
  }

  const examplePath = path.join(dir, "secrets.h.example");
  fs.writeFileSync(examplePath, exampleBody, "utf8");

  for (const ino of inos) {
    const inoPath = path.join(dir, ino);
    let src = fs.readFileSync(inoPath, "utf8");
    const before = src;

    src = ensureInclude(src);
    src = stripStaticWifiDevice(src);
    src = patchSdkConfig(src);

    if (name === "FleetSimulator_10_machines_http") {
      src = src.replace(
        /"REPLACE_WITH_TOKEN_(\d+)"/g,
        (_, n) => `FLEET_TOKEN_${n}`
      );
      // FLEET_TOKEN_N are macros — need string without quotes in array
      // Was: "REPLACE_WITH_TOKEN_1" → FLEET_TOKEN_1 (macro expands to string literal)
    }

    if (name === "esp32-claim") {
      src = src.replace(
        /API_BASE\s*=\s*"[^"]*"/,
        'API_BASE = "https://api.autoconnecto.in"'
      );
    }

    if (src !== before) {
      fs.writeFileSync(inoPath, src, "utf8");
      report.push(`patched ${name}/${ino}`);
    } else {
      report.push(`ok (include ensured) ${name}/${ino}`);
      if (!/#include\s+"secrets\.h"/.test(fs.readFileSync(inoPath, "utf8"))) {
        // ensureInclude might have failed edge case
      }
      // always rewrite if missing include was added
      const now = fs.readFileSync(inoPath, "utf8");
      if (now !== before) {
        /* already written */
      } else if (!/#include\s+"secrets\.h"/.test(now)) {
        fs.writeFileSync(inoPath, ensureInclude(now), "utf8");
        report.push(`  +include ${name}/${ino}`);
      }
    }
  }
}

console.log(report.join("\n"));
console.log(`\nDone: ${folders.length} sketch folders`);
