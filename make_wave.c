#include <stdio.h>
#include <math.h>
#include <stdlib.h>

/* track status */
#define SAMPLING_RATE 44100
#define SONG_LENGTH 300
#define WAVE_STYLE_NUM 3
#define MELODY_NUM 74

#define PI 3.1416

/* wave form */
#define SIN 's'
#define TRIANGLE 't'
#define NOKOGIRI 'n'
#define PULSE 'p'

/* wave data */
int wave[SAMPLING_RATE][SONG_LENGTH];

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

/* velocity from adsr+v */
int velocity_start[WAVE_STYLE_NUM][SAMPLING_RATE];
int velocity_finish[WAVE_STYLE_NUM][SAMPLING_RATE];

void velocity_set(void);
void make_wave(void);
double wave_generator(int melody, int x, int flag);
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
    scanf("%c",&style[i].form);
    /* check format */
    while(style[i].form < 'a' || style[i].form > 'z'){
      scanf("%c",&style[i].form);
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
	if(j < style[i].release){
	  velocity_finish[i][j] = (style[i].volume*style[i].sustain/100) - (style[i].volume*style[i].sustain*j/(100*style[i].release));
	}
      }
    }
  }
}

void make_wave(void){
  int melody, velocity, start_sec, start_wave, finish_sec, finish_wave;
  double start_time, finish_time;
  int t, x, tmp;

  scanf("%d",&melody);

  while(melody != 0){
    scanf("%d %lf %lf",&velocity, &start_time, &finish_time);

    /* convert to sec */
    start_sec = start_time;
    finish_sec = finish_time;

    /* convert to wave timing */
    start_wave = (start_time - start_sec) * SAMPLING_RATE;
    finish_wave = (finish_time - finish_sec) * SAMPLING_RATE;

    t = start_sec;  /* time */
    x = start_wave; /* heni */
    /* loop start time to finish time */
    /* wave[start_wave][start_sec] ~ wave[finish_wave][finish_sec] */
    while(1){
      if(t==finish_sec && (x%SAMPLING_RATE)==finish_wave) break;
      wave[x%SAMPLING_RATE][t] += velocity * wave_generator(melody, x-start_wave, 0);
      x++;
      if(x%SAMPLING_RATE == 0) t++; 
    }

    t = finish_sec;
    /* loop finish time to more 1 sec for release*/
    while(1){
      if(t==finish_sec+1 && (x%SAMPLING_RATE) == finish_wave) break;
      wave[x%SAMPLING_RATE][t] += velocity * wave_generator(melody, x-start_wave, 1);
      x++;
      if(x%SAMPLING_RATE == 0) t++;
    }

    scanf("%d",&melody);
  }
}

double wave_generator(int melody, int x, int flag){
  double freq[MELODY_NUM] = {0.0, 
      32.703, 34.648, 36.708, 38.891, 41.203, 43.654, 46.249, 48.999, 51.913, 55.0, 58.270, 61.735,
      65.406, 69.296, 73.416, 77.782, 82.407, 87.307, 92.499, 97.999, 103.826, 110.0, 116.541, 123.471,
      130.813, 138.591, 146.832, 155.563, 164.814, 174.614, 184.997, 195.998, 207.652, 220.0, 233.082, 246.942,
      261.626, 277.183, 293.665, 311.127, 329.628, 349.228, 369.994, 391.995, 415.305, 440.0, 466.164, 495.883,
      523.251, 554.365, 587.330, 622.254, 659.255, 698.456, 739.989, 783.991, 830.609, 880.0, 932.328, 987.767,
      1046.502, 1108.731, 1174.659, 1244.508, 1318.510, 1396.913, 1479.978, 1567.982, 1661.219, 1760.0, 1864.655, 1975.533,
      2093.005};

  double tmp_wave = 0.0;
  int i;

  /* start wave */
  if(flag == 0){
    for(i=0; i<WAVE_STYLE_NUM;i++){
      /* SIN wave */
      if(style[i].form == SIN){
	/* before decay */
	if(x<style[i].attack+style[i].decay){
	  tmp_wave += velocity_start[i][x] * sin(2*PI*freq[melody+12*(style[i].key-2)]*x/SAMPLING_RATE);
	}
	/* after decay */
	else{
	  tmp_wave += (style[i].volume*style[i].sustain * sin(2*PI*freq[melody+12*(style[i].key-2)]*x/(SAMPLING_RATE)))/100;
	}
      }
      /* other wave */
      else tmp_wave += 0.0;
    }
  }

  /* finish wave */
  if(flag == 1){
    for(i=0; i<WAVE_STYLE_NUM;i++){
      /* SIN wave */
      if(style[i].form == SIN){
	if(x<style[i].release){
	  tmp_wave += velocity_finish[i][x] * sin(2*PI*freq[melody+12*(style[i].key-2)]*x/SAMPLING_RATE);
	}
      }

      /* other wave */
      else tmp_wave += 0.0;
    }
  }

  return tmp_wave;


}

void output_wave(void){
  int i, j, fin_flag;

  for(j=0;j<SONG_LENGTH;j++){
    fin_flag = 1;
    for(i=0;i<SAMPLING_RATE;i++){
      printf("%d\n",wave[i][j]);
      if(wave[i][j] != 0) fin_flag = 0;
      if(wave[i][j] >60000){
	printf("BIG %d %d\n",i,j);
	exit(1);
      }
    }
    if(fin_flag) break;
  }
}

