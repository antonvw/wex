#!/bin/bash

files="*.po"

function determine_files
{
  find ../ -type d \( -path ../external -o -path ../test \) -prune -o -type f \( -name *.cpp -o -name *.h \) -print > locs
}

function create_mo()
{
  determine_files

  # merge (-j join) the po files
  for f in $files; do
    if [[ -f $f ]]; then
      xgettext -s -j -k_ -o "$f" --no-location --copyright-holder="A.M. van Wezenbeek" -f locs
      substitute "$f"
    else
      echo $f does not exist
    fi
  done
}

function create_pot()
{
  determine_files
  xgettext -s -k_ -o wex.pot --no-location --copyright-holder="A.M. van Wezenbeek" -f locs
}

function substitute
{
  sed -e "s/SOME.*/wex localization file $1/" -i "$1"
  sed -e "s/YEAR/2017-2025/" -i "$1"
  sed -e "s/PACKAGE/wex/" -i "$1"
  sed -e "s/VERSION/25.10/" -i "$1"
  sed -e "/FIRST AUTHOR.*/d" -i "$1"
  sed -e "s/charset=CHARSET/charset=UTF-8/" -i "$1"
}

function usage
{
  echo "usage: po-sync.sh [-hmp] [file]"
  echo "-m     sync all po files"
  echo "-p     create pot file"
  echo "[file] sync specified po file"
}

while getopts "hmp" opt; do
  case $opt in
    h)
      usage
      exit 1
    ;;

    m)
      create_mo
    ;;

    p)
      create_pot
    ;;

    \?)
      echo "illegal option -$OPTARG"
      exit 1
    ;;
  esac
done

# Now handle positional arguments
shift $((OPTIND - 1))
files=$1  # First positional argument

if [[ -n "$files" ]]; then
  create_mo
fi

