def test_turn_off_wifi(shell):
    shell.run_check("sudo killall wpa_supplicant")
    shell.run_check("sudo ip link set wlan0 down")
    shell.run_check("sudo systemctl stop dnsmasq")
