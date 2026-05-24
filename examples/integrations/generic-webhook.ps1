# Generic integration webhook — PowerShell example
param(
  [string]$ApiBase = $env:API_BASE,
  [string]$TenantId = $env:TENANT_ID,
  [string]$WebhookSecret = $env:WEBHOOK_SECRET,
  [string]$DeviceToken = $env:DEVICE_TOKEN,
  [string]$DeviceId = $env:DEVICE_ID
)

if (-not $ApiBase) { $ApiBase = "https://api.autoconnecto.in" }
if (-not $TenantId) { throw "Set TENANT_ID" }
if (-not $WebhookSecret) { throw "Set WEBHOOK_SECRET" }

$uri = "$ApiBase/api/v1/integrations/generic/telemetry?tenantId=$TenantId"

if ($DeviceToken) {
  $body = @{
    deviceToken = $DeviceToken
    temperature = 22.5
    humidity    = 58
    ts          = [int64]([DateTimeOffset]::UtcNow.ToUnixTimeMilliseconds())
  } | ConvertTo-Json
} elseif ($DeviceId) {
  $body = @{
    externalDeviceId = $DeviceId
    telemetry        = @{ temperature = 22.5; humidity = 58 }
  } | ConvertTo-Json -Depth 4
} else {
  throw "Set DEVICE_TOKEN or DEVICE_ID"
}

$headers = @{
  "Content-Type"     = "application/json"
  "X-Webhook-Secret" = $WebhookSecret
}

Invoke-RestMethod -Method Post -Uri $uri -Headers $headers -Body $body
