#!/bin/bash

echo "Bringing CAN interface down (if up)..."
sudo ip link set can0 down 2>/dev/null || true

echo "Setting CAN bitrate to 500k..."
sudo ip link set can0 type can bitrate 500000

echo "Bringing CAN interface up..."
sudo ip link set can0 up

echo "CAN interface status:"
ip -details link show can0
