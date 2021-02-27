#include "huffman.h"
deque<Node*> forest; //儲存各個節點的指標
map<unsigned char,string> mtable; //儲存編碼表
map<string,unsigned char> mtable1; //儲存編碼表
deque<unsigned char> file_c; //存放原始檔案資訊
bool comp(Node* a,Node* b){
    return a->val < b->val;
}
bool comp1(Node *a,Node *b){
	return a->freq < b->freq;
}
void printCode(Node *ptr,string s){
	if(ptr->lchild==NULL || ptr->rchild==NULL){
        	unsigned char c=ptr->val;
        	mtable.insert(pair<unsigned char,string>(c,s));
			delete ptr;
        	return;
    	}
    	if(ptr->lchild) printCode(ptr->lchild,s+"0");
    	if(ptr->rchild) printCode(ptr->rchild,s+"1");
}
deque<unsigned long> encode(){
	bitset<8> byte;
	deque<unsigned long> db;
	int index=7;
	map<unsigned char,string>::iterator position;
	for(long long j=0;j<file_c.size();j++){
		position=mtable.find(file_c[j]);
		for(int k=0;k<(position->second).size();k++){
				byte[index]=(position->second)[k]-'0';
				index--;
				if(index<0){
					db.push_back(byte.to_ulong());
					index=7;
				}
		}
	}
	if(index!=7){
		while(index>-1){
			byte[index]=0;
			index--;
		}
		db.push_back(byte.to_ulong());
	}
	return db;
}
void decode(long long ori,bitset<8> b,string& s,long long& c){
	string sb=b.to_string();
	map<string,unsigned char>::iterator position;
	for(int i=0;i<8;i++){
		s+=sb[i];
		if(c<ori){
			position=mtable1.find(s);
			if(position!=mtable1.end()){
				file_c.push_back(position->second);
				c++;
				s="";
			}
		}else break;
	}
}
char** strtok_filename(const char *d,const char *str){
	int num1=1;
	const char *delim=d;
	char copy[256];
	strcpy(copy,str);
	char *p=strtok(copy,delim);
	while(p!=NULL){
		num1++;
		p=strtok(NULL,delim);
	}
	char **argv=(char**) malloc((num1+1)*sizeof(char*));
	int i;
	for(i=0;i<num1;i++) argv[i]=(char*) malloc(256);
	num1=1;
	strcpy(copy,str);
	p=strtok(copy,delim);
	while(p!=NULL){
		strcpy(argv[num1-1],p);
		p=strtok(NULL,delim);
		num1++;
	}
	argv[num1]=(char*)0;
	return argv;
}
char* compress_file(const char *filename,long long &ori,long long &com,float &c_ratio){
    //加壓縮 
		int ascii[256]={0};
		ifstream fin(filename,ios::binary);
		//讀取檔案長度 
		fin.seekg(0,fin.end);
		long long length=fin.tellg(); 
		fin.seekg(0,fin.beg);
		unsigned char *buffer=new unsigned char[length];
		fin.read((char*)buffer,length);
		for(long long i=0;i<length;i++){
			file_c.push_back(buffer[i]);
			int index=buffer[i];
			ascii[index]++; //計算字元出現頻率 
		}
		delete [] buffer;
		for(int i=0;i<256;i++){
			if(ascii[i]!=0){
				forest.push_back(new Node(i,ascii[i],NULL,NULL));
			}
		}
		fin.close();
		int size=forest.size();
		for(int i=0;i<size-1;i++){ //製作huffman樹 
			sort(forest.begin(),forest.end(),comp);
			stable_sort(forest.begin(),forest.end(),comp1);
			Node *ptr=forest.front(); forest.pop_front();
			Node *ptr1=forest.front(); forest.pop_front();
			if(ptr->val<ptr1->val){
				Node *parent=new Node(ptr->val,ptr->freq+ptr1->freq,ptr,ptr1);
				forest.push_back(parent);
			}
			else{
				Node *parent=new Node(ptr1->val,ptr->freq+ptr1->freq,ptr1,ptr);
				forest.push_back(parent);
			}
		}
		printCode(forest.front(),""); //儲存編碼表
		int code_table=mtable.size(); //編碼表大小 
		long long original_byte=file_c.size(); //原始檔案大小 
		ori=original_byte;
		deque<unsigned long> d=encode(); //將編碼後的位元組存入d 
		int num=0;
		map<unsigned char,string>::iterator itr;
		for(itr=mtable.begin();itr!=mtable.end();itr++) num+=sizeof(itr->first)+(itr->second).size()+sizeof(char)*2; //計算編碼表的位元組數 
		long long compress_byte=d.size()+num+sizeof(int)+sizeof(int)+sizeof(long long)*2; //壓縮檔大小
		com=compress_byte; 
		int ratio=compress_byte/original_byte; //壓縮率
		c_ratio=(compress_byte*1.0)/(original_byte*1.0);
        char sub_file[]=".";
        char **argv=strtok_filename(sub_file,filename);
        char *c_filename=strcat(argv[0],".compress");
		ofstream fout(c_filename,ios::binary);
		fout.write((char*) &original_byte,sizeof(long long)); 
		fout.write((char*) &compress_byte,sizeof(long long));
		fout.write((char*) &ratio,sizeof(int));
		fout.write((char*) &code_table,sizeof(int));
		for(itr=mtable.begin();itr!=mtable.end();itr++){ //寫入編碼表 
			string s1='='+itr->second+'\n';
			unsigned char *buffer=new unsigned char[s1.length()+1];
			for(int j=0;j<s1.length()+1;j++){
				if(j==0) buffer[j]=itr->first;
				else buffer[j]=s1[j-1];
			}
			fout.write((char*)buffer,s1.length()+1);
			delete [] buffer;	
		}
		for(long long i=0;i<d.size();i++){ //寫入編碼後的位元組 
			char cb=d[i]; fout.write(&cb,sizeof(char));
		}
		fout.close();
		//printf("%ld\n",d.size()); //印出編碼後的位元組大小 
		
		//釋放map內存
		map<unsigned char,string> empty_map;
		mtable.swap(empty_map);
		mtable.clear();
		//釋放deque內存
		file_c.clear();
		forest.clear();
		d.clear();

        return c_filename; //回傳壓縮檔檔名
}
void uncompress_file(const char *compressed_filename,const char *filename){
    ifstream fin1(compressed_filename,ios::binary);
	fin1.seekg(0,fin1.end);
	long long length=fin1.tellg(); 
	fin1.seekg(0,fin1.beg);
	long long original_byte,compress_byte; int ratio,code_table;
	fin1.read((char*) &original_byte,sizeof(long long));
	//cout<<original_byte<<endl; //原始檔位元組數 
	fin1.read((char*) &compress_byte,sizeof(long long));
	//cout<<compress_byte<<endl; //壓縮檔位元組數 
	fin1.read((char*) &ratio,sizeof(int));
	//cout<<ratio<<endl; //壓縮率
	fin1.read((char*) &code_table,sizeof(int));
	length=length-24;
	unsigned char *buffer=new unsigned char [length];
	int i=0;
	fin1.read((char*)buffer,length);
	deque<unsigned char> aline;
	unsigned char c;
	string str("");
	while(i<length){ //輸出編碼表
		if(buffer[i]=='=' || buffer[i]=='\n' || buffer[i]=='0' || buffer[i]=='1'){} //cout<<(char)buffer[i];
		else if(0 <= int(buffer[i]) && 127 >= int(buffer[i])){} //cout<<(char)buffer[i];
		else {} //cout<<+buffer[i];
		aline.push_back(buffer[i]);
		if(buffer[i]=='\n'){
			c=aline[0];
			if(aline.size()==1){
				i++;
				continue;
			}
			for(int i1=2;i1<aline.size()-1;i1++){
				str+=aline[i1];
			}
			mtable1.insert(pair<string,unsigned char>(str,c));
			str="";
			aline.clear();
		}
		i++;
		if(mtable1.size()==code_table) break;
	}
	string s("");
	long long count=0;
	while(i<length){ //解碼 
		bitset<8> b(buffer[i]);
		decode(original_byte,b,s,count);
		i++;
	}
	fin1.close();
	//cout<<file_c.size()<<endl;
	delete [] buffer;
	//輸出編碼表檔案
	char sub_file[]=".";
	char **arg=strtok_filename(sub_file,filename);
	char *subfile=strcat(arg[0],"_code.txt");
	ofstream fout(subfile,ios::binary);
	map<string,unsigned char>::iterator itr;
	for(itr=mtable1.begin();itr!=mtable1.end();itr++){
		string s1='='+itr->first+'\n';
		unsigned char *buffer=new unsigned char[s1.length()+1];
		for(int j=0;j<s1.length()+1;j++){
			if(j==0) buffer[j]=itr->second;
			else buffer[j]=s1[j-1];
		}
		fout.write((char*)buffer,s1.length()+1);
		delete [] buffer;	
	}
	fout.close();
	//輸出解壓縮後的檔案 
	ofstream fout1(filename,ios::binary);
	unsigned char *file=new unsigned char[file_c.size()];
	for(long long i=0;i<file_c.size();i++) file[i]=file_c[i]; 
	fout1.write((char*)file,sizeof(char)*file_c.size());
	delete [] file; 
	fout1.close();
		
	//釋放map內存
	map<string,unsigned char> empty_map;
	mtable1.swap(empty_map);
	mtable1.clear(); 
	//釋放deque內存
	file_c.clear();
	aline.clear();
}