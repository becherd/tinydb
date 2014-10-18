#include "Database.hpp"
#include "operator/Tablescan.hpp"
#include "operator/CrossProduct.hpp"
#include "operator/Selection.hpp"
#include "operator/Projection.hpp"
#include "operator/Printer.hpp"
#include "operator/Chi.hpp"
#include <iostream>
#include <string>
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
int main()
{
   Database db;
   db.open("data/uni");
   Table& studenten=db.getTable("studenten");
   Table& hoeren=db.getTable("hoeren");


	//from Studenten s1, Studenten s2, hoeren h1, hoeren h2
   	unique_ptr<Tablescan> scanStudenten1(new Tablescan(studenten));
	unique_ptr<Tablescan> scanStudenten2(new Tablescan(studenten));
    unique_ptr<Tablescan> scanHoeren1(new Tablescan(hoeren));
	unique_ptr<Tablescan> scanHoeren2(new Tablescan(hoeren));

   	const Register* name1=scanStudenten1->getOutput("name");
   	const Register* name2=scanStudenten2->getOutput("name");
	const Register* matrNr1=scanStudenten1->getOutput("matrnr");
	const Register* matrNr2=scanStudenten2->getOutput("matrnr");
	const Register* matrNrHoeren1=scanHoeren1->getOutput("matrnr");
	const Register* matrNrHoeren2=scanHoeren2->getOutput("matrnr");
	const Register* vorlNrHoeren1=scanHoeren1->getOutput("vorlnr");
	const Register* vorlNrHoeren2=scanHoeren2->getOutput("vorlnr");



	//where s1.Name='Schopenhauer'
	std::string name = "Schopenhauer";
	Register* n = new Register();
	n->setString(name);
	unique_ptr<Selection> select(new Selection(move(scanStudenten1),name1,n));
	
 	//and s1.MatrNr=h1.MatrNr
	unique_ptr<CrossProduct> cp(new CrossProduct(move(select),move(scanHoeren1)));
   	unique_ptr<Selection> select2(new Selection(move(cp),matrNr1,matrNrHoeren1));

 	//and h1.VorlNr=h2.VorlNr 
	unique_ptr<CrossProduct> cp2(new CrossProduct(move(select2),move(scanHoeren2)));
   	unique_ptr<Selection> select3(new Selection(move(cp2),vorlNrHoeren1,vorlNrHoeren2));

	// and h1.MatrNr!=h2.MatrNr 
   unique_ptr<Chi> chi(new Chi(move(select3),Chi::NotEqual,matrNrHoeren1,matrNrHoeren2));
   const Register* chiResult=chi->getResult();
   unique_ptr<Selection> select4(new Selection(move(chi),chiResult));

	//and h2.MatrNr = s2.MatrNr
	unique_ptr<CrossProduct> cp3(new CrossProduct(move(select4),move(scanStudenten2)));
   	unique_ptr<Selection> select5(new Selection(move(cp3),matrNrHoeren2,matrNr2));

	//select s2.Name
   unique_ptr<Projection> project(new Projection(move(select5),{name2}));


   Printer out(move(project));

   out.open();
   while (out.next());
   out.close();

}
//---------------------------------------------------------------------------
