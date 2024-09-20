# OIFS and IFS are used to process path with space
OIFS="$IFS"
IFS=$'\n'

# Get absolute path of the cb.sh script
cb_sh=$(realpath cb.sh)
# Get direcory of the cb.sh script
root_dir=$(dirname $cb_sh)

# Get absolute paths of the various cb.c files.
for f in $(find $(realpath ./tests/) -iname "cb.c" ) ;
do 
  echo "Starting test for: $f" 
  file=$(printf "%s" "$f")
  # Get the parent directory path.
  # We strip every char from the end to the last slash
  # NOTE: dirname does not work very well with path containing spaces.
  dir=${file%/*}
  # Go to the cb.c file directory
  cd "$dir"
  # Execute cb.sh on the current cb.c, apprently we need to use the $((XXX)) to assigne a variable in a for loop.
  $cb_sh --cxflags -std=c89 -pedantic -Werror || { exit_code=$((1)); break; }
done

IFS="$OIFS"

if [ -v exit_code ]; then
  exit $exit_code
fi


