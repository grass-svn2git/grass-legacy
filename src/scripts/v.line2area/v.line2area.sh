:
################################################################################
# *** This is v.line2area . ***
#
# @(#) takes lines and translates them to areas by taking the file out to ASCII,
# @(#) changing the 'L's to 'A's, then bringing the file back in and 
# @(#) resupporting with the original dig_cats file and an 'L' to 'A' changed
# @(#) dig_att file.
# Written by Rick Thompson (CAST)
#
################################################################################

if [ `uname -m` = mips ] ; then
  ECHON="/usr/bsd43/bin/echo -n"
elif [ `uname -s` = SunOS -a `uname -r | sed 's/\...*$//'` = 5 ] ; then
  ECHON="/usr/ucb/echo -n"
else
  ECHON="echo -n"
fi

clear
echo ''
if [ $# = 1 ] ; then
  name="$1"
else
  g.ask type=old prompt="Which vector map to translate?" element=dig desc=vector unixfile=/tmp/$$
  . /tmp/$$
  rm -f /tmp/$$
  if [ -z $name ]; then
    echo "Vector map $name not found."
    exit
  fi
fi

################################################################################

v.out.ascii in=$name out=$name
cat $LOCATION/dig_ascii/$name | sed 's/L  /A  /' > $LOCATION/dig_ascii/$name.2
v.in.ascii in=$name.2 out=$name.2
cat $LOCATION/dig_att/$name | sed 's/L/A/' > $LOCATION/dig_att/$name.2
cp $LOCATION/dig_cats/$name $LOCATION/dig_cats/$name.2
v.support -r $name.2 op=build
rm $LOCATION/dig_ascii/$name $LOCATION/dig_ascii/$name.2

echo ''
$ECHON "Want to rename $name.2 to $name? (n) "
read ans
if [ "$ans" = "Y" -o "$ans" = "y" ] ; then
  g.rename vect=$name.2,$name
  echo ''
  echo "$name now contains areas."
else
  clear
  echo ''
  echo "$name.2 now contains areas."
fi
