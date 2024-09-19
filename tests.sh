# Get absolute path of the cb.sh script
cb_sh=$(realpath cb.sh)
# Get direcory of the cb.sh script
root_dir=$(dirname $cb_sh)

# Get absolute paths of the various cb.c files.
# read -r f is used to handle path containing spaces.
find $(realpath ./tests/) -iname "cb.c" | while read -r f 
do
  echo "Starting test for: $f" 
  file=$(printf "%s" "$f")
  # Get the parent directory path.
  # We strip every char from the end to the last slash
  # NOTE: dirname does not work very well with path containing spaces.
  dir=${file%/*}
  # Go to the cb.c file directory
  cd "$dir"
  # Execute cb.sh on the current cb.c
  $cb_sh --cxflags "-std=c89 -pedantic -Werror" || { exited=1; break; }
done

# Restore directory
cd $root_dir

if [ -v exited ]; then
  exit 1;
fi

exit 0