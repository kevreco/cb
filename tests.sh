# Get absolute path of the cb.sh script
cb_sh=$(realpath cb.sh)

shopt -s globstar
for f in ./tests/**/cb.c; do
  echo "Starting test for: $f" 
  $cb_sh --pedantic --file  "$(realpath "$f")" || { exit 1; }
done