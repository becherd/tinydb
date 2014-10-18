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
   Table& professoren=db.getTable("professoren");
   Table& hoeren=db.getTable("hoeren");
   Table& vorlesungen=db.getTable("vorlesungen");


	//from Professoren p, Vorlesungen v, hoeren h1, hoeren h2
   	unique_ptr<Tablescan> scanProfessoren(new Tablescan(professoren));
	unique_ptr<Tablescan> scanVorlesungen(new Tablescan(vorlesungen));
	unique_ptr<Tablescan> scanHoeren1(new Tablescan(hoeren));
	unique_ptr<Tablescan> scanHoeren2(new Tablescan(hoeren));


   	const Register* persNr=scanProfessoren->getOutput("persnr");
   	const Register* name=scanProfessoren->getOutput("name");
   	const Register* vorlNr=scanVorlesungen->getOutput("vorlnr");
   	const Register* gelesenVon=scanVorlesungen->getOutput("gelesenvon");
   	const Register* hVorlNr1=scanHoeren1->getOutput("vorlnr");
   	const Register* hVorlNr2=scanHoeren2->getOutput("vorlnr");
   	const Register* hMatrNr1=scanHoeren1->getOutput("matrnr");
   	const Register* hMatrNr2=scanHoeren2->getOutput("matrnr");




	
	//where p.PersNr=v.gelesenVon
	unique_ptr<CrossProduct> cp(new CrossProduct(move(scanProfessoren),move(scanVorlesungen)));
   	unique_ptr<Selection> select(new Selection(move(cp),persNr,gelesenVon));

 	//and h1.VorlNr=v.VorlNr
	unique_ptr<CrossProduct> cp2(new CrossProduct(move(select),move(scanHoeren1)));
   	unique_ptr<Selection> select2(new Selection(move(cp2),hVorlNr1,vorlNr));

 	//and h2.VorlNr=v.VorlNr
	unique_ptr<CrossProduct> cp3(new CrossProduct(move(select2),move(scanHoeren2)));
   	unique_ptr<Selection> select3(new Selection(move(cp3),hVorlNr2,vorlNr));

   	//and h1.MatrNr != h2.MatrNr
    unique_ptr<Chi> chi(new Chi(move(select3),Chi::NotEqual,hMatrNr1,hMatrNr2));
    const Register* chiResult=chi->getResult();
    unique_ptr<Selection> select4(new Selection(move(chi),chiResult));

	//select p.Name
    unique_ptr<Projection> project(new Projection(move(select4),{name}));


   Printer out(move(project));

   out.open();
   while (out.next());
   out.close();

}
//---------------------------------------------------------------------------
