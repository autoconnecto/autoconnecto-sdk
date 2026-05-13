$ErrorActionPreference = 'Stop'

$brokerHost = 'mqtt.autoconnecto.in'
$brokerPort = 8084

$tcp = New-Object Net.Sockets.TcpClient
$tcp.Connect($brokerHost, $brokerPort)

$ssl = New-Object Net.Security.SslStream(
  $tcp.GetStream(),
  $false,
  ([Net.Security.RemoteCertificateValidationCallback]{ param($s,$c,$ch,$e) return $true })
)
$ssl.AuthenticateAsClient($brokerHost)

$cert = New-Object Security.Cryptography.X509Certificates.X509Certificate2($ssl.RemoteCertificate)

Write-Host '=== LEAF CERT (server is presenting this) ==='
Write-Host ('Subject:    ' + $cert.Subject)
Write-Host ('Issuer:     ' + $cert.Issuer)
Write-Host ('NotBefore:  ' + $cert.NotBefore.ToString('u'))
Write-Host ('NotAfter:   ' + $cert.NotAfter.ToString('u'))
Write-Host ('Thumbprint: ' + $cert.Thumbprint)
Write-Host ('Serial:     ' + $cert.SerialNumber)

Write-Host ''
Write-Host '=== SAN (Subject Alternative Names) ==='
foreach ($ext in $cert.Extensions) {
  if ($ext.Oid.Value -eq '2.5.29.17') {
    $asn = New-Object Security.Cryptography.AsnEncodedData($ext.Oid, $ext.RawData)
    Write-Host $asn.Format($false)
  }
}

$chain = New-Object Security.Cryptography.X509Certificates.X509Chain
$chain.ChainPolicy.RevocationMode = 'NoCheck'
[void]$chain.Build($cert)

Write-Host ''
Write-Host '=== CHAIN (as resolved by Windows trust store) ==='
for ($i = 0; $i -lt $chain.ChainElements.Count; $i++) {
  $e = $chain.ChainElements[$i].Certificate
  Write-Host ("[$i] Subject: " + $e.Subject)
  Write-Host ('    Issuer:  ' + $e.Issuer)
  Write-Host ('    NotAfter:' + $e.NotAfter.ToString('u'))
}

$tcp.Close()
