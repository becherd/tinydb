Select c.c_name, c.c_address
From lineitem l, customer c, partsupp p, supplier s, orders o
Where c.c_address=s.s_address and c.c_name=s.s_name and l.l_orderkey=o.o_orderkey
	and o.o_custkey=c.c_custkey and l.l_partkey=p.ps_partkey and 
	l.l_suppkey=p.ps_suppkey and p.ps_suppkey=s.s_suppkey
