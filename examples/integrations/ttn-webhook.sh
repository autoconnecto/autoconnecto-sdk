#!/usr/bin/env bash
# Post a TTN / The Things Stack v3 style uplink.
set -euo pipefail

API_BASE="${API_BASE:-https://api.autoconnecto.in}"
TENANT_ID="${TENANT_ID:?Set TENANT_ID}"
WEBHOOK_SECRET="${WEBHOOK_SECRET:?Set WEBHOOK_SECRET}"
DEV_EUI="${DEV_EUI:?Set DEV_EUI}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BODY_FILE="${SCRIPT_DIR}/ttn-uplink.sample.json"

if command -v jq >/dev/null 2>&1; then
  BODY=$(jq --arg eui "$DEV_EUI" '.end_device_ids.dev_eui = $eui' "$BODY_FILE")
else
  BODY=$(sed "s/0102030405060708/${DEV_EUI}/" "$BODY_FILE")
fi

URL="${API_BASE}/api/v1/integrations/ttn/telemetry?tenantId=${TENANT_ID}"

curl -sS -X POST "${URL}" \
  -H "Content-Type: application/json" \
  -H "X-Webhook-Secret: ${WEBHOOK_SECRET}" \
  -d "${BODY}" \
  -w "\nHTTP %{http_code}\n"
