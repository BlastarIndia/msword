lib %1.lib,%1.lst
grep "^  " %1.lst | sed -e "s/  //" -e "s/  *$//" > foo$$
sed -f libstr.sed foo$$ > %1.str
rm %1.lst foo$$
