select s1.name
from studenten s1, hoeren h1, hoeren h2, studenten s2
where s2.name='Fichte' and s2.matrnr=h2.matrnr and h2.vorlnr=h1.vorlnr and h1.matrnr=s1.matrnr
