Code for Dashboards 7 segment display and light bar.

7 segment used to display speed between 0-99 km/h.
Light bar used to display state of charge for high voltage battery.

## Running on the bench (integration)

This is an end-to-end, on-hardware run that exercises CAN + GPIO/SPI.

1. Build: `make`
2. Bring up CAN: `make can-up`
   The bitrate is set in `scripts/setup_can.sh` (example: 500 kbit/s):
   ```
   sudo ip link set can0 down || true
   sudo ip link set can0 type can bitrate 500000
   sudo ip link set can0 up
   ```
3. In another terminal, send test frames:
   - `make can-test`, or use `cansend` directly:
     - `cansend can0 002#E803000000000000` (speed)
     - `cansend can0 080#FFFF000000000000` (battery SOC full)
     - `cansend can0 080#0000000000000000` (battery SOC empty)

   You run make can-test in a separate terminal because it sends CAN frames and then exits; the dashboard needs to be running at the same time to receive those frames. If you don’t run make can-test, you can still launch the dashboard, but it will just sit idle waiting for real CAN traffic from your bus.

4. Launch the dashboard: `./dashboard`
   The dashboard porgram initializes GPIO/SPI, brings up its threads, opens the CAN socket, and then waits for incoming CAN frames to update the speed display and battery bar in real time.

## Testing

- Unit tests (pure logic, no hardware required): `make test`
   Unit tests can be run from any terminal in the repository. 
   
- Pi hardware smoke tests (requires WiringPi + hardware):
  - `make test-speed`
  - `make test-battery`

## Installing in the car

Automate the same steps so the Pi boots straight into the dashboard. You can either use `make install` or follow the manual steps below.

### Option A: Makefile install (recommended)

Run from the repo root on the Pi:
```
make install
```
This installs `/usr/local/bin/setup-can.sh` and `/etc/systemd/system/dashboard.service`, then enables the service.

### Option B: Manual setup

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

## Troubleshooting (on car / Pi)

Service health:
- `systemctl status dashboard.service`
- `journalctl -u dashboard.service -f`
- `sudo systemctl restart dashboard.service`

CAN interface:
- `ip link show can0`
- `sudo /usr/local/bin/setup-can.sh`
- `candump can0` (from `can-utils`) to confirm frames are arriving

Hardware init:
- Check wiring, power, and WiringPi installation if logs show GPIO/SPI errors.
- `make test-speed` and `make test-battery` are quick GPIO/SPI smoke tests.

App binary:
- Rebuild if you updated code: `make`
- Verify the service points to the right binary: `systemctl cat dashboard.service`
