# Get absolute path of the cb.sh script
cb_sh=$(realpath cb.sh)
# Get direcory of the cb.sh script
root_dir=$(dirname $cb_sh)

shopt -s globstar
for f in ./tests/**/cb.c; do
  echo "Starting test for: $f" 
  # Get absolute path
  file=$(realpath "$f")
  # Get the parent directory path.
  # We strip every char from the end to the last slash
  # NOTE: dirname does not work very well with path containing spaces.
  dir=${file%/*}
  # Go to the cb.c file directory
  cd "$dir"
  # Execute cb.sh on the current cb.c, apprently we need to use the $((XXX)) to assigne a variable in a for loop.
  $cb_sh --pedantic || { exit_code=$((1)); break; }
  # Restore original directory to get the correct absolute path
  cd $root_dir
done

if [ -v exit_code ]; then
  exit $exit_code
fi

