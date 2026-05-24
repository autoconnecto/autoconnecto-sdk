#!/usr/bin/env bash
# Generic integration webhook — curl example
set -euo pipefail

API_BASE="${API_BASE:-https://api.autoconnecto.in}"
TENANT_ID="${TENANT_ID:?Set TENANT_ID}"
WEBHOOK_SECRET="${WEBHOOK_SECRET:?Set WEBHOOK_SECRET}"
DEVICE_TOKEN="${DEVICE_TOKEN:-}"
DEVICE_ID="${DEVICE_ID:-}"

URL="${API_BASE}/api/v1/integrations/generic/telemetry?tenantId=${TENANT_ID}"

if [[ -n "${DEVICE_TOKEN}" ]]; then
  BODY=$(cat <<EOF
{
  "deviceToken": "${DEVICE_TOKEN}",
  "temperature": 22.5,
  "humidity": 58,
  "ts": $(date +%s000)
}
EOF
)
else
  [[ -n "${DEVICE_ID}" ]] || { echo "Set DEVICE_TOKEN or DEVICE_ID"; exit 1; }
  BODY=$(cat <<EOF
{
  "externalDeviceId": "${DEVICE_ID}",
  "telemetry": {
    "temperature": 22.5,
    "humidity": 58
  }
}
EOF
)
fi

curl -sS -X POST "${URL}" \
  -H "Content-Type: application/json" \
  -H "X-Webhook-Secret: ${WEBHOOK_SECRET}" \
  -d "${BODY}" \
  -w "\nHTTP %{http_code}\n"
