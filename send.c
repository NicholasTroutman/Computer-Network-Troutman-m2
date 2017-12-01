//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <fcntl.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h> //timing necessary

unsigned long long expo(unsigned long long a, unsigned long long b, unsigned long long n)
{
    if (b==1)
        return (a%n);
    if (b==2)
        return ((a*a)%n);

    if (b%2==0){
            return ((expo(expo(a,b/2,n),2,n))%n);
    }
    else{
        return ((a*expo(expo(a,(b-1)/2,n),2, n) )%n);
    }
}





void get_Keys( unsigned long long *e, unsigned long long *n, unsigned long long *d)
{
        unsigned long long p = 55639 ;
        unsigned long long q = 68767;
        *e=7;
       
        //log(n,2)
        
        *n = p * q;
        //unsigned long long totient = (p-1)*(q-1);

        //precomputed for testing
        *d = 1093143631;
        //*d = 1093143631; //solve with wolframalpha (X*e)modn =1, solve for X
      // *n= 3826127113;
        //tot = 18446744073709551615
}

void encrypt_message(unsigned long long* buff , uint32_t len)
{
        unsigned long long e;
        unsigned long long n;
        unsigned long long d;

        get_Keys(&e,&n,&d);

        //Encrypt the data with these keys
        //encryption formula: Encrypted = Message^e % n
        //message  must be less than n;

        uint8_t i = 0;
	struct timeval stop, start;

        for(i=0; i<len;i++)
        {
		gettimeofday(&start,NULL);
                buff[i]=expo(buff[i],e,n);
		//usleep(50);
		gettimeofday(&stop,NULL);
		printf("%d: %lu\n",i,stop.tv_usec-start.tv_usec);
                //encrypt_int(&buff[i],e,n);
        }
}


int main(int argc, char **argv) {
    int pru_data, pru_clock; // file descriptors
    uint8_t sinebuf[490];
    char  txbuffer[100];
    txbuffer[0] = 'g';
    int buffer_size = 0;
	int  frames;
    ssize_t readpru, writepipe, prime_char, pru_clock_command;
    // Create a file
    //  Now open the PRU1 clock control char device and start the clock.

	unsigned long long c[24]; //24*4 = 96+4, 100 total bits (including the g header)

    if(argc == 2)
    {
	printf("%d\n", strlen(argv[1]));
	buffer_size = strlen(argv[1]);
    	frames=(buffer_size-1)/3 +1;
	int i,j;

	  //sent message
         for (j=0;j<buffer_size;j++)
        {
        printf("%d: %c = %d\n",j,(char)argv[1][j],argv[1][j]);
        }

	argv[1][buffer_size]=0;
	argv[1][buffer_size+1]=0;
	argv[1][buffer_size+2]=0;


		//pass all argument characters to c

	for (i=0;i<frames;i++)
	{
		c[i]=0;

       		for (j=0;j<3;j++)
        	{
        	//printf("%d: A: %llu\n",i,c[i]);
        	c[i] = c[i]<<8;
        	//printf("%d:  B: %llu\n",i,c[i]);
		c[i] = c[i] | (unsigned long long) argv[1][j+i*3] ;
        	//printf("%d: C: %llu\n",i,c[i]);
        	}
	}

	//printf("ull preEncryption: %llu\n",c[4]);

	//encrypt
	
	encrypt_message(c,frames); //pass all of the functions

	//printf("ull postEncryption: %llu\n",c[4]);
	int place=1;
	for (i=0;i<frames;i++)
	{
		for (j=0;j<4;j++)
    		{
		// printf("%d: A: %llu\n",i,c[i]);

        	txbuffer[4*(i+1)-j] = c[i] & 0xff;
        	
	 //printf("%d: B: %llu\n",i,c[i]);
	
	c[i] = c[i] >>8;
	
		//printf("%d: C: %llu\n",i,c[i]);
		}	
	}
	
	      //printf("\nNow we conver this frame back into uint8_t:\n");

	
	//sent message 
	 for (j=0;j<frames*4+1;j++)
        {
        printf("%d: %c = %d\n",j,(char)txbuffer[j],txbuffer[j]);
        }


        pru_clock = open("/dev/rpmsg_pru31", O_RDWR);
        pru_clock_command = write(pru_clock, txbuffer, frames*4+1 );
        if (pru_clock_command < 0)
            printf("The pru clock start command failed.");
        printf("Message sent successfully... size = %d\n",frames*4+2); //no idea how big buffer should be,1|frames*4|1
    }   
}
