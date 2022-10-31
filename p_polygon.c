// p_polygon.c

extern int      numsectors;
extern flats_t *flats;

void P_ReleaseLevel()
   {
    int sector, fc;

    for (sector = 0; sector < numsectors; sector++)
       {
        for (fc = 0; fc < flats[sector].count; fc++)
           {
            if (flats[sector].Floor != 0)
               {
                free(flats[sector].Floor[fc].Point);
               }
            if (flats[sector].Ceiling != 0)
               {
                free(flats[sector].Ceiling[fc].Point);
               }
           }
        if (flats[sector].Floor != 0)
           {
            free(flats[sector].Floor;
           }
        if (flats[sector].Ceiling != 0)
           {
            free(flats[sector].Ceiling;
           }
       }
   }