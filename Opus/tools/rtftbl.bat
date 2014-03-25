copy \cashmere\rtftbl.h \cashmere\rtftbl.bak
rtfgen >foo
diff foo rtftbl.h > \cashmere\foo2
more \cashmere\foo2
mv foo \cashmere\rtftbl.h

