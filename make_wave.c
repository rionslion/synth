#include <stdio.h>
#include <math.h>
#include <stdlib.h>

/* track status */
#define SAMPLING_RATE 44100
#define SONG_LENGTH 300
#define WAVE_STYLE_NUM 3
#define MELODY_NUM 12
#define MELODY_BASE 3
#define VOLUME_MAX 32767

#define PI 3.1416

/* wave form */
#define SIN 's'
#define TRIANGLE 't'
#define NOKOGIRI 'n'
#define PULSE 'p'

/* wave data */
double wave[SAMPLING_RATE][SONG_LENGTH];

/* track wave style */
struct wave_parameter{
  int form;
  int attack;
  int decay;
  int sustain;
  int release;
  int volume;
  int key;
};
struct wave_parameter style[WAVE_STYLE_NUM];

double freq[WAVE_STYLE_NUM];

/* velocity from adsr+v */
double velocity_start[WAVE_STYLE_NUM][SAMPLING_RATE];
double velocity_finish[WAVE_STYLE_NUM][SAMPLING_RATE];

void velocity_set(void);
void make_wave(void);
double wave_generator(int x, int t);
void output_wave(void);

int main(void){
  int i, j;

  /* initalize */
  for(i=0;i<SAMPLING_RATE;i++){
    for(j=0;j<SONG_LENGTH;j++){
      wave[i][j] = 0.0;
    }
  }

  /* wave_style load */
  for(i=0;i<WAVE_STYLE_NUM;i++){
    style[i].form = getchar();
    /* check format */
    while(style[i].form < 'a' || style[i].form > 'z'){
      style[i].form = getchar();
    }
    scanf("%d",&style[i].attack);
    scanf("%d",&style[i].decay);
    scanf("%d",&style[i].sustain);
    scanf("%d",&style[i].release);
    scanf("%d",&style[i].volume);
    scanf("%d",&style[i].key);
  }

  /* do making */
  velocity_set();
  make_wave();
  output_wave();

  return 0;
}

/* velocity setting to velocity_start and velocity_finish. */
void velocity_set(void){
  int i, j;

  for(i=0; i<WAVE_STYLE_NUM; i++){
    for(j=0; j<SAMPLING_RATE; j++){
      /* setting to start velocity */

      /* before attack timing */
      /* reject patern to devide 0. */
      if(style[i].attack == 0 && j == 0){
	velocity_start[i][j] = style[i].volume;
      }
      else if(j<=style[i].attack){
	velocity_start[i][j] = j*style[i].volume/style[i].attack;
      }

      /* after attack and before decay */
      /* reject patern to devide 0. */
      if(style[i].decay > 0){
	if(j>style[i].attack && j<=style[i].attack+style[i].decay){
	  velocity_start[i][j] = style[i].volume - ( (j-style[i].attack)*style[i].volume*(100-style[i].sustain)/(style[i].decay*100) );
	}
      }

      /* setting to finish velocity */
      /* reject patern to devide 0. */
      if(style[i].release > 0){
	if(j < style[i].attack+style[i].decay+style[i].release){
	  velocity_finish[i][j] = (style[i].volume*style[i].sustain/100) - (style[i].volume*style[i].sustain*j/(100*style[i].release));
	}
      }
    }
  }
}

/* wave data making by input track data */
void make_wave(void){
  int melody, velocity, start_sec, start_wave, finish_sec, finish_wave;
  double start_time, finish_time;
  int i, t, x, tmp;
  double melody_freq[MELODY_NUM] = {440.0, 466.164, 495.883, 523.251, 554.365, 587.330, 622.254, 659.255, 698.456, 739.989, 783.991, 830.609};

  scanf("%d",&melody);

  while(melody != 0){
    scanf("%d %lf %lf",&velocity, &start_time, &finish_time);

    /* convert to sec */
    start_sec = start_time;
    finish_sec = finish_time;

    /* convert to wave timing */
    start_wave = (start_time - start_sec) * SAMPLING_RATE;
    finish_wave = (finish_time - finish_sec) * SAMPLING_RATE;

    /* convert melody to frequency */
    for(i=0; i<WAVE_STYLE_NUM; i++){
      freq[i] = melody_freq[melody%MELODY_NUM] * (pow(2, (melody/MELODY_NUM)-MELODY_BASE)) * (pow(2, (style[i].key-2)-MELODY_BASE));
    }

    t = start_sec;  /* time */
    x = start_wave; /* heni */
    /* loop start time to finish time */
    /* wave[start_wave][start_sec] ~ wave[finish_wave][finish_sec] */
    while(1){
      if(t==finish_sec && (x%SAMPLING_RATE) == finish_wave) break;
      wave[x%SAMPLING_RATE][t] += velocity * wave_generator(x-start_wave, 0);
      x++;
      if(x%SAMPLING_RATE == 0) t++; 
    }

    tmp = x;

    /* loop finish time to more 1 sec for release*/
    while(1){
      if(t==finish_sec+1 && (x%SAMPLING_RATE) == finish_wave) break;
      wave[x%SAMPLING_RATE][t] += velocity * wave_generator(x-start_wave, x-tmp);
      x++;
      if(x%SAMPLING_RATE == 0) t++;
    }

    scanf("%d",&melody);
  }
}

/* wave data generate by melody */
double wave_generator(int x, int t){

  double tmp_wave = 0.0;
  int i, j;
  double df, f;

  /* start wave */
  if(t == 0){
    for(i=0; i<WAVE_STYLE_NUM;i++){
      /* calc diff */
      j = x/(SAMPLING_RATE/freq[i]);
      df = x-j*(SAMPLING_RATE/freq[i]);

      /* SIN wave */
      if(style[i].form == SIN){
	/* before decay */
	if(x<style[i].attack+style[i].decay){
	  tmp_wave += velocity_start[i][x] * sin(2*PI*freq[i]*df/SAMPLING_RATE);
	}
	
	/* after decay */
	else{
	  tmp_wave += (style[i].volume*style[i].sustain * sin(2*PI*freq[i]*df/SAMPLING_RATE))/100;
	}
      }
      
      /* TRIANGLE wave */
      else if(style[i].form == TRIANGLE){
	/* before decay */
	if(x<style[i].attack+style[i].decay){
	  if(df < SAMPLING_RATE/freq[i]/4){
	    tmp_wave += velocity_start[i][x] * 4*df*freq[i]/SAMPLING_RATE;
	  }
	    else if(df > 3*SAMPLING_RATE/freq[i]/4){
	    tmp_wave += velocity_start[i][x] * 4*df*freq[i]/SAMPLING_RATE - 4*velocity_start[i][x];
	    
	  }	  
	  else{
	    tmp_wave += - (velocity_start[i][x] * 4*df*freq[i]/SAMPLING_RATE) + 2*velocity_start[i][x];
	  }
	}	    
	
	/* after decay */
	else{
	  if(df < (SAMPLING_RATE/freq[i]/4)){
	      tmp_wave += (style[i].volume*style[i].sustain * 4*df*freq[i]/SAMPLING_RATE/100);
	  }
	  else if(df > 3*SAMPLING_RATE/freq[i]/4){
	    tmp_wave += (style[i].volume*style[i].sustain * 4*df*freq[i]/SAMPLING_RATE - 4*style[i].volume*style[i].sustain)/100; 
	  }
	  else{
	    tmp_wave +=( - (style[i].volume*style[i].sustain * 4*df*freq[i]/SAMPLING_RATE) + 2*style[i].volume*style[i].sustain)/100;
	  }
	}
      }
      /* NOKOGIRI wave */
      else if(style[i].form == NOKOGIRI){
	/* before decay */
	if(x<style[i].attack+style[i].decay){
	  tmp_wave += -velocity_start[i][x] + 2*velocity_start[i][x] * df*freq[i]/SAMPLING_RATE;
	}
	
	/* after decay */
	else{
	  tmp_wave += (-style[i].volume*style[i].sustain + 2*style[i].volume*style[i].sustain * df*freq[i]/SAMPLING_RATE)/100;
	}
      }
      /* PULSE wave */
      else if(style[i].form == PULSE){
	/* before decay */
	if(x<style[i].attack+style[i].decay){
	  if(df < SAMPLING_RATE/freq[i]/2){
	    tmp_wave += velocity_start[i][x];
	  }
	  else{
	    tmp_wave += -velocity_start[i][x];
	  }
	}
	
	/* after decay */
	else{
	  if(df < SAMPLING_RATE/freq[i]/2){
	    tmp_wave += style[i].volume*style[i].sustain/100;
	  }
	  else{
	    tmp_wave += -style[i].volume*style[i].sustain/100;
	  }
	}
      }
      
      /* other wave */
      else tmp_wave += 0.0;
    }
  }
  
  /* finish wave */
  else {
    for(i=0; i<WAVE_STYLE_NUM;i++){
      /* calc diff */
      j = x/(SAMPLING_RATE/freq[i]);
      df = x-j*(SAMPLING_RATE/freq[i]);

      /* SIN wave */
      if(style[i].form == SIN){
	if(t<style[i].release){
	  tmp_wave += velocity_finish[i][t] * sin(2*PI*freq[i]*df/SAMPLING_RATE);
	}
      }
      
      /* TRIANGLE wave */
      else if(style[i].form == TRIANGLE){
	if(t<style[i].release){
	  if(df < SAMPLING_RATE/freq[i]/4){
	    tmp_wave += velocity_finish[i][t] * 4*df*freq[i]/SAMPLING_RATE;
	  }
	  else if(df > 3*SAMPLING_RATE/freq[i]/4){
	    tmp_wave += velocity_finish[i][t] * 4*df*freq[i]/SAMPLING_RATE - 4*velocity_finish[i][t];
	    
	  }	  
	  else{
	    tmp_wave += - (velocity_finish[i][t] * 4*df*freq[i]/SAMPLING_RATE) + 2*velocity_finish[i][t];
	  }
	}
      }

      /* NOKOGIRI wave */
      else if(style[i].form == NOKOGIRI){
	if(t<style[i].release){
	  tmp_wave += -velocity_finish[i][t] + 2*velocity_finish[i][t] * df*freq[i]/SAMPLING_RATE;
	}
      }

      /* PULSE wave */
      else if(style[i].form == PULSE){
	if(t<style[i].release){
	  if(df < SAMPLING_RATE/freq[i]/2){
	    tmp_wave += velocity_finish[i][t];
	  }
	  else{
	    tmp_wave += -velocity_finish[i][t];
	  }
	}
      }

      /* other wave */
      else tmp_wave += 0.0;
    }
  }
  
  return tmp_wave;
  
  
}

/* output wave data */
void output_wave(void){
  int i, j, k, zero_count;

  zero_count = 0;
  for(j=0;j<SONG_LENGTH;j++){
    for(i=0;i<SAMPLING_RATE;i++){
      if(wave[i][j] > 0.01 || wave[i][j] < -0.01){
	for(k=0; k<zero_count; k++){
	  printf("0.0\n");
	}
	zero_count = 0;
	printf("%f\n",wave[i][j]);
      }
      else{
	zero_count++;
	if(zero_count > SAMPLING_RATE) break;
      }
    }
    if(zero_count > SAMPLING_RATE) break;
  }
  printf("0.0\n0.0\n0.0\n");
}

