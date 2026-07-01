typedef enum
{
    MODQPSK,
    MODD16QAM,
    MODD64QAM
} ModulationType;

int run_simulation(ModulationType mod_type,
                   const char *output_file);