#!/bin/bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
SERVICE_TEMPLATE="$ROOT_DIR/systemd/dashboard.service"
SERVICE_PATH="/etc/systemd/system/dashboard.service"
SETUP_CAN_SRC="$ROOT_DIR/scripts/setup_can.sh"
SETUP_CAN_DST="/usr/local/bin/setup-can.sh"

if [[ ! -f "$SERVICE_TEMPLATE" ]]; then
    echo "Missing service template: $SERVICE_TEMPLATE" >&2
    exit 1
fi

if [[ ! -f "$SETUP_CAN_SRC" ]]; then
    echo "Missing setup-can script: $SETUP_CAN_SRC" >&2
    exit 1
fi

install -m 755 "$SETUP_CAN_SRC" "$SETUP_CAN_DST"
sed "s|@WORKDIR@|$ROOT_DIR|g" "$SERVICE_TEMPLATE" > "$SERVICE_PATH"

systemctl daemon-reload
systemctl enable --now dashboard.service
echo "Installed and started dashboard.service"
