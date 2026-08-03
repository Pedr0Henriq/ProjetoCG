#!/usr/bin/env bash
set -euo pipefail
if [ ! -f screenshot.ppm ]; then
  echo "Arquivo screenshot.ppm nao encontrado. Execute o programa e pressione P primeiro."
  exit 1
fi
if command -v magick >/dev/null 2>&1; then
  magick screenshot.ppm screenshot.png
elif command -v convert >/dev/null 2>&1; then
  convert screenshot.ppm screenshot.png
else
  echo "ImageMagick nao encontrado. O arquivo PPM continua valido e pode ser aberto em varios visualizadores."
  exit 1
fi
echo "Criado: screenshot.png"
