#ifndef  _HUFFMAN_H_
#define _HUFFMAN_H_
#endif
#include <iostream>
#include <fstream>
#include <deque>
#include <string>
#include <algorithm>
#include <bitset>
#include <cstring>
#include <map>
using namespace std;
class Node{ //huffman樹節點 
	public:
		int val;
		int freq;
		Node *lchild;
		Node *rchild;
		Node(char _val,int _freq,Node *_lchild,Node *_rchild){
			val=_val;
        	freq=_freq;
        	lchild=_lchild;
        	rchild=_rchild;
		}
};
bool comp(Node*,Node*); //sort() 
bool comp1(Node*,Node*); //stable_sort()
void printCode(Node*,string); //將編碼存入編碼表 
deque<unsigned long> encode(); //將原始檔案編碼壓縮 
void decode(long long,bitset<8>,string&,long long&); //將壓縮檔解碼並還原
char* compress_file(const char *filename,long long &ori,long long &com,float &c_ratio); //壓縮函式
void uncompress_file(const char *compressed_filename,const char *filename); //解壓縮函式
char** strtok_filename(const char *d,const char *str); //將檔名與副檔名做切割