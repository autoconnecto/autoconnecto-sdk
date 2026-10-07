# Gateway relay sample (PowerShell).
# Default: POST /api/v1/{GATEWAY_TOKEN}/telemetry with childDeviceId envelope.
# Set $env:MODE = "webhook" plus TENANT_ID / WEBHOOK_SECRET for generic integration path.

$ErrorActionPreference = "Stop"

$ApiBase = if ($env:API_BASE) { $env:API_BASE } else { "https://api.autoconnecto.in" }
$GatewayToken = $env:GATEWAY_TOKEN
$ChildDeviceId = $env:CHILD_DEVICE_ID
$Mode = if ($env:MODE) { $env:MODE } else { "device" }

if (-not $GatewayToken) { throw "Set GATEWAY_TOKEN" }
if (-not $ChildDeviceId) { throw "Set CHILD_DEVICE_ID" }

if ($Mode -eq "webhook") {
  $TenantId = $env:TENANT_ID
  $Secret = $env:WEBHOOK_SECRET
  if (-not $TenantId) { throw "Set TENANT_ID for webhook mode" }
  if (-not $Secret) { throw "Set WEBHOOK_SECRET for webhook mode" }
  $Url = "$ApiBase/api/v1/integrations/generic/telemetry?tenantId=$TenantId"
  $Body = @{
    deviceToken    = $GatewayToken
    childDeviceId  = $ChildDeviceId
    telemetry      = @{ temperature = 24.1; humidity = 51 }
  } | ConvertTo-Json -Depth 5
  Invoke-RestMethod -Method Post -Uri $Url -Headers @{
    "Content-Type"      = "application/json"
    "X-Webhook-Secret"  = $Secret
  } -Body $Body
} else {
  $Url = "$ApiBase/api/v1/$GatewayToken/telemetry"
  $Body = @{
    childDeviceId = $ChildDeviceId
    telemetry     = @{ temperature = 24.1; humidity = 51 }
  } | ConvertTo-Json -Depth 5
  Invoke-RestMethod -Method Post -Uri $Url -ContentType "application/json" -Body $Body
}

Write-Host "OK"
