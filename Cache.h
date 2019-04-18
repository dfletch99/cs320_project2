#ifndef CACHE_H
#define CACHE_H
#include <string>
#include <iostream>

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
		void prefectchOnMiss();
}

Cache::Cache(char * inFile, char * outFile){
	this->inFile = inFile;
	this->outFile = outFile;
}

void Cache::directMapped(){
	int instructionCounter = 0, hitCounter = 0;
	char flag;
	unsigned long long addr;
	
	int oneKB[1024][8];
	int fourKB[4096][8];
	int sixteenKB[16384][8];
	int thirtytwoKB[32768][8];

	ifstream infile;
	infile.open(inFile, ios::in);
	ofstream outfile;
	outfile.open(outfile, ios::app);

	while(infile >> flag >> hex >> addr){
		int offset = addr & 0x1f;

		int oneKBIndex = addr & 0x7fe >> 5;
		int fourKBIndex = addr & 0x1ffe >> 5;
		int sixteenKBIndex = addr & 0x7ffe >> 5;
		int thirtytwoKBIndex = addr & 0xfffe >> 5;

		int oneKBTag = addr & 0xffff8 >> 15;
		int fourKBTag = addr & 0xfff7 >> 17;
		int sixteenKBTag = addr & 0xfff8 >> 19;
		int thirtytwoKBTag = addr & 0xfff >> 20;

		if(oneKB[index]

		
	}
}
