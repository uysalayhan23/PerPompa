#!/usr/bin/env bash
# Purely local SHA256 verification; never uploads or programs an Arduino.
set -euo pipefail
cd "$(dirname "$0")"
echo 'b9d04926776ce3703043c507fa804c6cc6ff6ca494f84df6b6926b37ff3d38f2  read_only_candidate.tar.gz' | sha256sum -c -
out="$(mktemp -d)"
trap 'rm -rf "$out"' EXIT
tar -xzf read_only_candidate.tar.gz -C "$out"
echo 'e0d852757f74fedc4c00dd9da05b9cf02bf660c72bf7dcaa4afc52c8a7710e21  '"$out"'/Peristaltic_Pump_Software_v1.06/Peristaltic_Pump_Software_v1.06.ino' | sha256sum -c -
printf '%s\n' 'P004_REVIEW_SOURCE_SHA256_PASS;NOT_PRODUCTION;NO_UPLOAD'
