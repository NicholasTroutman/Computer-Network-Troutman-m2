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
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

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
        
        //*n = p * q;
        //unsigned long long totient = (p-1)*(q-1);

        //precomputed for testing
        *d = 1093143631;
        //*d = 1093143631; //solve with wolframalpha (X*e)modn =1, solve for X
       *n= 3826127113;
        //tot = 18446744073709551615
}


void decrypt_message(unsigned long long* buff , uint32_t len)
{
        unsigned long long e;
        unsigned long long n;
        unsigned long long d;
        get_Keys(&e,&n,&d);

        //Encrypt the data with these keys
        //decryption formula: Message = (Encrypted^d) % n
        //message  must be less than n;

        uint8_t i = 0;
	struct timeval stop, start;
        for(i=0; i<len;i++)
        {
		gettimeofday(&start,NULL);
            //printf("B4: %d = %llu\n",i,buff[i]);
                buff[i]=expo(buff[i],d,n);
		gettimeofday(&stop, NULL);
		printf("%d: %lu\n", i, stop.tv_usec -start.tv_usec);
           // printf("After: %d = %llu\n\n",i,buff[i]);
            
        }
}



int main(int argc, char **argv) {
    int soundfifo, pru_data, pru_clock; // file descriptors
    uint8_t sinebuf[490];
    uint8_t final[490];
    ssize_t readpru, writepipe, prime_char, pru_clock_command;
    unsigned long long m[24]; //that is the current size, TODO #define max size
	
// Create a file
    soundfifo = open("msg.log", O_RDWR|O_CREAT, 0666);  
    if (soundfifo < 0)
      printf("Failed to open msg.log");
    printf("The node is currently listening on the light signals...\n");
	//  Open the character device to PRU0.
    pru_data = open("/dev/rpmsg_pru30", O_RDWR);
    if (pru_data < 0)
      printf("Failed to open pru character device rpmsg_pru30.");
    //  The character device must be "primed".
    prime_char = write(pru_data, "prime", 6);
    if (prime_char < 0)
      printf("Failed to prime the PRU0 char device.");
    //  This is the main data transfer loop.
    //  Note that the number of transfers is finite.
    //  This can be changed to a while(1) to run forever.
	
	  for (int i = 0; i < 1; i++) {
	    
      readpru = read(pru_data, sinebuf, 490);
	int message_size=strlen(sinebuf);;
	int l = (strlen(sinebuf))-2;
	int frames = ((l-1)/4)+1;
	printf("STRLEN(SINEBUF) = %d\nFRAMES = %d\n",l,frames);

	//decrypt
	int j;
	
	
	for (j=0;j<l;j++)

	{
	printf("%d: %c = %d\n",j,(char)sinebuf[j],sinebuf[j]);
	}
		
	int k;
	
	for (k=0;k<frames;k++)
	{
		m[k]=0;
		for (j=0;j<4;j++)
        	{
		//printf("A: %d: %llu\n",j,m[k]);
        	m[k] = m[k]<<8;
        	//printf("B: %d: %llu\n",j,m[k]);
        	m[k] = m[k] | (unsigned long long) sinebuf[j+k*4] ;
        	//printf("C: %d: %llu\n",j,m[k]);
        	}
	}

	//printf("ull preDecryption: %llu\n",m[4]);

	decrypt_message(m,l); //lenght being passed, not 1

	  // printf("ull postDecryption: %llu\n",m[4]);
      
	for (k=0;k<frames;k++)
	{
      		for (j=0;j<3;j++)
    		{
        	final[3*(k+1)-j-1] = m[k] & 0xff;
        	m[k] = m[k] >>8;
    		}
	}
      
      //printf("Now we conver this frame back into uint8_t:\n");
   	int i;     
          //  for (i=0;i<(frames*4);i++)
             //   {
        //        printf("%c = %d\n",(char)final[i], final[i]);
               // }



      writepipe = write(soundfifo, final, 490);

    printf("Encrypted: %s\n", sinebuf);
	printf("Decrypted: %s\n\n",final);    

	printf("Only %d Characters:\nEncrypted: ",frames*4);

	for (k=0;k<frames*3;k++)
	{
	printf("%c",sinebuf[k]);
	}
printf("\nDecrypted: ");
	for (k=0;k<frames*3;k++)
	{
	printf("%c",final[k]);
	}
	printf("\n");

}
    //printf("The last value of readpru is %d and the last value of writepipe is "%d.\n", readpru, writepipe);
}
