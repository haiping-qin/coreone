if [ $1 = "-u" ]; then
	input=$2
else
	input=$1
fi

[ ! -f $input ] && echo "Input configuration '$1' not existed" && exit 1

tmpfile=`mktemp`
tmpfile2=`mktemp`

echo "include $input" > $tmpfile
echo "all:;" >> $tmpfile

make -p -f $tmpfile | grep "^CONFIG_" | \
	sed -e "s/ =/=/g;s/= /=/g" | \
	sed -e "/=$/d;/=[nN]$/d;/-[-nN].*=/d;/-=/d" \
	    -e "/![^=nN-]/d" | \
	sed -e "s/-.*=/=/g" | sort -u > $tmpfile2

if [ $1 != "-u" ]; then
	cp $tmpfile2 $DOT_CONFIG
fi
cp $tmpfile2 $BUILD_CONFIG

cat $BUILD_CONFIG | \
	sed -e "s/^CONFIG_/#define CONFIG_/g;s/=y$/=1/g" | \
	sed -e "s/=/ /g" > $tmpfile

cmp -s $tmpfile $SRC_CONFIG
if [ $? -ne 0 ]; then
	echo "  GEN    $SRC_CONFIG"
	cp $tmpfile $SRC_CONFIG
fi

rm -f $tmpfile
rm -f $tmpfile2
