# One-time sign-in for the counter's Spotify "now playing" and exact YouTube stats.
#
#   .\connect-accounts.ps1 -Spotify
#   .\connect-accounts.ps1 -YouTube
#
# It opens your browser so you can sign in and allow access, then shows a private
# "sign-in pass" (a refresh token) to paste into the counter's settings page.
# Everything happens between your PC and Spotify/Google - nothing is sent anywhere
# else, and the pass is only stored on the counter. README.md explains the setup.
param([switch]$Spotify, [switch]$YouTube)
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Web

if (-not $Spotify -and -not $YouTube) {
  Write-Output 'Run with -Spotify or -YouTube, e.g.  .\connect-accounts.ps1 -Spotify'
  exit 1
}

function ConvertTo-Base64Url([byte[]]$bytes) {
  [Convert]::ToBase64String($bytes).TrimEnd('=').Replace('+', '-').Replace('/', '_')
}

function New-RandomText {
  $bytes = New-Object byte[] 48
  [System.Security.Cryptography.RandomNumberGenerator]::Create().GetBytes($bytes)
  ConvertTo-Base64Url $bytes
}

function Get-Challenge([string]$verifier) {
  $hash = [System.Security.Cryptography.SHA256]::Create().ComputeHash([Text.Encoding]::ASCII.GetBytes($verifier))
  ConvertTo-Base64Url $hash
}

# Opens the sign-in page and waits for the browser to come back to this PC
function Wait-ForCode([string]$authUrl, [string]$state) {
  $tcp = [System.Net.Sockets.TcpListener]::new([System.Net.IPAddress]::Loopback, 8888)
  $tcp.Start()
  Start-Process $authUrl
  Write-Output 'Your browser is open - sign in and allow access...'
  try {
    while ($true) {
      $client = $tcp.AcceptTcpClient()
      $stream = $client.GetStream()
      $reader = New-Object System.IO.StreamReader($stream)
      $line = $reader.ReadLine()
      if ($line -notmatch '^GET /(callback)?\?([^ ]*)') { $client.Close(); continue }
      $query = [System.Web.HttpUtility]::ParseQueryString($matches[2])
      $ok = $query['code'] -and $query['state'] -eq $state
      $text = if ($ok) { 'Done! Close this tab and go back to the PowerShell window.' } else { "That didn't work: $($query['error'])" }
      $html = "<html><body style='font-family:sans-serif;background:#0b1026;color:#2ee6a8;padding:40px'><h2>$text</h2></body></html>"
      $body = [Text.Encoding]::UTF8.GetBytes($html)
      $head = [Text.Encoding]::ASCII.GetBytes("HTTP/1.1 200 OK`r`nContent-Type: text/html; charset=utf-8`r`nContent-Length: $($body.Length)`r`nConnection: close`r`n`r`n")
      $stream.Write($head, 0, $head.Length)
      $stream.Write($body, 0, $body.Length)
      $client.Close()
      if (-not $ok) { throw "Sign-in didn't finish ($($query['error']))" }
      return $query['code']
    }
  } finally {
    $tcp.Stop()
  }
}

function Show-Result([string]$what, [System.Collections.IDictionary]$fields) {
  Write-Output ''
  Write-Output "=== $what - copy these into the counter's settings page ==="
  Write-Output '(section "Spotify and exact YouTube stats")'
  foreach ($k in $fields.Keys) { Write-Output ("{0}: {1}" -f $k, $fields[$k]) }
  Write-Output ''
  Write-Output 'Keep the sign-in pass private - it lets the counter read your account.'
}

if ($Spotify) {
  $client = (Read-Host 'Spotify app Client ID').Trim()
  $redirect = 'http://127.0.0.1:8888/callback'
  $verifier = New-RandomText
  $state = New-RandomText
  $url = 'https://accounts.spotify.com/authorize?response_type=code' +
         '&client_id=' + [Uri]::EscapeDataString($client) +
         '&redirect_uri=' + [Uri]::EscapeDataString($redirect) +
         '&scope=' + [Uri]::EscapeDataString('user-read-currently-playing user-read-playback-state') +
         '&code_challenge_method=S256&code_challenge=' + (Get-Challenge $verifier) +
         '&state=' + $state
  $code = Wait-ForCode $url $state
  $token = Invoke-RestMethod -Method Post -Uri 'https://accounts.spotify.com/api/token' -Body @{
    grant_type = 'authorization_code'; code = $code; redirect_uri = $redirect; client_id = $client; code_verifier = $verifier
  }
  Show-Result 'Spotify' ([ordered]@{ 'Spotify app ID' = $client; 'Spotify sign-in pass' = $token.refresh_token })
}

if ($YouTube) {
  $client = (Read-Host 'Google OAuth Client ID').Trim()
  $secret = (Read-Host 'Google OAuth Client secret').Trim()
  $redirect = 'http://127.0.0.1:8888'
  $verifier = New-RandomText
  $state = New-RandomText
  $scopes = 'https://www.googleapis.com/auth/yt-analytics.readonly https://www.googleapis.com/auth/yt-analytics-monetary.readonly'
  $url = 'https://accounts.google.com/o/oauth2/v2/auth?response_type=code&access_type=offline&prompt=consent' +
         '&client_id=' + [Uri]::EscapeDataString($client) +
         '&redirect_uri=' + [Uri]::EscapeDataString($redirect) +
         '&scope=' + [Uri]::EscapeDataString($scopes) +
         '&code_challenge_method=S256&code_challenge=' + (Get-Challenge $verifier) +
         '&state=' + $state
  $code = Wait-ForCode $url $state
  $token = Invoke-RestMethod -Method Post -Uri 'https://oauth2.googleapis.com/token' -Body @{
    grant_type = 'authorization_code'; code = $code; redirect_uri = $redirect
    client_id = $client; client_secret = $secret; code_verifier = $verifier
  }
  if (-not $token.refresh_token) { throw 'Google did not give a sign-in pass. Remove the app at myaccount.google.com/permissions and try again.' }
  Show-Result 'YouTube' ([ordered]@{ 'Google client ID' = $client; 'Google client secret' = $secret; 'YouTube sign-in pass' = $token.refresh_token })
}

