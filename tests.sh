
# Get absolute path of the cb.sh script
cb_sh=$(realpath cb.sh)
# Get direcory of the cb.sh script
root_dir=$(dirname $cb_sh)

# Get absolute paths of the various cb.c files/ 
for f in $(find $(realpath .)/tests/ -iname "cb.c") ; do
  # Go to the cb.c directory
  cd $(dirname $f)
  # Execute cb.sh on the current cb.c
  $cb_sh
  
done

# Restore directory
cd $root_dir