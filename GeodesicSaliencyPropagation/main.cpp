#include <iostream>
#include "GeodesicSaliencyPropagation.h"


using namespace std;

/**
*	Convert char array to lowercase string
*/
string lowerParam(char* argv){
	for (char* lower = argv; *lower != '\0'; lower++){
		if (*lower >= 'A' && *lower <= 'Z'){
			*lower -= 'A' - 'a';
		}
	}
	return string(argv);
}

int main(int argc, char *argv[])
{
	if (argc < 5){
		cout << "Zadejte prosim nasledujici parametry:" << endl << endl;
		cout << "zdrojovy cilovy K CT [-M pravidelnost] [-GB rozsahX [rozsahY]] [-SP cesta] [-IPL] [-CH cesta] [-SF prah rozsahY posunX strmost]" << endl << endl;
		cout << "  zdrojovy\tCesta ke zdrojovemu obrazku" << endl;
		cout << "  cilovy\tNazev souboru pro vyslednou masku" << endl;
		cout << "  K\tPocet superpixelu, ktere se vygeneruji" << endl;
		cout << "  CT\tPrah Harrisova detektoru rohu, desetinna hodnota v rozsahu 0 - 255" << endl;
		cout << "  -M\tUroven dodrzeni kontur vs. pravidelnost superpixelu" << endl << "\tcele cislo 0 - 40 (vychozi 40)" << endl;
		cout << "  -GB\tPrumer Gaussova vyhlazeni. RozsahX je kladne cele liche cislo." << endl << "\tRozsahY je stejny jako RozsahX kdyz neni zadany. (vychozi 5)" << endl;
		cout << "  -SP\tNazev souboru pro ulozeni vykreslenych superpixelu" << endl;
		cout << "  -IPL\tNeukoncovat program pri zaplneni 40% obrzku klicovymi body" << endl;
		cout << "  -CH\tCesta k obrazku s vykreslenym konvexnim obalem" << endl;
		cout << "  -SF\tParametry step function: rozsahY / (1 + e^( (x - posunX) * -strmost)" << endl << "\tVse jsou realna cisla bez omezeni, vychozi hodnoty:" << endl;
		cout << "\t  prah = 0.005 - odecita se od delky mezi superpixely" << endl;
		cout << "\t  rozsahY = 2" << endl;
		cout << "\t  posunX = 2.8" << endl;
		cout << "\t  strmost = 4" << endl;

		cin.get();
		return 0;
	}
	try{
		Settings settings;
		settings.FileName = argv[1];
		settings.OutpuFileName = argv[2];

		int tmp = stoi(argv[3]);
		if (tmp < 11){
			throw GSPException("Pocet superpixelu musi byt cele cislo, vetsi nez 10");
		}
		settings.Superpixels = tmp;

		double dtmp = stod(argv[4]);
		if (dtmp < 0 || dtmp > 255){
			throw GSPException("Corner detector threshold musi byt v rozsahu 0 - 255");
		}
		settings.CornerDetectorThreshold = dtmp;

		int i = 5;
		try{
			for (; i < argc; i++){

				//Parse all optional parameters
				string param = lowerParam(argv[i]);
				if (param == "-m")
				{
					tmp = stoi(argv[i + 1]);
					if (tmp < 0 || tmp > 40){
						throw GSPException("Parametr M musi byt v rozsahu 0 - 40!");
					}
					settings.M = tmp;
					i++;
				}
				else if (param == "-gb")
				{
					tmp = stoi(argv[i + 1]);
					if (tmp < 1 || tmp % 2 == 0){
						throw GSPException("Parametr GB musi byt kladne cele liche cislo");
					}
					settings.GaussianBlurX = tmp;
					settings.GaussianBlurY = tmp;

					bool setYRange = true;
					try{
						if (i + 2 < argc){
							tmp = stoi(argv[i + 2]);
						}
						else{
							setYRange = false;
						}
					}
					catch (std::exception&){
						setYRange = false;
					}
					if (setYRange){
						if (tmp < 1 || tmp % 2 == 0){
							throw GSPException("Parametr GB rozsahY musi byt kladne cele liche cislo");
						}
						settings.GaussianBlurY = tmp;
						i++;
					}
					i++;
				}
				else if (param == "-sp")
				{
					settings.SuperpixelatedFile = argv[i + 1];
					i++;
				}
				else if (param == "-ipl")
				{
					settings.IgnorePointsLimit = true;
				}
				else if (param == "-ch")
				{
					settings.ConvexHullFile = argv[i + 1];
					i++;
				}
				else if (param == "-sf")
				{
					settings.StepFuncThreshold = stod(argv[i + 1]);
					settings.StepFuncYRange = stod(argv[i + 2]);
					settings.StepFuncXOffset = stod(argv[i + 3]);
					settings.StepFuncSteepness = stod(argv[i + 4]);
					i += 4;
				}
				else{
					string msg = "Neznamy parametr " + string(argv[i]) + "\nPro napovedu spustte bez parametru";
				}
			}
		}
		catch (GSPException& e){
			throw e;
		}
		catch (std::exception& e){
			string msg = "Spatne zadany parametr " + string(argv[i]) + "\nVice informaci: " + e.what();
			throw GSPException(msg.c_str());
		}
		std::unique_ptr<GeodesicSaliencyPropagation> gsp(new GeodesicSaliencyPropagation(settings));
	}
	catch (GSPException& e){
		cout << e.what();
		cin.get();
	}
	catch (cv::Exception& e){
		cout << "OpenCV error: " << endl << e.what();
		cin.get();
	}
	catch (std::exception e){
		cout << "Something really bad happend: " << e.what();
		cin.get();
	}
	return 0;
}
