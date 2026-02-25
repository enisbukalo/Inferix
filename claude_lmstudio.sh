#!/usr/bin/env bash
# Set up the API endpoint for LM Studio
export ANTHROPIC_BASE_URL=http://192.168.50.69:1234
export ANTHROPIC_AUTH_TOKEN=lmstudio
LM_API_TOKEN=lmstudio
LM_HOST=http://192.168.50.69:1234

# Check if LM Studio server is running
echo "Checking if LM Studio server is running..."
server_status=$(lms server status 2>&1)
if echo "$server_status" | grep -q "port 1234"; then
  echo "The server is running on port 1234."
else
  echo "The server is not running."
  echo "Please start the LM Studio server before running this script."
  exit 1
fi

# Helper: get LOADED model IDs using /api/v0/models (has "state" field)
get_loaded_models() {
  curl -sf "$LM_HOST/api/v0/models" -H "Authorization: Bearer $LM_API_TOKEN" \
    | python3 -c "
import sys, json
try:
    data = json.load(sys.stdin)
    for m in data.get('data', []):
        if m.get('state') == 'loaded' and m.get('type') == 'llm':
            print(m.get('id', ''))
except Exception as e:
    sys.stderr.write(f'Error parsing /api/v0/models: {e}\n')
"
}

# Helper: get all downloaded LLMs from /api/v1/models
# Outputs: key<TAB>display_name per line
get_downloaded_models() {
  curl -sf "$LM_HOST/api/v1/models" -H "Authorization: Bearer $LM_API_TOKEN" \
    | python3 -c "
import sys, json
try:
    data = json.load(sys.stdin)
    for m in data.get('models', []):
        if m.get('type') == 'llm':
            key = m.get('key', '')
            display = m.get('display_name') or key
            print(f'{key}\t{display}')
except Exception as e:
    sys.stderr.write(f'Error parsing /api/v1/models: {e}\n')
"
}

# Check for loaded models
echo "Checking for loaded models..."
loaded_models=$(get_loaded_models)

if [ -z "$loaded_models" ]; then
  echo "No models are currently loaded in LM Studio."
  echo ""
  echo "Here are the LLM models you have downloaded:"
  echo ""
  downloaded=$(get_downloaded_models)
  if [ -z "$downloaded" ]; then
    echo "Error: Could not retrieve downloaded models from LM Studio."
    exit 1
  fi
  i=1
  while IFS=$'\t' read -r key display; do
    printf "%d\t%s\n" "$i" "$display"
    i=$((i+1))
  done <<< "$downloaded"
  echo ""
  read -p "Enter the number of the model you want to load: " model_choice
  if ! [[ "$model_choice" =~ ^[0-9]+$ ]]; then
    echo "Error: Please enter a valid number."
    exit 1
  fi
  selected_key=$(echo "$downloaded" | sed -n "${model_choice}p" | cut -f1)
  selected_display=$(echo "$downloaded" | sed -n "${model_choice}p" | cut -f2)
  if [ -z "$selected_key" ]; then
    echo "Error: Invalid model selection."
    exit 1
  fi
  echo "Loading model: $selected_display ($selected_key)"
  echo "(This may take a while — the API blocks until the model is fully loaded...)"
  load_response=$(curl -s -w "\n%{http_code}" "$LM_HOST/api/v1/models/load" \
    -H "Authorization: Bearer $LM_API_TOKEN" \
    -H "Content-Type: application/json" \
    -d "{\"model\": \"$selected_key\"}")
  load_http_code=$(echo "$load_response" | tail -n1)
  load_body=$(echo "$load_response" | head -n -1)
  if [ "$load_http_code" != "200" ]; then
    echo "Error: Failed to load model (HTTP $load_http_code)"
    echo "$load_body"
    exit 1
  fi
  echo "Model $selected_display loaded successfully."
  selected_model="$selected_key"
else
  echo ""
  echo "Available loaded models:"
  echo ""
  echo "$loaded_models" | nl -nln
  echo ""
  read -p "Enter the number of the model you want to use: " model_choice
  if ! [[ "$model_choice" =~ ^[0-9]+$ ]]; then
    echo "Error: Please enter a valid number."
    exit 1
  fi
  num_models=$(echo "$loaded_models" | wc -l)
  if [ "$model_choice" -lt 1 ] || [ "$model_choice" -gt "$num_models" ]; then
    echo "Error: Please enter a number between 1 and $num_models."
    exit 1
  fi
  selected_model=$(echo "$loaded_models" | sed -n "${model_choice}p")
  if [ -z "$selected_model" ]; then
    echo "Error: Invalid model selection."
    exit 1
  fi
fi

echo ""
echo "Using model: $selected_model"
read -p "Are you sure you want to run 'claude --model $selected_model .'? (y/N): " confirm
if [[ ! "$confirm" =~ ^[Yy] ]]; then
  echo "Operation cancelled."
  exit 0
fi
claude --model "$selected_model" .
