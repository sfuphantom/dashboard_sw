Code for Dashboards 7 segment display and light bar.

7 segment used to display speed between 0-99 km/h.
Light bar used to display state of charge for high voltage battery.

## Running on the bench

1. Build: `make`
2. Configure CAN for your desired bitrate (example uses 500 kbit/s):
   ```
   sudo ip link set can0 down || true
   sudo ip link set can0 type can bitrate 500000
   sudo ip link set can0 up
   ```
3. Launch the dashboard: `./dashboard`
4. Inject frames with `cansend` from `can-utils`, e.g.  
   `cansend can0 002#1027000000000000` (speed)  
   `cansend can0 080#FFFF000000000000` (battery SOC)

## Installing in the car

Automate the same steps so the Pi boots straight into the dashboard:

1. Create `/usr/local/bin/setup-can.sh`:
   ```sh
   #!/bin/bash
   ip link set can0 down 2>/dev/null
   ip link set can0 type can bitrate 500000
   ip link set can0 up
   ```
   Make it executable: `sudo chmod +x /usr/local/bin/setup-can.sh`

2. Create `/etc/systemd/system/dashboard.service`:
   ```
   [Unit]
   Description=Dashboard display
   After=network.target

   [Service]
   WorkingDirectory=/home/pi/Developer/dashboard_sw
   ExecStartPre=/usr/local/bin/setup-can.sh
   ExecStart=/home/pi/Developer/dashboard_sw/dashboard
   Restart=always

   [Install]
   WantedBy=multi-user.target
   ```

3. Enable the service so it runs every boot:
   ```
   sudo systemctl daemon-reload
   sudo systemctl enable --now dashboard.service
   ```

Boot sequence afterward:
- Pi powers up and Linux starts.
- `setup-can.sh` configures `can0`.
- systemd launches `dashboard`, restarts it on failure, and the dash immediately begins receiving CAN frames from the BMS/VCU.
