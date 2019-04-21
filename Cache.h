#ifndef CACHE_H
#define CACHE_H
#include <string>
#include <iostream>
#include <fstream>

using namespace std;

class Cache{
		char * inFile;
		char * outFile;
	public:
		Cache(char *, char *);
		void directMapped();
		void setAssociative();
		void fullyAssociative();
		void noAlloc();
		void prefetch();
		void prefetchOnMiss();
};

Cache::Cache(char * inFile, char * outFile){
	this->inFile = inFile;
	this->outFile = outFile;
}

void Cache::directMapped(){
	int instructionCounter = 0, oneKBHitCounter = 0, fourKBHitCounter = 0, sixteenKBHitCounter = 0, thirtytwoKBHitCounter = 0;
	char flag;
	unsigned long long addr;
	
	unsigned long long oneKB[1024];
	unsigned long long fourKB[4096];
	unsigned long long sixteenKB[16384];
	unsigned long long thirtytwoKB[32768];

	ifstream infile;
	infile.open(inFile, ios::in);
	ofstream outfile;
	outfile.open(outFile, ios::app);

	while(infile >> flag >> hex >> addr){
		//int offset = addr & 0xf;

		unsigned long long oneKBIndex = addr & 0x3ff0 >> 4;
		unsigned long long fourKBIndex = addr & 0xfff0 >> 4;
		unsigned long long sixteenKBIndex = addr & 0x3fff0 >> 4;
		unsigned long long thirtytwoKBIndex = addr & 0x7fff0 >> 4;

		unsigned long long oneKBTag = addr & 0xffffc000 >> 14;
		unsigned long long fourKBTag = addr & 0xffff0000 >> 16;
		unsigned long long sixteenKBTag = addr & 0xfffc0000 >> 18;
		unsigned long long thirtytwoKBTag = addr & 0xfff80000 >> 19;

		if((oneKB[oneKBIndex] & 0xffffc000 >> 14) == oneKBTag){
			oneKBHitCounter++;
		}
		else{
			oneKB[oneKBIndex] = addr;
		}
		if((fourKB[fourKBIndex] & 0xffff0000 >> 16) == fourKBTag){
			fourKBHitCounter++;
		}
		else{
			fourKB[fourKBIndex] = addr;
		}
		if((sixteenKB[sixteenKBIndex] & 0xfffc0000 >> 18) == sixteenKBTag){
			sixteenKBHitCounter++;
		}
		else{
			sixteenKB[sixteenKBIndex] = addr;
		}
		if((thirtytwoKB[thirtytwoKBIndex] & 0xfff80000 >> 19) == thirtytwoKBTag){
			thirtytwoKBHitCounter++;
		}
		else{
			thirtytwoKB[thirtytwoKBIndex] = addr;
		}
		instructionCounter++;
	}
	outfile << oneKBHitCounter << "," << instructionCounter << "; ";
	outfile << fourKBHitCounter << "," << instructionCounter << "; ";
	outfile << sixteenKBHitCounter << "," << instructionCounter << "; ";
	outfile << thirtytwoKBHitCounter << "," << instructionCounter << ";" << endl;
	infile.close();
	outfile.close();
	return;
}

void Cache::setAssociative(){}

void Cache::fullyAssociative(){}

void Cache::noAlloc(){}

void Cache::prefetch(){}

void Cache::prefetchOnMiss(){}

#endif
