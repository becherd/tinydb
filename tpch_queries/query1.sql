Select c.c_name
From orders o, customer c, lineitem l, partsupp p, supplier s
Where l.l_orderkey=o.o_orderkey and o.o_custkey=c.c_custkey
	and p.ps_partkey=l.l_partkey and l.l_suppkey=p.ps_suppkey and
	s.s_nationkey=c.c_nationkey and p.ps_suppkey=s.s_suppkey
