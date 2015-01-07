Select r.r_name
From lineitem l, partsupp p, supplier s, nation n,
	region r
Where l.l_partkey=p.ps_partkey and s.s_suppkey=p.ps_suppkey and
	l.l_suppkey=p.ps_suppkey and s.s_nationkey=n.n_nationkey
	and n.n_regionkey=r.r_regionkey and l.l_shipmode='TRUCK'
