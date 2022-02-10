#! /bin/bash

cd /var/www/G-Pascal

# Note: the line with -ot (older than) is doing a modified-time check
#       and also a test for if the destination (htm) file exists at all.

for FILENAME in assembler hardware_mods history index pascal_compiler adventure
do
  if [ "${FILENAME}.htm" -ot "markdown/${FILENAME}.md" ] || [ ! -f "${FILENAME}.htm" ]; then
        echo "Doing $FILENAME"
        pandoc --no-highlight \
               -f markdown+smart --standalone -t html5\
               --css="/G-Pascal/G-Pascal.css" \
               --metadata pagetitle="$FILENAME"  \
               "markdown/${FILENAME}.md" -o "${FILENAME}.htm"
      fi
done

