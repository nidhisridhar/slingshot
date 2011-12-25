#include <iostream>
#include <fstream>
#include <string>
#include "AstroFileReader.h"

using namespace std;

void AstroFileReader::ReadHorizonQuery(HorizonFile *smallbodies)
{
	string line;
	unsigned int lineNumber = 0;
	unsigned int foundNumber = 0;
	bool filestart = false;
	size_t found;

	ifstream myfile ("../data/horizons.txt");
	
	if (myfile.is_open())
	{
		while (! myfile.eof() )
		{
			getline (myfile,line);

			if(lineNumber >= 10)
			{
				found = line.find("[...unnamed...]");
				if(found == string::npos)
				{
					double currElements[9];
					string currNames[2];
					cout << line << endl;
					//smallbodies->elements[lineNumber][0] = currElements[0];
					//smallbodies->names[lineNumber][0] = currNames[0];
					
					foundNumber++;
					printf("%d\n",foundNumber);
				}
			}
		lineNumber++;
		}
	}else cout << "Unable to open file"; 

myfile.close();
}

	

