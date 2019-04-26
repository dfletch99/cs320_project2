#ifndef CACHE_H
#define CACHE_H
#include <string>
#include <iostream>
#include <fstream>
#include <limits>

using namespace std;

class Cache{
		char * inFile;
		char * outFile;
	public:
		class Node{
			public:
				Node * left;
				Node * right;
				int hotCold;
				Node(int hc){
					left = nullptr;
					right = nullptr;
					hotCold = hc;
				}
				Node * makeNewNodes(int size){
					if(size == 1) return nullptr;
					Node * root = new Node(0);
					size = size/2;
					root->left = makeNewNodes(size);
					root->right = makeNewNodes(size);
					return root;
				}

				void setNewBits(Node * node, int index, int size){
					if(size == 1) return;
					if(index > (size/2)-1){
						node->hotCold = 1;
						setNewBits(node->right, index/2, size/2);
					}
					else{
						node->hotCold = 0;
						setNewBits(node->left, index/2, size/2);
					}
					return;
				}

				int findIndex(Node * node, int size){
					if(size == 1) return 0;
					size = size/2;
					int ret = 0;
					if(node->hotCold == 1){
						ret += findIndex(node->left, size);
					}
					else{
						ret += size + findIndex(node->right, size);
					}
					return ret;
				}
		};
		Cache(char *, char *);
		void directMapped();
		void setAssociative();
		void fullyAssociative();
		void noAlloc();
		void prefetch();
		void prefetchOnMiss();
		Node * makeNewNodes(int size){
			if(size == 1) return nullptr;
			Node * root = new Node(0);
			size = size/2;
			root->left = makeNewNodes(size);
			root->right = makeNewNodes(size);
			return root;
		}

		void setNewBits(Node * node, int index, int size){
			if(size == 1) return;
			if(index > (size/2)-1){
				node->hotCold = 1;
				setNewBits(node->right, index - (size/2), size/2);
			}
			else{
				node->hotCold = 0;
				setNewBits(node->left, index, size/2);
			}
			return;
		}

		int findIndex(Node * node, int size){
			if(size == 1) return 0;
			size = size/2;
			int ret = 0;
			if(node->hotCold == 1){
				ret += findIndex(node->left, size);
			}
			else{
				ret += size + findIndex(node->right, size);
			}
			return ret;
		}
		
		void deleteNodes(Node * node){
			if(node->left == nullptr && node->right == nullptr) return;
			deleteNodes(node->left);
			deleteNodes(node->right);
			delete node;
		}
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

		unsigned long long cache2Tag = addr >> 13;
		unsigned long long cache4Tag = addr >> 12;
		unsigned long long cache8Tag = addr >> 11;
		unsigned long long cache16Tag = addr >> 10;





		/*2 WAY*/
		//find matching tag
		bool tagFound = false;

		for(int i = 0; i < 2; i++){
			if(cache2Tag == (cache2[cache2Index][i] >> 13) && cache2Counter[cache2Index][i] != -2){
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
			if(cache4Tag == (cache4[cache4Index][i] >> 12) && cache4Counter[cache4Index][i] != -2){
				hc4++;
				cache4Counter[cache4Index][i] = -1;
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
				cache4Counter[cache4Index][lruIndex] = -1;
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
			if(cache8Tag == (cache8[cache8Index][i] >> 11) && cache8Counter[cache8Index][i] != -2){
				hc8++;
				cache8Counter[cache8Index][i] = -1;
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
				cache8Counter[cache8Index][lruIndex] = -1;
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
			if(cache16Tag == (cache16[cache16Index][i] >> 10) && cache16Counter[cache16Index][i] != -2){
				hc16++;
				cache16Counter[cache16Index][i] = -1;
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
				cache16Counter[cache16Index][lruIndex] = -1;
			}
		}
		//update lruCounter
		for(int i = 0; i < 16; i++){
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

void Cache::fullyAssociative(){
	int instructionCounter = 0, hitCounterLRU = 0, hitCounterHC = 0;
	char flag;
	unsigned long long addr;

	unsigned long long cacheLRU[512] = {0};
	unsigned long long cacheHC[512] = {0};

	int cacheCounter[512];
	
	fill_n(cacheCounter, 512, -2);
	Node * root = makeNewNodes(512);

	ifstream infile;
	infile.open(inFile, ios::in);
	ofstream outfile;
	outfile.open(outFile, ios::app);

	while(infile >> flag >> hex >> addr){
		//int offset = 0x1f;
	
		unsigned long long cacheTag = addr >> 5;

		/*~~~~~~LRU replacement~~~~~~*/
		//find tag
		bool tagFound = false;
		for(int i = 0; i < 512; i++){
			if((cacheLRU[i]) >> 5 == cacheTag && cacheCounter[i] != -2){
				hitCounterLRU++;
				cacheCounter[i] = -1;
				tagFound = true;
				break;
			}
		}
		//if tag not found

		if(!tagFound){
			//look for unused way;
			bool foundEmpty = false;
			int highestNumber = -1;
			int lruIndex = -1;
			for(int i = 0; i < 512; i++){
				if(cacheCounter[i] == -2){
					cacheLRU[i] = addr;
					cacheCounter[i] = -1;
					foundEmpty = true;
					break;
				}
				else if(cacheCounter[i] > highestNumber){
					highestNumber = cacheCounter[i];
					lruIndex = i;
				}
			}
			if(!foundEmpty){
				cacheLRU[lruIndex] = addr;
				cacheCounter[lruIndex] = -1;
			}
		}
		//update lruCounter
		for(int i = 0; i < 512; i++){
			if(cacheCounter[i] != -2)
				cacheCounter[i]++;
		}
		/*~~~~~~LRU replacement~~~~~~*/

		/*~~~~~~HOT-COLD replacement~~~~~~*/
		//find tag
		tagFound = false;
		for(int i = 0; i < 512; i++){
			if((cacheHC[i]) >> 5 == cacheTag){
				hitCounterHC++;
				setNewBits(root, i, 512);
				tagFound = true;
				break;
			}
		}
		//if tag not found
		if(!tagFound){
			int index = findIndex(root, 512);
			//cout << index << endl;
			cacheHC[index] = addr;
			setNewBits(root, index, 512);
		}
		/*~~~~~~HOT-COLD replacement~~~~~~*/
		instructionCounter++;
	}
	deleteNodes(root);
	outfile << hitCounterLRU << "," << instructionCounter << ";" << endl;	
	outfile << hitCounterHC << "," << instructionCounter << ";" << endl;
	infile.close();
	outfile.close();

	return;
}

void Cache::noAlloc(){
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

		unsigned long long cache2Tag = addr >> 13;
		unsigned long long cache4Tag = addr >> 12;
		unsigned long long cache8Tag = addr >> 11;
		unsigned long long cache16Tag = addr >> 10;

		bool isStore = (flag == 'S');



		/*2 WAY*/
		//find matching tag
		bool tagFound = false;

		for(int i = 0; i < 2; i++){
			if(cache2Tag == (cache2[cache2Index][i] >> 13) && cache2Counter[cache2Index][i] != -2){
				hc2++;
				cache2Counter[cache2Index][i] = -1;
				tagFound = true;
				break;
			}
		}
		//if miss, populate cache appropriately
		if(!tagFound && !isStore){
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
			if(cache4Tag == (cache4[cache4Index][i] >> 12) && cache4Counter[cache4Index][i] != -2){
				hc4++;
				cache4Counter[cache4Index][i] = -1;
				tagFound = true;
				break;
			}
		}
		//if miss, populate cache appropriately
		if(!tagFound && !isStore){
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
				cache4Counter[cache4Index][lruIndex] = -1;
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
			if(cache8Tag == (cache8[cache8Index][i] >> 11) && cache8Counter[cache8Index][i] != -2){
				hc8++;
				cache8Counter[cache8Index][i] = -1;
				tagFound = true;
				break;
			}
		}
		//if miss, populate cache appropriately
		if(!tagFound && !isStore){
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
				cache8Counter[cache8Index][lruIndex] = -1;
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
			if(cache16Tag == (cache16[cache16Index][i] >> 10) && cache16Counter[cache16Index][i] != -2){
				hc16++;
				cache16Counter[cache16Index][i] = -1;
				tagFound = true;
				break;
			}
		}
		//if miss, populate cache appropriately
		if(!tagFound && !isStore){
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
				cache16Counter[cache16Index][lruIndex] = -1;
			}
		}
		//update lruCounter
		for(int i = 0; i < 16; i++){
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

void Cache::prefetch(){
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

		unsigned long long cache2Tag = addr >> 13;
		unsigned long long cache4Tag = addr >> 12;
		unsigned long long cache8Tag = addr >> 11;
		unsigned long long cache16Tag = addr >> 10;

		unsigned long long addr2 = addr + 0x20;

		unsigned long long cache2Index2 = (addr2 & 0x1fe0) >> 5;
		unsigned long long cache4Index2 = (addr2 & 0xfe0) >> 5;
		unsigned long long cache8Index2 = (addr2 & 0x7e0) >> 5;
		unsigned long long cache16Index2 = (addr2 & 0x3e0) >> 5;

		unsigned long long cache2Tag2 = addr2 >> 13;
		unsigned long long cache4Tag2 = addr2 >> 12;
		unsigned long long cache8Tag2 = addr2 >> 11;
		unsigned long long cache16Tag2 = addr2 >> 10;



		/*2 WAY*/
		//find matching tag
		bool tagFound = false;
		bool tagFound2 = false;

		for(int i = 0; i < 2; i++){
			if(cache2Tag == (cache2[cache2Index][i] >> 13) && cache2Counter[cache2Index][i] != -2){
				hc2++;
				cache2Counter[cache2Index][i] = -1;
				tagFound = true;
				break;
			}
		}

		for(int i = 0; i < 2; i++){
			if(cache2Tag2 == (cache2[cache2Index2][i] >> 13) && cache2Counter[cache2Index2][i] != -2){
				cache2Counter[cache2Index2][i] = -1;
				tagFound2 = true;
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

		if(!tagFound2){
			int highestNumber = -1;
			int lruIndex = -1;
			bool foundEmpty = false;
			for(int i = 0; i < 2; i++){
				if(cache2Counter[cache2Index2][i] == -2){ //not used yet
					cache2[cache2Index2][i] = addr2;
					cache2Counter[cache2Index2][i] = -1;
					foundEmpty = true;
					break;
				}
				else if(cache2Counter[cache2Index2][i] > highestNumber){
					lruIndex = i;
					highestNumber = cache2Counter[cache2Index2][i];
				}
			}
			if(!foundEmpty){
				cache2[cache2Index2][lruIndex] = addr2;
				cache2Counter[cache2Index2][lruIndex] = -1;
			}
		}
		//update lruCounter
		for(int i = 0; i < 2; i++){
			if(cache2Counter[cache2Index][i] != -2){
				cache2Counter[cache2Index][i]++;
			}
			if(cache2Counter[cache2Index2][i] != -2){
				cache2Counter[cache2Index2][i]++;
			}
		}

		/*2 WAY*/

		/*4 WAY*/
		//find matching tag
		tagFound = false;
		tagFound2 = false;

		for(int i = 0; i < 4; i++){
			if(cache4Tag == (cache4[cache4Index][i] >> 12) && cache4Counter[cache4Index][i] != -2){
				hc4++;
				cache4Counter[cache4Index][i] = -1;
				tagFound = true;
				break;
			}
		}

		for(int i = 0; i < 4; i++){
			if(cache4Tag2 == (cache4[cache4Index2][i] >> 12) && cache4Counter[cache4Index2][i] != -2){
				cache4Counter[cache4Index2][i] = -1;
				tagFound2 = true;
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
				cache4Counter[cache4Index][lruIndex] = -1;
			}
		}

		if(!tagFound2){
			int highestNumber = -1;
			int lruIndex = -1;
			bool foundEmpty = false;
			for(int i = 0; i < 4; i++){
				if(cache4Counter[cache4Index2][i] == -2){ //not used yet
					cache4[cache4Index2][i] = addr2;
					cache4Counter[cache4Index2][i] = -1;
					foundEmpty = true;
					break;
				}
				else if(cache4Counter[cache4Index2][i] > highestNumber){
					lruIndex = i;
					highestNumber = cache4Counter[cache4Index2][i];
				}
			}
			if(!foundEmpty){
				cache4[cache4Index2][lruIndex] = addr2;
				cache4Counter[cache4Index2][lruIndex] = -1;
			}
		}
		//update lruCounter
		for(int i = 0; i < 4; i++){
			if(cache4Counter[cache4Index][i] != -2){
				cache4Counter[cache4Index][i]++;
			}
			if(cache4Counter[cache4Index2][i] != -2){
				cache4Counter[cache4Index2][i]++;
			}
		}
		/*4 WAY*/

		/*8 WAY*/
		tagFound = false;
		tagFound2 = false;

		for(int i = 0; i < 8; i++){
			if(cache8Tag == (cache8[cache8Index][i] >> 11) && cache8Counter[cache8Index][i] != -2){
				hc8++;
				cache8Counter[cache8Index][i] = -1;
				tagFound = true;
				break;
			}
		}

		for(int i = 0; i < 8; i++){
			if(cache8Tag2 == (cache8[cache8Index2][i] >> 11) && cache8Counter[cache8Index2][i] != -2){
				cache8Counter[cache8Index2][i] = -1;
				tagFound2 = true;
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
				cache8Counter[cache8Index][lruIndex] = -1;
			}
		}

		if(!tagFound2){
			int highestNumber = -1;
			int lruIndex = -1;
			bool foundEmpty = false;
			for(int i = 0; i < 8; i++){
				if(cache8Counter[cache8Index2][i] == -2){ //not used yet
					cache8[cache8Index2][i] = addr2;
					cache8Counter[cache8Index2][i] = -1;
					foundEmpty = true;
					break;
				}
				else if(cache8Counter[cache8Index2][i] > highestNumber){
					lruIndex = i;
					highestNumber = cache8Counter[cache8Index2][i];
				}
			}
			if(!foundEmpty){
				cache8[cache8Index2][lruIndex] = addr2;
				cache8Counter[cache8Index2][lruIndex] = -1;
			}
		}
		//update lruCounter
		for(int i = 0; i < 8; i++){
			if(cache8Counter[cache8Index][i] != -2){
				cache8Counter[cache8Index][i]++;
			}
			if(cache8Counter[cache8Index2][i] != -2){
				cache8Counter[cache8Index2][i]++;
			}
		}
		/*8 WAY*/

		/*16 WAY*/
		tagFound = false;
		tagFound2 = false;

		for(int i = 0; i < 16; i++){
			if(cache16Tag == (cache16[cache16Index][i] >> 10) && cache16Counter[cache16Index][i] != -2){
				hc16++;
				cache16Counter[cache16Index][i] = -1;
				tagFound = true;
				break;
			}
		}

		for(int i = 0; i < 16; i++){
			if(cache16Tag2 == (cache16[cache16Index2][i] >> 10) && cache16Counter[cache16Index2][i] != -2){
				cache16Counter[cache16Index2][i] = -1;
				tagFound2 = true;
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
				cache16Counter[cache16Index][lruIndex] = -1;
			}
		}

		if(!tagFound2){
			int highestNumber = -1;
			int lruIndex = -1;
			bool foundEmpty = false;
			for(int i = 0; i < 16; i++){
				if(cache16Counter[cache16Index2][i] == -2){ //not used yet
					cache16[cache16Index2][i] = addr2;
					cache16Counter[cache16Index2][i] = -1;
					foundEmpty = true;
					break;
				}
				else if(cache16Counter[cache16Index2][i] > highestNumber){
					lruIndex = i;
					highestNumber = cache16Counter[cache16Index2][i];
				}
			}
			if(!foundEmpty){
				cache16[cache16Index2][lruIndex] = addr2;
				cache16Counter[cache16Index2][lruIndex] = -1;
			}
		}
		//update lruCounter
		for(int i = 0; i < 16; i++){
			if(cache16Counter[cache16Index][i] != -2){
				cache16Counter[cache16Index][i]++;
			}
			if(cache16Counter[cache16Index2][i] != -2){
				cache16Counter[cache16Index2][i]++;
			}
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

void Cache::prefetchOnMiss(){
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

		unsigned long long cache2Tag = addr >> 13;
		unsigned long long cache4Tag = addr >> 12;
		unsigned long long cache8Tag = addr >> 11;
		unsigned long long cache16Tag = addr >> 10;

		unsigned long long addr2 = addr + 0x20;

		unsigned long long cache2Index2 = (addr2 & 0x1fe0) >> 5;
		unsigned long long cache4Index2 = (addr2 & 0xfe0) >> 5;
		unsigned long long cache8Index2 = (addr2 & 0x7e0) >> 5;
		unsigned long long cache16Index2 = (addr2 & 0x3e0) >> 5;

		unsigned long long cache2Tag2 = addr2 >> 13;
		unsigned long long cache4Tag2 = addr2 >> 12;
		unsigned long long cache8Tag2 = addr2 >> 11;
		unsigned long long cache16Tag2 = addr2 >> 10;

		/*2 WAY*/
		//find matching tag
		bool tagFound = false;
		bool tagFound2 = false;

		for(int i = 0; i < 2; i++){
			if(cache2Tag == (cache2[cache2Index][i] >> 13) && cache2Counter[cache2Index][i] != -2){
				hc2++;
				cache2Counter[cache2Index][i] = -1;
				tagFound = true;
				break;
			}
		}

		for(int i = 0; i < 2; i++){
			if(cache2Tag2 == (cache2[cache2Index2][i] >> 13) && cache2Counter[cache2Index2][i] != -2){
				if(!tagFound){
					cache2Counter[cache2Index2][i] = -1;
				}
				tagFound2 = true;
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

		if(!tagFound && !tagFound2){
			int highestNumber = -1;
			int lruIndex = -1;
			bool foundEmpty = false;
			for(int i = 0; i < 2; i++){
				if(cache2Counter[cache2Index2][i] == -2){ //not used yet
					cache2[cache2Index2][i] = addr2;
					cache2Counter[cache2Index2][i] = -1;
					foundEmpty = true;
					break;
				}
				else if(cache2Counter[cache2Index2][i] > highestNumber){
					lruIndex = i;
					highestNumber = cache2Counter[cache2Index2][i];
				}
			}
			if(!foundEmpty){
				cache2[cache2Index2][lruIndex] = addr2;
				cache2Counter[cache2Index2][lruIndex] = -1;
			}
		}
		//update lruCounter
		for(int i = 0; i < 2; i++){
			if(cache2Counter[cache2Index][i] != -2){
				cache2Counter[cache2Index][i]++;
			}
			if(cache2Counter[cache2Index2][i] != -2 && !tagFound){
				cache2Counter[cache2Index2][i]++;
			}
		}

		/*2 WAY*/

		/*4 WAY*/
		//find matching tag
		tagFound = false;
		tagFound2 = false;

		for(int i = 0; i < 4; i++){
			if(cache4Tag == (cache4[cache4Index][i] >> 12) && cache4Counter[cache4Index][i] != -2){
				hc4++;
				cache4Counter[cache4Index][i] = -1;
				tagFound = true;
				break;
			}
		}

		for(int i = 0; i < 4; i++){
			if(cache4Tag2 == (cache4[cache4Index2][i] >> 12) && cache4Counter[cache4Index2][i] != -2){
				if(!tagFound){
					cache4Counter[cache4Index2][i] = -1;
				}
				tagFound2 = true;
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
				cache4Counter[cache4Index][lruIndex] = -1;
			}
		}

		if(!tagFound && !tagFound2){
			int highestNumber = -1;
			int lruIndex = -1;
			bool foundEmpty = false;
			for(int i = 0; i < 4; i++){
				if(cache4Counter[cache4Index2][i] == -2){ //not used yet
					cache4[cache4Index2][i] = addr2;
					cache4Counter[cache4Index2][i] = -1;
					foundEmpty = true;
					break;
				}
				else if(cache4Counter[cache4Index2][i] > highestNumber){
					lruIndex = i;
					highestNumber = cache4Counter[cache4Index2][i];
				}
			}
			if(!foundEmpty){
				cache4[cache4Index2][lruIndex] = addr2;
				cache4Counter[cache4Index2][lruIndex] = -1;
			}
		}
		//update lruCounter
		for(int i = 0; i < 4; i++){
			if(cache4Counter[cache4Index][i] != -2){
				cache4Counter[cache4Index][i]++;
			}
			if(cache4Counter[cache4Index2][i] != -2 && !tagFound){
				cache4Counter[cache4Index2][i]++;
			}
		}
		/*4 WAY*/

		/*8 WAY*/
		tagFound = false;
		tagFound2 = false;

		for(int i = 0; i < 8; i++){
			if(cache8Tag == (cache8[cache8Index][i] >> 11) && cache8Counter[cache8Index][i] != -2){
				hc8++;
				cache8Counter[cache8Index][i] = -1;
				tagFound = true;
				break;
			}
		}

		for(int i = 0; i < 8; i++){
			if(cache8Tag2 == (cache8[cache8Index2][i] >> 11) && cache8Counter[cache8Index2][i] != -2){
				if(!tagFound){
					cache8Counter[cache8Index2][i] = -1;
				}
				tagFound2 = true;
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
				cache8Counter[cache8Index][lruIndex] = -1;
			}
		}

		if(!tagFound && !tagFound2){
			int highestNumber = -1;
			int lruIndex = -1;
			bool foundEmpty = false;
			for(int i = 0; i < 8; i++){
				if(cache8Counter[cache8Index2][i] == -2){ //not used yet
					cache8[cache8Index2][i] = addr2;
					cache8Counter[cache8Index2][i] = -1;
					foundEmpty = true;
					break;
				}
				else if(cache8Counter[cache8Index2][i] > highestNumber){
					lruIndex = i;
					highestNumber = cache8Counter[cache8Index2][i];
				}
			}
			if(!foundEmpty){
				cache8[cache8Index2][lruIndex] = addr2;
				cache8Counter[cache8Index2][lruIndex] = -1;
			}
		}
		//update lruCounter
		for(int i = 0; i < 8; i++){
			if(cache8Counter[cache8Index][i] != -2){
				cache8Counter[cache8Index][i]++;
			}
			if(cache8Counter[cache8Index2][i] != -2 && !tagFound){
				cache8Counter[cache8Index2][i]++;
			}
		}
		/*8 WAY*/

		/*16 WAY*/
		tagFound = false;
		tagFound2 = false;

		for(int i = 0; i < 16; i++){
			if(cache16Tag == (cache16[cache16Index][i] >> 10) && cache16Counter[cache16Index][i] != -2){
				hc16++;
				cache16Counter[cache16Index][i] = -1;
				tagFound = true;
				break;
			}
		}

		for(int i = 0; i < 16; i++){
			if(cache16Tag2 == (cache16[cache16Index2][i] >> 10) && cache16Counter[cache16Index2][i] != -2){
				if(!tagFound){
					cache16Counter[cache16Index2][i] = -1;
				}
				tagFound2 = true;
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
				cache16Counter[cache16Index][lruIndex] = -1;
			}
		}

		if(!tagFound && !tagFound2){
			int highestNumber = -1;
			int lruIndex = -1;
			bool foundEmpty = false;
			for(int i = 0; i < 16; i++){
				if(cache16Counter[cache16Index2][i] == -2){ //not used yet
					cache16[cache16Index2][i] = addr2;
					cache16Counter[cache16Index2][i] = -1;
					foundEmpty = true;
					break;
				}
				else if(cache16Counter[cache16Index2][i] > highestNumber){
					lruIndex = i;
					highestNumber = cache16Counter[cache16Index2][i];
				}
			}
			if(!foundEmpty){
				cache16[cache16Index2][lruIndex] = addr2;
				cache16Counter[cache16Index2][lruIndex] = -1;
			}
		}
		//update lruCounter
		for(int i = 0; i < 16; i++){
			if(cache16Counter[cache16Index][i] != -2){
				cache16Counter[cache16Index][i]++;
			}
			if(cache16Counter[cache16Index2][i] != -2 && !tagFound){
				cache16Counter[cache16Index2][i]++;
			}
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

#endif
