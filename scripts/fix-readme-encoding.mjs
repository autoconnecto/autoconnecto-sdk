import fs from "fs";
import path from "path";
import { fileURLToPath } from "url";

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "..");
const readmePath = path.join(root, "README.md");
let s = fs.readFileSync(readmePath, "utf8");

// Map common Unicode punctuation (and CP1252 mojibake of those) to ASCII.
const byCode = new Map([
  [0x00b7, " - "], // ·
  [0x2013, "-"], // –
  [0x2014, "-"], // —
  [0x2018, "'"],
  [0x2019, "'"],
  [0x201c, '"'],
  [0x201d, '"'],
  [0x2026, "..."],
  [0x2190, "<-"],
  [0x2192, "->"],
  [0x00d7, "x"],
  [0xfffd, "-"],
]);

s = [...s]
  .map((ch) => {
    const cp = ch.codePointAt(0);
    if (byCode.has(cp)) return byCode.get(cp);
    // Drop leftover C1 controls from broken UTF-8 reinterpretation
    if (cp >= 0x80 && cp <= 0x9f) return "";
    return ch;
  })
  .join("");

// Collapse leftover multi-byte mojibake runs (Â, â, etc. still present as Latin-1)
s = s.replace(/Â·/g, " - ");
s = s.replace(/Â /g, " ");
s = s.replace(/â[\u0080-\u00ff]{1,3}/g, "-");

s = s.replace(
  /### Option C[\s\S]*?(?=### Option D|## Quick Start)/,
  `### Option C - Arduino Library Manager

The library is listed in the Arduino Library Manager. In Arduino IDE: **Sketch > Include Library > Manage Libraries**, search **AutoconnectoSDK**, install.

Listing: [docs.arduino.cc/libraries/autoconnectosdk](https://docs.arduino.cc/libraries/autoconnectosdk/)

`
);

s = s.replace(/\*\*Version:\*\*\s*1\.7\.78/, "**Version:** 1.7.79");

s = s.replace(
  /\*\*Tested board:\*\*[^\n]+/,
  "**Tested board:** ESP32 (ESP32 Arduino core). This is the only architecture in `library.properties`. HTTPS example sketches for ESP8266 and Pico W exist in the repo but have not been run on those boards."
);

fs.writeFileSync(readmePath, s, "utf8");

const sample = s.slice(0, 500);
const bad = /[\u0080-\u009f\ufffd]/.test(s);
const stale =
  /after registration/i.test(s) || /Once this repository is public/i.test(s);
console.log(
  JSON.stringify({
    ok: !stale,
    bad,
    stale,
    length: s.length,
    head: sample.replace(/\n/g, " | "),
  })
);
