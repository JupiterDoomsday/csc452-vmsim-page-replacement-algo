//AUTHOR: ISABELA HUTCHINGS
// CLASS: csc 452 summer 2019
// Project 4
// purpose: compiled a c++ script to implement the 4 page replacement algorithms
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <queue>
#include <time.h>
using namespace std;

#define PAGES (2<<19) // number of pages in the page table is (2^32) / (2^13 | 8KB)  
//this is the struct that defines PTE
int d_writes;
class Page_table_entry{
	public:
		bool v=false;
		bool r=true;
		bool d= false;
	int frame_num;	
};
//this is a custom script that splits a string based on a string delim and converts the peices
// int a vector<string>
vector<string> split(const string& text, const string& delims)
{
    vector<string> tokens;
    ssize_t start = text.find_first_not_of(delims), end = 0;

    while((end = text.find_first_of(delims, start)) != -1)
    {
        tokens.push_back(text.substr(start, end - start));
        start = text.find_first_not_of(delims, end);
    }
    if(start != -1)
        tokens.push_back(text.substr(start));

    return tokens;
}
/* this is a helper function that converts the address string into is 
 * respective hex values and take the MSB 19 values and returns it as an int 
 */
 int get_index_helper(string &line){
 	long int ul;
 	int i;
 		vector<string> hex= split(line,",");
 		if(hex.size()>2) {
 			hex.clear();
 			return -1;
 		}
 		char *s= &hex[0][0];
 		sscanf(s,"%lx",&ul);
 		i = (int)(ul >> 13);
 		hex.clear();
 		return i;
 }
/*
 * These function below are used for implementing the optimal page replacement program
 *
 */

 /* setup_list  makes an array for every single addresses or the next 500 addresses in file trace
  * open the file and iterate through each file until you get to count, 
  * THEN you add the values into the p vector and start count. once we reach the endof the file or to 500
  * we exit the program with out now updated p vector<int>
  */
 void setup_list(vector<int> &p, string s, unsigned long start){
 	p.clear();
 	ifstream fp(&s[0]);
 	string line;
 	int count=0;
 	unsigned long i=0;

 	if(fp.is_open()){
 		while(getline(fp,line) ){
			if (count == 500) break;
			if(i == start){
				vector<string> words = split(line," ");
 				if(words[0].size() > 1){
 					words.clear();
 	 				continue;
 				}
 				int i= get_index_helper(words[1]);
 				if(i==-1) 
 					continue;
 				else 
 					p.push_back(i);
 				count++;
			}
			i++;
		}
		fp.close();	
 	}		
 	else{
 		cerr << "ERROR: Invalid file\n";
 		return;
 	}
 }
//this is a debugger funciton that prints out the entire 
 void print_string(vector<int> &fr){
	for (int i=0;i<fr.size();i++){
		cout<< i<<": page "<<fr[i]<<"\n";
	}
}
 /* predict looks t our page_list vector (whcihc contains all of our future history for the next 500 steps)
  *  and the frames vector<int> (which contains all of the pages currently in our frame)
  * we then look through each page# refernce in the frames and check to see which one is refernce the "farthest"
  * we retur the frame index of that page # and set that as our page to replace
  */
int predict(vector<int> &page_list,vector<int> &fr,int index){
	int res=-1,farthest= index;
	for(int i=0;i<fr.size();i++){
		int j;
		for(j=index; j<page_list.size();j++){
			if(fr[i]== page_list[j]){
				if(j>farthest){
					farthest = j;
					res=i;
				}
			break;
		}
	}
	if (j == page_list.size()) 
            return i;
	}
	return (res == -1) ? 0 : res; 
}
 /* Optimal page implement s the entire peices of the optimal program 
  * its check if the framepage has any empty space 
  * or if we need to evict. if woe need to evict we use the predict() to give us an index for frames<int>
  * and swap out the pages
  */
void optimalPage(Page_table_entry *pages, vector<int> &frames, vector<int> &page_list, int pn,int num_of_frames, int index){
	if(frames.size()< num_of_frames){
		frames.push_back(pn);
 		pages[pn].frame_num=(frames.size()-1);
 		cout << "no eviction\n";
	}
	else{
		int j= predict(page_list,frames,index);
		pages[pn].frame_num=j;
		
		if(pages[frames[j]].d){ //if the exempt page is dirty then we write to disk
			d_writes++;
			pages[frames[j]].d=false;
			cout << "evict dirty\n";
		} 
		else
			cout << "evict clean\n";
		
		pages[frames[j]].v=false;
		frames[j]=pn;
	}
	pages[pn].v=true;
}
/*
 * These function below are used for implementing the clock algorithm
 * we keep track of an outside int called point that is set to 2 in main().
 * during a page fault it check every next index in the array w/ pointer, and sees if its sets to false
 * if it is that where we store our page #.
 */
static int clock_replace_and_Update(Page_table_entry *pages, vector<int> &frames, bool sec_chance [], int x, int fn, int pointer){
	//this wile loop iterate though pointer to find an empty
	while(true){
		if(!sec_chance[pointer]){ //if the 2nd chance is a false space
			if(frames.size()< fn){ //check to see if the frames has  empty space, if it is add the page in
						frames.push_back(pointer);
 						pages[pointer].frame_num=(frames.size()-1);
 						cout << "no eviction\n";
				}
			else{ //if it isn't swap them out
				if(pages[frames[pointer]].d){ //if the exempt page is dirty then we write to disk
					d_writes++;
					pages[frames[pointer]].d=false;
					cout << "evict dirty\n";
				} 
				else
					cout << "evict clean\n";
				pages[frames[pointer]].v=false;
				frames[pointer]=x;
				pages[x].frame_num=pointer; //update pointer
			}
			pages[x].v=true;
			return ((pointer +1) % frames.size());
		}
		sec_chance[pointer]=false;

		pointer= ((pointer+1) % frames.size());
	}
}

/* this implements the FIFO algorithm
 * it just takes out frames <int> queue check to see if we have empty space
 * if we have empty space then we push the page # ont to the frame and set it to valid
 * if it isn't we pop out the first element and then push the new page to replace
 *
 */

void fifo_page(Page_table_entry *pages, queue<int> &frames,int pn,int fn){
	if(frames.size()< fn){
		frames.push(pn);
 		pages[pn].frame_num=(frames.size()-1);
 		cout << "no eviction\n";
	}
	else{
		int f= frames.front();
		frames.pop();
		pages[f].v=false;
		if(pages[f].d){ //if the exempt page is dirty then we write to disk
					d_writes++;
					pages[f].d=false;
					cout << "evict dirty\n";
				} 
				else
					cout << "evict clean\n";
		frames.push(pn);
	}
	pages[pn].v=true;
}

/* randpage implement the random algorithm
 * it first checks to see if the frames <int> vector has empty space
 * if it is empty we just insert it into frames.
 * if not we generate a random page number rand_num  using rand() and then swa them out.
 */
void randPage(Page_table_entry *pages, vector<int> &frames,int pn,int fn){
	if(frames.size()< fn){
		frames.push_back(pn);
 		pages[pn].frame_num=(frames.size()-1);
 		cout << "no eviction\n";
	}
	else{
		int rand_num= rand() % fn;
		pages[frames[rand_num]].v=false;
		if(pages[frames[rand_num]].d){ //if the exempt page is dirty then we write to disk
			d_writes++;
			pages[frames[rand_num]].d=false; //set the dirty bit to false since we've saved the page onto the disk
			cout << "evict dirty\n";
		} 
		else
			cout << "evict clean\n";
		pages[pn].frame_num=rand_num;
		frames[rand_num]=pn;
	}
	pages[pn].v=true;

}
//this main function calls the genenral function opens the main memory trace read and calls its repective page replace algorithm
int main(int argc, char* argv[]){
	if (argc<6 ){
		cerr << "ERROR: not enough input"; 
		return -1;
	}
	int num_of_frames= std::atoi(argv[2]);
	string alg= argv[4];
	int i;
	unsigned long count=0;
	int track=0;
	int mem_acc=0;
	int p_fault=0;
	int pointer=0;
	d_writes=0;
	vector<int> frames;
	vector<int> page_list;
	queue <int> q_frames;
	bool sec_chance [num_of_frames];

	if(alg.compare("rand")==0) srand (time(NULL));//setup random seed to make consistent random numbers
	if(alg.compare("opt")==0)
		setup_list(page_list,argv[5],count);
	
	Page_table_entry (*pages)= new Page_table_entry[PAGES];
	ifstream fp(argv[5]);
	string line;
	if(fp.is_open()){ //we beging to read from the memory trace
		while(getline(fp,line)){
			vector<string> words = split(line," ");
 			if(words[0].size() > 1 || words.size() > 2){
 				words.clear();
 				continue;
 				}
 			i = get_index_helper(words[1]);
 			if(i ==-1) continue;
	// this check to see if their been any write on the disk or any page is being modified.
 			if(words[0][0]=='M'){
 				pages[i].d=true;
 				d_writes++;
 			}
 			else if(words[0][0]=='S'){
 				pages[i].d=false;
 				d_writes++;
 			}
 			else if(words[0][0]=='I'){ //if the instruction is instruction fetch
 				d_writes++;
 				mem_acc++;
 			}
 			else{ //if the instruction is just a load increment memory acces
 				mem_acc++;
 			}
	//this check to see if the memory we are trying to access is valid and handling page faults
 			if(pages[i].v){
 				cout << "Hit!\n";
 				if(alg.compare("clock")==0)
 					sec_chance[pages[i].frame_num]=true;	
 			}
 			else{ //if page fault has occured call out the repective functions for each algorithm
 				p_fault++;
 				cout << "Page Fault- ";
 				if(alg.compare("opt")==0){
 					if(track == 251){ //this check to see if we need to our predictions frame_list
 						track=0;
 						setup_list(page_list,argv[5],count);
 					} 
 					optimalPage(pages,frames,page_list,i,num_of_frames,track);
 				} 
 				else if(alg.compare("clock")==0){
 					pointer=clock_replace_and_Update(pages,frames,sec_chance,i,num_of_frames,pointer);
 				}
 				else if(alg.compare("rand")==0){
 					randPage(pages,frames,i,num_of_frames);
 				}
 				else{
 					fifo_page(pages,q_frames,i,num_of_frames);
 				}
 			}
 			track++;
 			count++;
		}
		fp.close();
	}
	else{
		cout << "ERROR: Unable to open file";
		return -1;
	}
	//this deallocate all of the specific c++ containers that might have had memory allocated within them
	delete[] pages;
	frames.clear();
	if(alg.compare("opt")==0) page_list.clear();
	if(alg.compare("rand")==0)
		while(!q_frames.empty()) q_frames.pop();
	//this prints out the general info of the memory trace
	cout<< "Algorithm:  "<<alg<< "\n";
	cout<< "Number of frames:  "<<num_of_frames<<"\n";
	cout<<"Total Memory accesses:  "<< mem_acc<<"\n";
	cout<<"Total page faults:  "<< p_fault<<"\n";
	cout<<"Total writes to disk:  "<< d_writes<<"\n";
	cout<<"Total size of page table:    "<< (PAGES*8) << " bytes\n";
	return 0;
}




 
