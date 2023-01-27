#include<stdio.h>
#include<stdlib.h> 
#include<string.h>
#include<time.h>

// #define DEBUG 1

/* SIG Simplified Iterated Greedy graph coloring algorithm C.C. McGeoch 2011 
   getinput.c holds input command parser

   Usage: sig filename 
   filename: input file containing algorithm parameters. Each line 
    starts with a 3-character keyword; lines can appear in any order.
    If line is not present, defaults are used.  

    com    comment lines are ignored 
    inp    filename       // name of input graph file 
    int    i              // initialization policy: (default 1)
                                    2 = greedy input order
                                    1 = each vertex a distinct color
    max    i              // maximum number of iterations: (default 10)
    tar    k              // target coloring, stop if it is reached:  
                             (default 0) 
    cwt    a b c d e f    // integer weights for 6 color
                          // permutation rules (default 10 10 10 10 10 10) 
                              a: 1..k  up
                              b: largest first (by number of vertices in set) 
                              c: smallest first 
                              d: random 
                              e: down k..1 
                              f: reverse current order (new) ?? 
    vwt    w x y z        // integer weights for 4 vertex permutation rules
                          // (default 10 10 10 10 10) what about tiebreakers 
                              w: 1..k 
                              x: k..1 
                              y: random 
                              z: decreasing vertex count in colorset

    rev    r              // revert to previous best if color sum 
                          // does not change after r iterations (default 10) 

    tri    t              // number of trials of the alg 
    see    s              // random number seed 

    
   Outputs: 
     To report performance indicators:       #define REPORTSTATS  1
     To report the coloring and counts:      #define REPORTCOLORS 1
     To see intermediate values              #define DEBUG 1

*/ 
typedef char stringbuf[100]; 
stringbuf filename; 
FILE *fp, *fopen(); 

#define COPTIONS 6
#define VOPTIONS 4

/*--------global parameters --*/
 int initpolicy = 1;     /* 1=all different, 2=random 3 = greedy */
 int maxiter=10;         /* max no iterations */ 
 int target=0;           /* stop early if target coloring is reached */ 
 int cweights[COPTIONS];        /* color reorder policy weights */ 
 int vweights[VOPTIONS];        /* vertex reorder policy weights */ 

 double ccutoffs[COPTIONS];     /* cuttoff for probabilities */ 
 double vcutoffs[VOPTIONS];     /* cutoff for probabilities */ 

 int revert=10;          /* revert if no improvement in color sum after r iters */
 int trials=1;          /* trials of the algorithm */

 int seed ;              /* if seed not set in command file, use time */

/*--------parameter table */  
#define  TSIZE 10         /* number of paramters */      
char* cmdtable[] = { "---",   // 0 sentinel 
		       "com",   // 1
		       "inp",   // 2
		       "int",   // 3
		       "max",   // 4
		       "tar",   // 5 
		       "cwt",   // 6
		       "vwt",   // 7
		       "rev",   // 8
		       "tri",   // 9 
		       "see"  // 10
 };

/*----tabinit-----------------------------*/ 
void tabinit()      /* initalize command table*/ 
{
  int cum;
  int i; 
 
  cum=0;  // defaults 
  for (i=0; i < 6; i++) {
    ccutoffs[i] = cum + (double)1.0/6.0; 
        cum = ccutoffs[i];
  }
  cum=0; 
  for (i = 0; i < 4; i++) {
    vcutoffs[i] = cum + (double)1.0/4.0; 
    cum = vcutoffs[i]; 
  }
  trials = 1;
  srand48 ( (long) time(0)); 

  //  srand48 ( (long) 1234567 );  // default,may be changed 


}
/*----lookup--------------------------------------*/ 
int lookup (char* cmd) /*lookup command in table */ 
{
  int i;
  for (i = TSIZE; i >= 0; i--) {
    if ( !strcmp(cmdtable[i], cmd) ) break; 
  }
  return (i);
}//lookup 

/*----transinput----------------------------------*/ 
void transinput() /*read commands, set parameters */ 
{
  char cmd[4];
  //char buf[100];
  int index;
  int i;
  double tot;
  double cum; 
  stringbuf buf; 

  tabinit(); 

  while (scanf("%3s", cmd) != EOF) { //3-char command 
    fgets(buf, sizeof(buf), stdin);  //rest of the line  

     index = lookup(cmd);

    switch(index) { 
    case -1:
    case 0: // no match 
      printf("%s : Unknown command\n", cmd);
      break; 
    case 1: // comment  ignore 
      break;
    case 2: // input graph file name 
      sscanf(buf, "%99s", filename) ;
      if ((fp = fopen(filename, "r")) == NULL) {
	printf("sig: can't open file %s\n", filename); 
	exit(1);
      }
      break;
    case 3: //initialization policy 1 or 2 
      sscanf(buf, "%d", &initpolicy);
      if ( (initpolicy < 1) || (initpolicy >2)) {
	printf("sig: initpolicy must be 1 (max), 2 (greedy); defaulting to 1\n");
	initpolicy=1;
      }
      break;
    case 4: //maximum iterations
      sscanf(buf, "%d", &maxiter);
      break;
    case 5: //target color count
      sscanf(buf, "%d", &target); 
      break;
    case 6: //color re-order policy 6 weights
       sscanf(buf, "%d %d %d %d %d %d", //ridiculous c 
	     &cweights[0], &cweights[1], &cweights[2], 
	      &cweights[3], &cweights[4], &cweights[5]); 
      tot=0.0; 
      cum=0.0; 
      for (i=0; i < COPTIONS; i++) tot += (double) cweights[i];
      for (i=0; i < COPTIONS; i++) {
	ccutoffs[i] = cum + ((double) cweights[i] / tot);
        cum = ccutoffs[i]; 
      } 

      break;
    case 7: //vertex re-order policy 4 weights
      sscanf(buf, "%d %d %d %d", &vweights[0], &vweights[1], &vweights[2], &vweights[3]) ; 
      tot=0.0;
      cum=0.0;  

      for (i=0; i < VOPTIONS; i++) tot += (double) vweights[i];
      for (i=0; i < VOPTIONS; i++) {
	vcutoffs[i] = cum + ((double) vweights[i] / tot);
        cum = vcutoffs[i]; 
      } 
   
      break;
    case 8: //revert count
      sscanf(buf, "%d", &revert); 
      break;
    case 9: //trials
      sscanf(buf, "%d", &trials);
      break;
    case 10: //rng seed
      sscanf(buf, "%d", &seed);
      srand48( (long) seed); 
      printf("seed is %d\n", seed); 

    }//switch
  }//while scanf

#ifdef DEBUG
  printf("\ninput_file   %s\n", filename);
  printf("init_policy    %d\n", initpolicy);
  printf("max_iterations %d\n", maxiter);
  printf("target_color   %d\n", target);
  printf("color_weights: ");
  for (i=0; i < 6; i++) printf("%d \t", cweights[i]);
  printf("\n");
  printf("color_cutoffs: ");
  for (i=0; i < 6; i++) printf("%lf \t", ccutoffs[i]);
  printf("\n");
  printf("vertex_weights: "); 
  for (i=0; i<4; i++) printf("%d \t", vweights[i]);
  printf("\n");
  printf("vertex_cutoffs: "); 
  for (i=0; i<4; i++) printf("%lf \t", vcutoffs[i]);
  printf("\n");
  printf("revert after %d\n", revert);
#endif 


}//translate input commands



