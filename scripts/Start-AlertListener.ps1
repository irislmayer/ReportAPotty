# --- HELP TO HIDE POWERSHELL WINDOW ---
$kernel32 = Add-Type -MemberDefinition '[DllImport("kernel32.dll")] public static extern IntPtr GetConsoleWindow();' -Name "Kernel32" -PassThru
$user32   = Add-Type -MemberDefinition '[DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);' -Name "User32" -PassThru
$hwnd     = $kernel32::GetConsoleWindow()
if ($hwnd -ne [IntPtr]::Zero) { [void]$user32::ShowWindow($hwnd, 0) } # 0 = SW_HIDE

$port = 8080
if ($udpClient) { $udpClient.Close() } 
$udpClient = New-Object System.Net.Sockets.UdpClient($port)

# Load the Windows Forms Library
[void] [System.Reflection.Assembly]::LoadWithPartialName("System.Windows.Forms")

Write-Host "==============================================" -ForegroundColor Green
Write-Host "PowerShell Taskbar Alert Server Live on port $port..." -ForegroundColor Green
Write-Host "Awaiting wireless alerts from your Arduino..."  -ForegroundColor Green
Write-Host "==============================================" -ForegroundColor Green

# Initialize the persistent System Tray Icon asset
$objNotifyIcon = New-Object System.Windows.Forms.NotifyIcon 
$objNotifyIcon.Icon = [System.Drawing.SystemIcons]::Information
$objNotifyIcon.Visible = $True 

try {
    while ($true) {
        # Halt execution and wait for an instant packet from Arduino
        $remoteIpEndPoint = New-Object System.Net.IPEndPoint([System.Net.IPAddress]::Any, 0)
        $receivedBytes = $udpClient.Receive([ref]$remoteIpEndPoint)
        
        # Read the text string (DEVICE_ID) sent by the Arduino
        $receivedString = [System.Text.Encoding]::ASCII.GetString($receivedBytes)

        Write-Host "[ALERT RECEIVED] Message from device: $receivedString" -ForegroundColor Yellow

        # Set up and trigger Notification layout parameters
        $objNotifyIcon.BalloonTipIcon = "None" 
        $objNotifyIcon.BalloonTipText = "$receivedString needs attention" 
        $objNotifyIcon.BalloonTipTitle = "Alert!"
        
        # Display the alert bubble 30 seconds
        $objNotifyIcon.ShowBalloonTip(30000)
    }
}
catch {
    Write-Error $_.Exception.Message
}
finally {
    # Cleanup resources upon manual termination
    if ($objNotifyIcon) { $objNotifyIcon.Visible = $False }
    $udpClient.Close()
}