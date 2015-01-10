/* mixer.c */

#include <stdio.h>
#include <stdlib.h>

#define TRACK_MAX 10
#define DATA_MAX 44100*300
#define VALUE_MAX 32765

double sum[DATA_MAX];

int main(int argc, char *argv[]){
  FILE *f[TRACK_MAX];
  int flag[TRACK_MAX], flag2;
  double n[TRACK_MAX];
  int i, max, data;
  long j, t;

  /* file load */
  if(argc < 2){
    printf("Usage: a.out file1 file2 ...\n");
    exit(1);
  }
  else if(argc > TRACK_MAX+1){
    printf("too much track.\n");
    exit(1);
  }

  for(i=0; i<argc-1; i++){
    f[i] = fopen(argv[i+1], "r");
    if(f[i] == NULL){
      printf("cannot open %s.\n",argv[i+1]);
      exit(1);
    }
    flag[i] = 0;
  }
  
  /* add */
  max = 0.0;
  t = 0;
  while(1){
    sum[t] = 0.0;
    for(i=0; i<argc-1; i++){
      if(flag[i] != EOF){
	flag[i] = fscanf(f[i], "%lf", &n[i]);
      }
      else n[i] = 0.0;
      sum[t] += n[i];
    }
    if(sum[t] > max){
      max = sum[t];
    }
    else if(sum[t]<0 && -sum[t] > max){
      max = -sum[t];
    }

    t++;
    flag2 = 1;
    for(i=0; i<argc-1; i++){
      if(flag[i] != EOF){
	flag2 = 0;
	break;
      }
    }
    if(flag2) break;
  }

  /* put */
  for(j=0; j<t; j++){
    data = sum[j]/max*SAMPLING_RATE;
    printf("%d\n",data);
  }

  return 0;
}
