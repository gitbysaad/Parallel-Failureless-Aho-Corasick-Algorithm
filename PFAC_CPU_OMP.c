#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <omp.h>

//#define OUTPUT_PFAC_TABLE        //creating output table for PFAC

#define MAX_STATE 131000        //define max state
#define CHAR_SET 256            //define character set to 256  singed 256 values 
#define PATTERN_LEN 10000
char *input_string;            //input 
int input_size;
int PFAC_table[MAX_STATE][CHAR_SET];
int output_table[MAX_STATE];
int *match_result;
int pattern_len[11000];
int initial_state;
    
int create_table(char *patternfilename)     //create table function 1. parameter initialization
{
    int i, j;
    int state_num = 1;
    int pattern_num = 0;
    int ch;
    int state = 0;
    FILE *fptr;
    FILE *fpout;
    int pre_state;
    char pre_char;
    int final_state=0;
    int count;
    char string[PATTERN_LEN];
 
    // initialize PFAC table                    
    for (i = 0; i < MAX_STATE; i++) {       //Init with Max State and Max Char Set to table 
        for (j = 0; j < CHAR_SET; j++) {
            PFAC_table[i][j] = -1;
        }
        output_table[i] = 0;
    }
    
    // open input file
    fptr = fopen(patternfilename, "rb");        //Open Pattern File
    if (fptr == NULL) {                         //If can't open file then Error
        perror("Open input file failed.");
        exit(1);
    }
    
   //count pattern number                       //Count No. of Pattern 
   pattern_num=1;
   count=0;
   fgets(string, PATTERN_LEN,fptr);            //copies contents of file *ptr to string 
   while(!feof(fptr)){
   	int len=strlen(string);                    //string lenght is stored to len
		 if ( '\n' == string[len-1] ){        //if detects \n then reduces the len of pattern 
 	          len-- ;
 	   }
	   pattern_len[pattern_num]=len;          //pattern_len[1]=calculated length
     count++;                                 //count is incremented
     pattern_num++;                           //pattern length is incremented
     fgets(string, PATTERN_LEN,fptr);         //continues the process until the end of the file
     
   }
   printf("The number of pattern:%d\n ",count);     //exits while and prints no. of patterns (count)
   rewind(fptr);                                    //set file position to the beginning of the file
      
   //set initial state                                  //stating with initial state
   initial_state=count;                                 //no of pattern = no of initial state
   printf("initial state : %d\n",initial_state);        //prints no of initial state
   
   
  //end of count pattern number                         //resetting no. of pattern 
	pattern_num=0;                                      //assigning pat no. =0
	state=initial_state;                                //no of pattern/initial state=state_num 
  state_num=initial_state+1;                            //+1 the init_state and assign to state_num 3 becomes 4      
  
//initializing no of state counting for pattern file

    while (1) {                        // It is an infinite loop which will run till a break statement is issued explicitly.                                                             
        int ch = fgetc(fptr);           //get next character
        if (ch == EOF) {    // read file end
            break;              //break
        }
        else if (ch == '\n') {    // detects a new line then creates o/p table 
        	PFAC_table[pre_state][pre_char] = final_state;       //table [int state] [char char] type
            pattern_num = pattern_num + 1;                      //inc pattern number by 0+1=1
            output_table[final_state] = pattern_num;            //o/p table [final state table] = 0;
            
            for(i=0;i<256;i++){                                     //for 256 values of char
               PFAC_table[final_state][i]=PFAC_table[state][i];  //current state [] [1st char]= final state [][]
               PFAC_table[state][i]=-1;                          //state table is set to null now.
             }
             
            state_num--;                                        //state no is decremented post
            state = initial_state;                              //state=initial state
            final_state++;                                      //final state inc. 
        }
        else {    // create table                           //if not a new line then 
            if (PFAC_table[state][ch] == -1) {              //if char is null               
                PFAC_table[state][ch] = state_num;          //state no. val of current 2d PFAC table
                pre_state=state;                            //state becomes pre state
                pre_char=ch;                                //char become pre char
                state = state_num;                          
                state_num = state_num + 1;
            }
            else {
            	  pre_state=state;
                pre_char=ch;
                state = PFAC_table[state][ch];

            }
        }
        
        if (state_num > MAX_STATE) {                            //no of state exceeds max state 
            perror("State number overflow\n");                  //print overflow
            exit(1);                                            //abnormal termination
        }
    }
    printf("The number of state is %d\n",state_num);            //final no of state
    
#ifdef OUTPUT_PFAC_TABLE                                //opens file to write into table
    // open output file
    fpout = fopen("PFAC_table.txt", "w");                 //create file
    if (fpout == NULL) {                                //if null
        perror("Open output file failed.\n");
        exit(1);                                        //terminate
    }
    // output PFAC table                                
    for (i = 0; i < state_num; i++) {                         //prints output of PFAC_table                         
        for (j = 0; j < CHAR_SET; j++) {
            fprintf(fpout, "%d ", PFAC_table[i][j]);        //prints table id + 
        }
        fprintf(fpout, "%d\n", output_table[i]);            //prints table o/p
    }
#endif
    
    return state_num;                                         //send back state_num
}


void PFAC_CPU(char* d_input_string, int* d_match_result, int input_size){
		int start;
		int pos;    // position to read input for the thread
    int state;
    int inputChar;
    int match_pattern = 0;	
    struct timeval t_start, t_end;
    float elapsedTime;



    // initialize match result on CPU

    for (pos = 0; pos < input_size; pos++) {
        d_match_result[pos] = 0;
    }

    // start time
    gettimeofday(&t_start, NULL);
    #pragma omp parallel private (start, state, pos, inputChar) shared (d_match_result, input_size, d_input_string, PFAC_table, initial_state) num_threads(8)
    {
    #pragma omp for schedule(dynamic,32768)
    for (start=0; start < input_size; start++) {
        state = initial_state;
        pos = start;

        while ( (state != -1) && (pos < input_size) ) {         //if not -1 and in range
 
            if(state <initial_state){                           
            	 d_match_result[start]=state+1;
            }
            	
            // read input character

            inputChar =(unsigned char)d_input_string[pos];         //string position to i/p char 0-255

            state = PFAC_table[state][inputChar];

			pos = pos + 1;
        }
    }
    }
    
    // stop time
    gettimeofday(&t_end, NULL);
    // compute and print the elapsed time in millisec
    elapsedTime = (t_end.tv_sec - t_start.tv_sec) * 1000.0;
    elapsedTime += (t_end.tv_usec - t_start.tv_usec) / 1000.0;
    printf("The elapsed time is %f ms\n", elapsedTime);
    printf("The input size is %d bytes\n", input_size );
    printf("The throughput is %f Gbps\n",(float)input_size/(elapsedTime*1000000)*8 );
	
}


int main(int argc, char **argv)
{
    int i;
    int state_num;
    int deviceID = 0;
    FILE *fpin;
    FILE *fpout;
    int count=0;
 
    if(argc!=3){                                                            //no of arg should be 3
       printf("using command %s pattern_file input_file\n",argv[0]);
       exit(0);
    }
    state_num = create_table(argv[1]);                  //call create table and store value in state_num

    // read input data
    fpin = fopen(argv[2], "rb");                               //opening file failed
    if (fpin == NULL) {
        perror("Open input file failed.");
        exit(1);
    }
    // obtain file size:
    fseek (fpin , 0 , SEEK_END);                //set position (input file, 0, till the end)
    input_size = ftell (fpin);                  //current file position -> size of the file 
    rewind (fpin);                              //reset the positon to the top of the file
    // allocate memory to contain the whole file:
    input_string = (char *) malloc (sizeof(char)*input_size);          //dynamic memory allocation    
    if (input_string == NULL){                                         //internal error
        perror("Allocate input memory error");                          //system error
        exit (1);
    }//else printf("The input size is %d\n",input_size);
    	
    // copy the file into the buffer:
    input_size = fread (input_string, 1, input_size, fpin);         //copy fpin to i/p string
    fclose(fpin);
    
    // allocate memory for output
    match_result = (int *) malloc (sizeof(int)*input_size);             
    if (match_result == NULL){
        perror("Allocate output memory error");
        exit (1);
    }

    
    //printf("[1]\n");
    PFAC_CPU(input_string, match_result, input_size);
    //printf("[2]\n");

    // Output results
    fpout = fopen("PFAC_CPU_OMP_match_result.txt", "w");
    if (fpout == NULL) {
        perror("Open output file failed.\n");
        exit(1);
    }
    // Output match result to file
    for (i = 0; i < input_size; i++) {
        if (match_result[i] != 0) {
            fprintf(fpout, "At position %4d, match pattern %d\n", i, match_result[i]);
            count++;
        }
    }
    printf("The matched number is %d\n", count);
    fclose(fpout);
    
    free( input_string ) ;
    free( match_result ) ;

    return 0;
}
