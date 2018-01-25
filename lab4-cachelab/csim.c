#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

typedef struct 
{
	unsigned tag;
	unsigned usedtime;
} block;
block *cache;


int getOpt(int argc,char **argv,int *s,int *E,int *b,int *verbose,char *tracefile);
void printHelpMenu();
void cacheStateOut(char op,int type);
void find(char op,unsigned addr,unsigned size,int time);


int hit,miss,eviction;
int verbose;
int s,E,b;

int main(int argc,char **argv)
{
	FILE *fp;
	char tracefile[100];
	char op[10];
	unsigned addr,size;
	int t;

	getOpt(argc,argv,&s,&E,&b,&verbose,tracefile); // get option

	if(s<0||E<0||b<1){
		printf("Invalid Cache parameter\n\n");
		exit(1);
	}

	cache = (block *)malloc(sizeof(block)* E<<s); 
	memset(cache,0,sizeof(block)* E<<s);

	fp = fopen (tracefile,"r");
	while(fscanf(fp,"%s%x,%d\n",op,&addr,&size) > 0){
		if(verbose)
			printf("%s %x,%d ",op,addr,size);
		switch(op[0]){
			case 'M': hit++;
			case 'L': 
			case 'S': find(op[0],addr,size,++t); 	
		}
	}
    printSummary(hit, miss, eviction);
    free(cache);
    fclose(fp);
    return 0;
}

void find(char op, unsigned addr,unsigned size,int time){
	int i;
	unsigned tag = addr >>b >>s ;
	unsigned set_index = addr >> b &((1<<s) -1);
	block *cache_set = cache + E * set_index ;  // set address
	block *eviction_block = cache_set;			// LRU cacheline
	for(i = 0;i<E;i++){
		if(cache_set[i].usedtime>0 && cache_set[i].tag ==tag){ //hit
			cache_set[i].usedtime = time;
			hit++;
			if(verbose) cacheStateOut(op,0);
			return;
			
		}
		else if(!cache_set[i].usedtime){ // empty block
			miss++;
			cache_set[i].tag = tag;
			cache_set[i].usedtime = time;
			if(verbose) cacheStateOut(op,1);
			return;
		}
		else if(cache_set[i].usedtime < eviction_block->usedtime) // !=tag , current block is older
			eviction_block = cache_set+i; 						 
	}
	miss ++;
	eviction ++;
	eviction_block->tag = tag; // replace sacrifice cacheline
	eviction_block->usedtime = time;
	if(verbose) cacheStateOut(op,2);
	return ;
}

int getOpt(int argc,char **argv,int *s,int *E,int *b,int *verbose,char *tracefile)
{
	int oc;
	while((oc=getopt(argc,argv,"hvs:E:b:t:"))!=-1){
		switch(oc){
			case 'h': printHelpMenu();break; // print usage
			case 'v': *verbose=1;break; 	
			case 's': *s = atoi(optarg);break;
			case 'E': *E = atoi(optarg);break;
			case 'b': *b = atoi(optarg);break;
			case 't': strcpy(tracefile,optarg);break;
			default : printf("input error\n");break;
		}
	}
	return 0;
}
void cacheStateOut(char op,int type){
	switch(type){
		case 0: //hit
			switch(op){
				case 'L':
				case 'S':printf("hit\n");break;
				case 'M':printf("hit hit\n");break;
			}break;

		case 1: //miss
			switch(op){
				case 'L':
				case 'S':printf("miss\n");break;
				case 'M':printf("miss hit\n");break;
			}
		case 2: //eviction
			switch(op){
				case 'L':
				case 'S':printf("miss eviction\n");break;
				case 'M':printf("miss eviction hit\n");break;
			}break;
	}
}

void printHelpMenu(){
    printf("Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <file>\n");
    printf("Options:\n");
    printf("-h         Print this help message.\n");
    printf("-v         Optional verbose flag.\n");
    printf("-s <num>   Number of set index bits.\n");
    printf("-E <num>   Number of lines per set.\n");
    printf("-b <num>   Number of block offset bits.\n");
    printf("-t <file>  Trace file.\n\n\n");
    printf("Examples:\n");
    printf("linux>  ./csim -s 4 -E 1 -b 4 -t traces/yi.trace\n");
    printf("linux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}

void checkOptarg(char *curOptarg){
    if(curOptarg[0]=='-'){
        printf("./csim :Missing required command line argument\n");
        printHelpMenu();
        exit(0);
    }
}