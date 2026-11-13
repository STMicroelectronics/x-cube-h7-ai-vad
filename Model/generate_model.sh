#!/bin/bash
# Requires: ST Edge AI Core v4.1.0
#   STM32CubeAI 12.1.0 - versions as reported by the generated reports.
# Override the tool location with the STEDGEAI env var if it moves.

set -euo pipefail

STEDGEAI="${STEDGEAI:-stedgeai}"

generate_variant()
{
  model="$1"
  variant="$2"

  # Pre-processing writes frame-major features directly into input 0.
  # All-float32 IO: the feature tensor is kept float32 so the application
  # writes features directly, with no app-side quantization. The 3x41
  # float32 input tensor is 492 B per variant (123 values x 4 B).
  "$STEDGEAI" generate -m "$model" \
    --target stm32h7 -O time \
    --input-data-type float32 \
    --output-data-type float32 \

  mkdir -p "$variant"
  cp st_ai_output/network.c "$variant/network.c"
  cp st_ai_output/network.h "$variant/network.h"
  cp st_ai_output/network_data.c "$variant/network_data.c"
  cp st_ai_output/network_data.h "$variant/network_data.h"
  cp st_ai_output/network_details.h "$variant/network_details.h"
  cp st_ai_output/network_c_info.json "$variant/network_c_info.json"
  cp st_ai_output/network_generate_report.txt "$variant/network_generate_report.txt"
}

generate_variant ten-vad-fixed_dec_partial_qdq_w8a8.onnx partial
generate_variant ten-vad-fixed_dec_full_qdq_w8a8.onnx full
