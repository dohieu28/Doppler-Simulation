#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "..\inc\config_utils.h"
#include "..\inc\simulation.h"

int main()
{
    srand(time(NULL));

    run_simulation(MODQPSK,
                   "output/output_ai_qpsk.csv");

    run_simulation(MODD16QAM,
                   "output/output_ai_16qam.csv");

    run_simulation(MODD64QAM,
                   "output/output_ai_64qam.csv");

    printf("Done!\n");
}