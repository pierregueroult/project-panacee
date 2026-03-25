# Genetics notes

## 1 Comment fonctionne un algorithme génétique ?
Un algorithme génétique s'inspire de la sélection naturelle de Darwin : on fait "évoluer" une population de solutions candidates vers la meilleure solution possible, génération après génération. 

## 2 Les structs genetics du projet
 
### Individu
Tout d'abord, nous aurons une struct pour caractériser un individu, où il y a une liste d'index représentant les villes où il y a un hôpital. Ainsi qu'une variable fitness calculant la fitness del'individu.
```c
typedef struct {
    int     *town_indexes;  /* indices dans towns[] des villes avec hopital */
    int      size;          /* nombre d'hôpitaux                            */
    Fitness  fitness;       /* résultat de l'évaluation                     */
} Individual;
```

### Population

Il y aura pour chaque gnération, une population avec un nombre n de population
```c
typedef struct {
    Individual *individuals;
    int         size;
} Population;
```

