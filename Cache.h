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
	
	unsigned long long oneKB[32];
	unsigned long long fourKB[128];
	unsigned long long sixteenKB[512];
	unsigned long long thirtytwoKB[1024];

	ifstream infile;
	infile.open(inFile, ios::in);
	ofstream outfile;
	outfile.open(outFile, ios::app);

	while(infile >> flag >> hex >> addr){
		//int offset = addr & 0x1f;

		unsigned long long oneKBIndex = (addr & 0x3e0) >> 5;
		unsigned long long fourKBIndex = (addr & 0xfe0) >> 5;
		unsigned long long sixteenKBIndex = (addr & 0x3fe0) >> 5;
		unsigned long long thirtytwoKBIndex = (addr & 0x7fe0) >> 5;

		unsigned long long oneKBTag = (addr & 0xfffffc00) >> 10;
		unsigned long long fourKBTag = (addr & 0xfffff000) >> 12;
		unsigned long long sixteenKBTag = (addr & 0xffffc000) >> 14;
		unsigned long long thirtytwoKBTag = (addr & 0xffff8000) >> 15;
		
		if(((oneKB[oneKBIndex] & 0xfffffc00) >> 10) == oneKBTag){
			oneKBHitCounter++;
		}
		else{
			oneKB[oneKBIndex] = addr;
		}
		if(((fourKB[fourKBIndex] & 0xfffff000) >> 12) == fourKBTag){
			fourKBHitCounter++;
		}
		else{
			fourKB[fourKBIndex] = addr;
		}
		if(((sixteenKB[sixteenKBIndex] & 0xffffc000) >> 14) == sixteenKBTag){
			sixteenKBHitCounter++;
		}
		else{
			sixteenKB[sixteenKBIndex] = addr;
		}
		if(((thirtytwoKB[thirtytwoKBIndex] & 0xffff8000) >> 15) == thirtytwoKBTag){
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

void Cache::setAssociative(){
	int instructionCounter = 0, hc2 = 0, hc4 = 0, hc8 = 0, hc16 = 0;
	char flag;
	unsigned long long addr;
	
	unsigned long long cache2[256][2];
	unsigned long long cache4[128][4];
	unsigned long long cache8[64][8];
	unsigned long long cache16[32][16];

	int cache2Counter[256][2];
	int cache4Counter[128][4];
	int cache8Counter[64][8];
	int cache16Counter[32][16];

	for(int i = 0; i < 256; i++){
		for(int j = 0; j < 2; j++){
			cache2Counter[i][j] = -2;
		}
	}
	for(int i = 0; i < 128; i++){
		for(int j = 0; j < 4; j++){
			cache4Counter[i][j] = -2;
		}
	}
	for(int i = 0; i < 64; i++){
		for(int j = 0; j < 8; j++){
			cache8Counter[i][j] = -2;
		}
	}
	for(int i = 0; i < 32; i++){
		for(int j = 0; j < 16; j++){
			cache16Counter[i][j] = -2;
		}
	}

	ifstream infile;
	infile.open(inFile, ios::in);
	ofstream outfile;
	outfile.open(outFile, ios::app);
	while(infile >> flag >> hex >> addr){
		//int offset = 0x1f;
		
		unsigned long long cache2Index = (addr & 0x1fe0) >> 5;
		unsigned long long cache4Index = (addr & 0xfe0) >> 5;
		unsigned long long cache8Index = (addr & 0x7e0) >> 5;
		unsigned long long cache16Index = (addr & 0x3e0) >> 5;

		unsigned long long cache2Tag = (addr & 0xffffe) >> 13;
		unsigned long long cache4Tag = (addr & 0xfffff) >> 12;
		unsigned long long cache8Tag = (addr & 0xfffff8) >> 11;
		unsigned long long cache16Tag = (addr & 0xfffffc) >> 10;

		/*2 WAY*/
		//find matching tag
		bool tagFound = false;
		for(int i = 0; i < 2; i++){
			if((cache2[cache2Index][i] & 0xffffe) >> 13 == cache2Tag){
				hc2++;
				cache2Counter[cache2Index][i] = -1;
				tagFound = true;
				break;
			}
		}
		//if miss, populate cache appropriately
		if(!tagFound){
			int highestNumber = -1;
			int lruIndex = -1;
			bool foundEmpty = false;
			for(int i = 0; i < 2; i++){
				if(cache2Counter[cache2Index][i] == -2){ //not used yet
					cache2[cache2Index][i] = addr;
					cache2Counter[cache2Index][i] = -1;
					foundEmpty = true;
					break;
				}
				else if(cache2Counter[cache2Index][i] > highestNumber){
					lruIndex = i;
					highestNumber = cache2Counter[cache2Index][i];
				}
			}
			if(!foundEmpty){
				cache2[cache2Index][lruIndex] = addr;
				cache2Counter[cache2Index][lruIndex] = -1;
			}
		}
		//update lruCounter
		for(int i = 0; i < 2; i++){
			if(cache2Counter[cache2Index][i] != -2)
				cache2Counter[cache2Index][i]++;
		}
		/*2 WAY*/

		/*4 WAY*/
		//find matching tag
		tagFound = false;
		for(int i = 0; i < 4; i++){
			if((cache4[cache4Index][i] & 0xfffff) >> 12 == cache4Tag){
				hc4++;
				tagFound = true;
				break;
			}
		}
		//if miss, populate cache appropriately
		if(!tagFound){
			int highestNumber = -1;
			int lruIndex = -1;
			bool foundEmpty = false;
			for(int i = 0; i < 4; i++){
				if(cache4Counter[cache4Index][i] == -2){ //not used yet
					cache4[cache4Index][i] = addr;
					cache4Counter[cache4Index][i] = -1;
					foundEmpty = true;
					break;
				}
				else if(cache4Counter[cache4Index][i] > highestNumber){
					lruIndex = i;
					highestNumber = cache4Counter[cache4Index][i];
				}
			}
			if(!foundEmpty){
				cache4[cache4Index][lruIndex] = addr;
			}
		}
		//update lruCounter
		for(int i = 0; i < 4; i++){
			if(cache4Counter[cache4Index][i] != -2)
				cache4Counter[cache4Index][i]++;
		}
		/*4 WAY*/

		/*8 WAY*/
		//find matching tag
		tagFound = false;
		for(int i = 0; i < 8; i++){
			if((cache8[cache8Index][i] & 0xfffff8) >> 11 == cache8Tag){
				hc8++;
				tagFound = true;
				break;
			}
		}
		//if miss, populate cache appropriately
		if(!tagFound){
			int highestNumber = -1;
			int lruIndex = -1;
			bool foundEmpty = false;
			for(int i = 0; i < 8; i++){
				if(cache8Counter[cache8Index][i] == -2){ //not used yet
					cache8[cache8Index][i] = addr;
					cache8Counter[cache8Index][i] = -1;
					foundEmpty = true;
					break;
				}
				else if(cache8Counter[cache8Index][i] > highestNumber){
					lruIndex = i;
					highestNumber = cache8Counter[cache8Index][i];
				}
			}
			if(!foundEmpty){
				cache8[cache8Index][lruIndex] = addr;
			}
		}
		//update lruCounter
		for(int i = 0; i < 8; i++){
			if(cache8Counter[cache8Index][i] != -2)
				cache8Counter[cache8Index][i]++;
		}
		/*8 WAY*/

		/*16 WAY*/
		//find matching tag
		tagFound = false;
		for(int i = 0; i < 16; i++){
			if((cache16[cache16Index][i] & 0xfffffc) >> 10 == cache16Tag){
				hc16++;
				tagFound = true;
				break;
			}
		}
		//if miss, populate cache appropriately
		if(!tagFound){
			int highestNumber = -1;
			int lruIndex = -1;
			bool foundEmpty = false;
			for(int i = 0; i < 16; i++){
				if(cache16Counter[cache16Index][i] == -2){ //not used yet
					cache16[cache16Index][i] = addr;
					cache16Counter[cache16Index][i] = -1;
					foundEmpty = true;
					break;
				}
				else if(cache16Counter[cache16Index][i] > highestNumber){
					lruIndex = i;
					highestNumber = cache16Counter[cache16Index][i];
				}
			}
			if(!foundEmpty){
				cache16[cache16Index][lruIndex] = addr;
			}
		}
		//update lruCounter
		for(int i = 0; i < 2; i++){
			if(cache16Counter[cache16Index][i] != -2)
				cache16Counter[cache16Index][i]++;
		}
		/*16 WAY*/
		instructionCounter++;
	}
	outfile << hc2 << "," << instructionCounter << "; ";
	outfile << hc4 << "," << instructionCounter << "; ";
	outfile << hc8 << "," << instructionCounter << "; ";
	outfile << hc16 << "," << instructionCounter << ";" << endl;
	infile.close();
	outfile.close();
	return;
}

void Cache::fullyAssociative(){}

void Cache::noAlloc(){}

void Cache::prefetch(){}

void Cache::prefetchOnMiss(){}

#endif
