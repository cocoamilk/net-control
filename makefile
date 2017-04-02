ALL: datta-fw datta-bw gnet

CC-OPTIONS=-O2 -Wno-ignored-qualifiers -Wno-tautological-compare -std=c++11 -I./cparse-master cparse-master/core-shunting-yard.o cparse-master/builtin-features.o 

datta-fw: datta-fw.cc
	g++ $(CC-OPTIONS) datta-fw.cc -o datta-fw

datta-bw: datta-bw.cc
	g++ $(CC-OPTIONS) datta-bw.cc -o datta-bw

gnet: gnet.cc
	g++ $(CC-OPTIONS) gnet.cc -o gnet
