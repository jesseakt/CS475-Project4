/**
 * CS 475 Project 4
 * Author: Jesse Thoren
 * Date: May 1, 2017
 * Description: A project to simulate growth of populations of grain and
 *    'graindeer' subject to monthly conditions on temperature,
 *    precipitation, etc.
 * References: web.engr.oregonstate.edu/~mjb/cs575/Projects/proj04.html
 * **/

#include <omp.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

//Define Global Variables
int     NowYear;    //2017 - 2022
int     NowMonth;   //0 - 11

float   NowPrecip;  //inches of rain per month
float   NowTemp;    //temp this month
float   NowHeight;  //grain height in inches
int     NowNumDeer; //number of deer in current population
int     NowHunted;  //Deer Hunted this month


/* Units of grain growth are inches
 * Units of temperature are degress Fahrenheit
 * Units of precipitation are inches*/

const float GRAIN_GROWS_PER_MONTH =     8.0;
const float ONE_DEER_EATS_PER_MONTH =   0.5;

const float DEER_CONSERVATION_LIMIT =   3.0; //No deer hunted unless >this

const float AVG_PRECIP_PER_MONTH =      6.0; //average
const float AMP_PRECIP_PER_MONTH =      6.0; //plus or minus
const float RANDOM_PRECIP =             2.0; //plus or minus noise

const float AVG_TEMP =                  50.0;//average
const float AMP_TEMP =                  20.0;//plus or minus
const float RANDOM_TEMP =               10.0;//plus or minus noise

//Random Number Generation (Given in project outline)
float Ranf( unsigned int *seedp, float low, float high)
{
    float r = (float) rand_r( seedp ); // 0-RAND_MAX
    return ( low + r*(high-low)/(float)RAND_MAX);
}

int Ranf( unsigned int *seedp, int ilow, int ihigh)
{
    float low = (float)ilow;
    float high = (float)ihigh + 0.9999f;
    return (int)(Ranf(seedp, low, high));
}

//SQR (Given in project outline)
float SQR(float x)
{
    return x*x;
}

//setAngle is a function that produces the angle as given in project outline
float setAngle(int CurrentMonth)
{
    return (30.*(float)CurrentMonth + 15.) * (M_PI / 180.);
}

//setPrecip sets precipitation given an angle
float setPrecip(unsigned int *seed, float angle)
{
    float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin(angle);
    precip += Ranf(seed, -RANDOM_PRECIP, RANDOM_PRECIP);
    if(precip < 0.)
        precip = 0.;
    return precip;

}

//setTemp sets temperature given an angle
float setTemp(unsigned int *seed, float angle)
{
    float temp = AVG_TEMP - AMP_TEMP * cos( angle );
    return (temp + Ranf(seed, -RANDOM_TEMP, RANDOM_TEMP));
}

//GrainDeer Thread
void GrainDeer(unsigned int *seed)
{
    while(NowYear < 2023)
    {
        //Compute temp next value for this quantity
        //Add a deer if height exceeds number of deer, else subtract 1.
        int newAmount = NowNumDeer + ((float)NowNumDeer>NowHeight ? -1:1);
        //Done Computing barrier:
        #pragma omp barrier
        
        //Assign necessary quantities
        NowNumDeer = newAmount;
        //Done Assigning barrier:
        #pragma omp barrier

        //HuntUpdate
        #pragma omp barrier

        //Done Printing barrier:
        #pragma omp barrier
    }
}
//Grain Thread
void Grain( unsigned int *seed)
{
    while(NowYear < 2023)
    {
        //Compute temp next value for this quantity
        float tempFactor = exp( -SQR( (NowTemp - AVG_TEMP)/10.));
        float precipFactor = exp( -SQR( (NowPrecip - AVG_PRECIP_PER_MONTH) / 10.));
        float newHeight = NowHeight;
        newHeight += tempFactor * precipFactor * GRAIN_GROWS_PER_MONTH;
        newHeight -= (float)NowNumDeer * ONE_DEER_EATS_PER_MONTH;
        if(newHeight<0)
            newHeight = 0.;
        //Done Computing barrier:
        #pragma omp barrier

        //Assign necessary quantities
        NowHeight = newHeight;
        //Done Assigning barrier:
        #pragma omp barrier

        //HuntUpdate
        #pragma omp barrier

        //Done Printing barrier:
        #pragma omp barrier
    }
}

//Watcher Thread
void Watcher(unsigned int *seed)
{
    while(NowYear < 2023)
    {
        //Compute temp next value for this quantity
        //Done Computing barrier:
        #pragma omp barrier

        //Assign Necessary Quantities
        //Done Assigning barrier:
        #pragma omp barrier

        //Hunt Update
        #pragma omp barrier

        //Print results and increment month
        fprintf(stderr, "Year: %d, Month %d\n Temperature:     %.2f\n Precipitation:   %.2f\n Grain Height:    %.2f\n Number of Deer:  %d\n Deer Hunted:     %d\n\n", NowYear, NowMonth, NowTemp, NowPrecip, NowHeight, NowNumDeer, NowHunted);
        NowMonth++;
        if(NowMonth == 12){
            NowMonth = 0;
            NowYear++;
        }
        //Calculate new Temperature and Precipitation
        float ang = setAngle(NowMonth);
        NowTemp = setTemp(seed, ang);
        NowPrecip = setPrecip(seed, ang);
        //Done Printing Barrier
        #pragma omp barrier
    }
}
//MyAgent Thread
void MyAgent(unsigned int *seed)
{
    while(NowYear < 2023)
    {
        //Compute temp next value for this quantity
        int newHunted = 0;
        //Generate a random number between 0 and the number of deer
        int randomNum = Ranf(seed,0,NowNumDeer);
        //Integer Div by Cons limit to calc number of hunted deer
        newHunted = (int)((float)randomNum/DEER_CONSERVATION_LIMIT);

        //Done Computing barrier:
        #pragma omp barrier

        //Assign necessary quantities
        NowHunted = newHunted;
        //Done Assigning barrier:
        #pragma omp barrier

        //Hunt Update
        NowNumDeer -= NowHunted;
        #pragma omp barrier

        //Done Printing barrier:
        #pragma omp barrier
    }
}

//Main Method
int main()
{
    //Seed RNG
    unsigned int seed = 0;

    NowYear = 2017; //Starting Year
    NowMonth = 0;   //Starting Month
    NowNumDeer = 1; //Starting Amount of Deer
    NowHeight = 1.; //Starting Height

    //Confirm OpenMP is supported by current machine
    #ifndef _OPENMP
        fprintf(stderr, "OpenMP is not supported on this machine. \n");
        return 1;
    #endif

    //Set Initial Angle
    float ang = setAngle(NowMonth);

    //Set Initial Temperature
    NowTemp = setTemp(&seed, ang);
    
    //Set Initial Precipitation
    NowPrecip = setPrecip(&seed, ang);

    omp_set_num_threads( 4 );   //Same as # of sections
    #pragma omp parallel sections private(seed)
    {
        //Graindeer Calculation
        #pragma omp section
        {
            GrainDeer(&seed);
        }

        //Grain Calculation
        #pragma omp section
        {
            Grain(&seed);
        }

        //Watcher Function
        #pragma omp section
        {
            Watcher(&seed);
        }

        //MyVariable Calculation
        #pragma omp section
        {
            MyAgent(&seed);
        }
    }

    return 0;
}
