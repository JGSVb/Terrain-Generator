#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <sys/ioctl.h>

#include <math.h> // pow

#define LERP(a, b, fac) ((double)((a) * (1-(fac)) + (b) * (fac)))


// Ferramentas para o terminal =) 
#define TERMGOTO(x, y)               printf("\033[%d;%dH", (y), (x))
#define TERMCLEAR()                  printf("\e[1;1H\e[2J")
#define TERMPAINTCELL(x, y, r, g, b) {TERMGOTO((x), (y)); printf("\033[48;2;%d;%d;%dm \033[0m", (r), (g), (b));}


#define MIN(a, b) ((a)<(b) ? (a) : (b))
#define MAX(a, b) ((a)<(b) ? (b) : (b))



bool inside_array(int * array, int length, int value){

	int i;

	for(i=0; i<length; i++){
		if(array[i]==value)
			return true;
	}

	return false;

}


void array_copy(int * dest, int * source, int length){
	
	int i;

	for(i=0; i<length; i++)
		dest[i] = source[i];

}


int * array_get_random(int length, int min, int max){

	int * new;
	int i;

	new = malloc(sizeof(int) * length);

	for(i=0; i<length; i++){

		// Não queremos números repetidos na array
		// Ok, afinal queremos

		// do
		new[i] = min + rand() % (max - min);
		// while(inside_array(new, i, new[i]));

	}


	return new;
}


int * array_get_interpolated(int * array, int length, int step){

	int * temp;
	int i, j, a, b;

	temp = malloc(sizeof(int) * length * step);

	for(i=0; i<length; i++){

		for(j=0; j<step; j++){

			temp[i*step+j] = LERP(array[i], array[i+1], pow((double)j/(double)step, 3));

		}

	}

	return temp;

}


// Isto é quase um fosse um box blur
void array_blur(int * array, int length, int radius, int steps){

	int * temp;
	int i, j, k, s;

	// AVG não é um antivírus, é uma abreviatura para "average", ou seja, "média", em português
	double avg;
	int halrad;

	halrad = radius/2;

	temp = malloc(sizeof(int) * length);

	for(s=0; s<steps; s++){

		for(i=0; i<length; i++){
			
			k = 1;
			for(j=MAX(0, i-halrad); j<MIN(length, i+halrad); j++){

				avg += array[i];

				k++;
			}

			avg = avg/(double)k;
			temp[i] = (int)avg;

		}

		array_copy(array, temp, length);

	}

	free(temp);

}



void array_show(int * array, int length){
	int i, k;

	for(i=0; i<length; i++){
		
		printf("%d ", array[i]);

	}

	putchar('\n');

}


	



void render_terrain(int * array, int length, int height){

	int x, y, curr;
	int r, g, b;
	int val;

	for(x=0; x<length; x++){
		for(y=0; y<height; y++){

			val = (255 - pow((double)y/height, 2)*255) * 0.5;

			TERMPAINTCELL(x, y, (int)((double)val*0.7), (int)((double)val * 1.1), (int)((double)val * 1.5));

		}

	}

	for(x=0; x<length; x++){

		curr = array[x];
		
		for(y=0; y<curr; y++){

			val = pow(((double)y/height), 2) * 200 + 50;

			TERMPAINTCELL(x, height-y, (int)((double)val*0.7), (int)((double)val * 2), (int)((double)val*0.8));

		}
	}
	
	TERMGOTO(0, height+2);

	printf("Para sair digite [ENTER]");
	scanf("*");

	TERMCLEAR();

}


int main(void){

	{
		struct timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);
		srand(ts.tv_nsec);
	}


	int * array, * final;
	
	struct winsize termdim;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &termdim);

	int step       = 5;
	
	// Tamanho do terminal dividido por 5
	// já que depois será interpolado com um passo de 5,
	// por exemplo:
	//
	// 1       2|          3|
	// 1 2 3 4 5| 6 7 8 9 10|
	//
	// O tamanho aumentou 5 vezes
	int src_length = termdim.ws_col/5;
	int dst_length = src_length * step - step;

	// 3/4 da altura do terminal
	int height     = termdim.ws_row*0.75;


	array = array_get_random(src_length, 5, height);
	final = array_get_interpolated(array, src_length, step);
	
	array_blur(final, dst_length, 5, 13);

	TERMCLEAR();
	render_terrain(final, dst_length, height);

	free(array);
	free(final);


	return 0;
}
