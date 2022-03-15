#include "mbed.h"
#include <stdlib.h>

#define N 1000
#define thresh 0.51

Serial pc(USBTX, USBRX);
SPI spi(D11, D12, D13);  
//MOSI(D11, Nucleo) ---> SDI(4132-103, chip) , miso(D12, Nucleo) ---> SDO(4132, Nucleo),
//sclk(D13, Nucleo) ---> sclk(4132-103)

DigitalOut cs(D7); 
// CS(D7, Nucleo) ---> CS(4132-103, CS)
//To set the resistor to approx 10k Ohms the first byte should be ?? and the //second byte should be ??
//To set the resistor to approx 75 ohm the first byte should be ?? and the //second byte should be ??

DigitalIn low(D2); // When held high, sets the rheostat resistance to 10k (Gain of )
DigitalIn med(D3); // When held high, sets the rheostat resistance to 7.5k (Gain of )
DigitalIn high(D4); // When held high, sets the rheostat resistance to 5.5k (Gain of )

AnalogIn input(A5);
DigitalOut output(D2);

Ticker gain_check;
Ticker sampler;

void write_rheostat(unsigned char res);
void process_input();
void adjust_gain();

double buffer[N];
double sum;
int curi;

int main() {    
    sum = 0;
    curi = 0;
    
    write_rheostat(0x45);
    
    adjust_gain();
    sampler.attach(&process_input, 1.0/N);
}

void process_input()
{
    sum -= buffer[curi];
    
    double cur = input;
    cur = 0.5 + abs(input - 0.5);
    buffer[curi] = cur;
    
    sum += cur;
    
    if (curi == 0 || curi == N/2)
    {
        if (sum/N >= thresh)
        {
            output = 1;
        }
        else
        {
            output = 0;
        }
    }
    
    
    curi = (curi + 1) % N;
    sampler.attach(&process_input, 1.0/N);
}

void adjust_gain()
{
    unsigned char res = 0x04;
    write_rheostat(res);
    return;
    
    if (med == 1)
    {
        res = 0x5F;
    }
    else if (high == 1)
    {
        res = 0x45;
    }

    write_rheostat(res);
}

void write_rheostat(unsigned char res)
{
    //Chip must be deselected (aka set high) This tells the device "DO NOT LISTEN //TO ME"
    cs = 1;
    
    spi.format(8,0);     // Setup the spi for 8 bit data, high steady state clock,
    spi.frequency(10000000); //set SCLK to 10MHz

    //Before reading or writing the chip must set the chip select low
    //Tells the device HEY I'M TALKING TO YOU, LISTEN TO ME
    cs = 0;
 
    //This is where we TALK to the device and tell it what you want.
    pc.printf("Writing to SPI\n");

    //First byte (aka first 8 bits) Tell the chip WHERE you want read/write AND
    //if you want READ or WRITE
    //the “0x” means to "interpret the numbers that follow as hexadecimal".
    spi.write(0x00);

    //The second byte (aka second 8 bits) Tell the chip WHAT you are WRITING
    spi.write(res);
    //spi.write(0x45);  // 5.5k
    //spi.write(0x5F); // 7.5k
    //spi.write(0xFF); // 10k

    // Set the chip select to 1 to tell the device "I AM DONE TALKING TO YOU, STOP          
    // LISTENING TO ME"
    cs = 1;
}
