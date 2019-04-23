#include "Cache.h"


using namespace std;

int main(int argc, char ** argv){
	if(argc != 3){
		cout << "\t~~~~~~~~~~~~~~~Error, must input inFile and outFile~~~~~~~~~~~~~~~" << endl;
		return -1;
	}
	Cache c(argv[1], argv[2]);
	c.directMapped();
	c.setAssociative();


	return 0;
}
