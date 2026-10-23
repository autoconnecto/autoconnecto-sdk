import fs from "fs";
import path from "path";
import { fileURLToPath } from "url";

const root = path.join(path.dirname(fileURLToPath(import.meta.url)), "..", "examples");

function walk(dir, pred, out = []) {
  for (const e of fs.readdirSync(dir, { withFileTypes: true })) {
    const p = path.join(dir, e.name);
    if (e.isDirectory()) walk(p, pred, out);
    else if (pred(e.name)) out.push(p);
  }
  return out;
}

const inos = walk(root, (n) => n.endsWith(".ino"));
const examples = walk(root, (n) => n === "secrets.h.example");
const missing = inos.filter(
  (f) => !/#include\s+"secrets\.h"/.test(fs.readFileSync(f, "utf8"))
);
const inline = inos.filter((f) =>
  /config\.(wifiSSID|wifiPassword|deviceToken|lteApn)\s*=\s*"/.test(
    fs.readFileSync(f, "utf8")
  )
);
const staticCred = inos.filter((f) =>
  /static const char\*\s*(WIFI_SSID|WIFI_PASSWORD|DEVICE_TOKEN)\s*=/.test(
    fs.readFileSync(f, "utf8")
  )
);

console.log("inos", inos.length);
console.log("secrets.h.example", examples.length);
console.log("missing include", missing.map((p) => path.relative(root, p)));
console.log("inline config strings", inline.map((p) => path.relative(root, p)));
console.log("static cred leftovers", staticCred.map((p) => path.relative(root, p)));
