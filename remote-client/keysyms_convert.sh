#!/bin/sh

OUT=vnc_keysyms_unicode.h

rm -f $OUT

echo "static struct { uint32_t X; uint32_t unicode; } XKeysymToUnicode[] = {" >> $OUT

cat keysyms.txt | grep '^0x' | while read KEYSYM UNICODE TYPE COMMENT; do
	[ "$TYPE" = "o" ] && continue
	[ "$TYPE" = "f" ] && continue
	[ "$TYPE" = "r" ] && continue
	[ "$TYPE" = "" ] && continue
	UNICODE="`echo $UNICODE | sed 's/^U//'`"
	echo "	{ $KEYSYM, 0x$UNICODE }," >> $OUT
done

echo "	{ 0, 0 } };" >> $OUT
