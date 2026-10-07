#!/usr/bin/env bash
# Gateway relay via generic integration webhook (or device-token HTTP).
#
# Preferred device-token path (matches GatewayRelay_http.ino):
#   POST /api/v1/{GATEWAY_TOKEN}/telemetry
#   { "childDeviceId": "<uuid>", "telemetry": { ... } }
#
# This script also shows the generic webhook variant when you already have a
# tenant webhook secret and are posting through Integrations → Generic HTTP.
set -euo pipefail

API_BASE="${API_BASE:-https://api.autoconnecto.in}"
GATEWAY_TOKEN="${GATEWAY_TOKEN:?Set GATEWAY_TOKEN (gateway device access token)}"
CHILD_DEVICE_ID="${CHILD_DEVICE_ID:?Set CHILD_DEVICE_ID (child device UUID)}"

# Mode: device (default) | webhook
MODE="${MODE:-device}"

BODY=$(cat <<EOF
{
  "childDeviceId": "${CHILD_DEVICE_ID}",
  "telemetry": {
    "temperature": 24.1,
    "humidity": 51
  }
}
EOF
)

if [[ "${MODE}" == "webhook" ]]; then
  TENANT_ID="${TENANT_ID:?Set TENANT_ID for webhook mode}"
  WEBHOOK_SECRET="${WEBHOOK_SECRET:?Set WEBHOOK_SECRET for webhook mode}"
  # Generic adapter accepts deviceToken + childDeviceId for gateway relay.
  BODY=$(cat <<EOF
{
  "deviceToken": "${GATEWAY_TOKEN}",
  "childDeviceId": "${CHILD_DEVICE_ID}",
  "telemetry": {
    "temperature": 24.1,
    "humidity": 51
  }
}
EOF
)
  URL="${API_BASE}/api/v1/integrations/generic/telemetry?tenantId=${TENANT_ID}"
  curl -sS -X POST "${URL}" \
    -H "Content-Type: application/json" \
    -H "X-Webhook-Secret: ${WEBHOOK_SECRET}" \
    -d "${BODY}" \
    -w "\nHTTP %{http_code}\n"
else
  URL="${API_BASE}/api/v1/${GATEWAY_TOKEN}/telemetry"
  curl -sS -X POST "${URL}" \
    -H "Content-Type: application/json" \
    -d "${BODY}" \
    -w "\nHTTP %{http_code}\n"
fi
