#include <iostream>
#include <fstream>

using namespace std;

typedef struct HorizonFile
{	
	string name[17037];
	string epoch_yr[17037];
	double a[17037];
	double e[17037];
	double i[17037];
	double om[17037];
	double w[17037];
	double f[17037];
} HorizonFile;


class AstroFileReader
{
	

public:
	
	void ReadHorizonQuery( HorizonFile * );

};