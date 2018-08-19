#ifndef PFT_STRUCT_H
#define PFT_STRUCT_H

struct PFT_struct
{
        double Shootmass;
        double Rootmass;
        double Repro;
        int Pop;

        PFT_struct() {
            Shootmass = 0;
            Rootmass = 0;
            Repro = 0;
            Pop = 0;
        }

        ~PFT_struct(){}
};

#endif // PFT_STRUCT_H
