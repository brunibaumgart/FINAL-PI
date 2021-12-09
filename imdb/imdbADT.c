#include "imdbADT.h"
#include <stdio.h>
#include <errno.h>

#define TOP 5

typedef struct movie
{
    char *name;
    size_t votes;
    float rating;
    char **genres; //Generos a los que pertence la pelicula (puede ser + de 1)
} TMovie;

typedef struct genre
{
    char *genre;                //Nombre del genero actual
    size_t types[CANT_TYPES_G]; //En cada posicion se guarda la cantidad de peliculas/series del genero
    struct genres *next;        //Puntero al siguiente genero
} TGenre;

typedef TGenre *TListGenre;

typedef struct year
{
    size_t year;                //Anio actual
    TListGenre firstGenre;      //Puntero al primer nodo da la lista de generos
    size_t types[CANT_TYPES_Y]; //En cada posicion se guarda la cantidad de cortos/peliculas/series del anio
    TMovie topFilms[TOP];       //TOP 5 peliculas mas votadas del anio
    struct year *next;          //Puntero al siguiente anio
} TYear;

typedef TYear *TListYear;

typedef struct imdbCDT
{
    TListYear first; //Puntero al primer nodo da la lista de anios
    size_t qtyYears; //Cantidad de anios
} imdbCDT;

imdbADT newImdb()
{
    return calloc(1, sizeof(imdbCDT));
}

//Agrega la pelicula/serie/corto a su anio
int addToYear(imdbADT imdb, titleTypeY type, char *title, size_t year, char *genres, double rating, long votes)
{
}

//Devuelve el anio a buscar, en caso de no ecnontrarlo retorna NULL
static TListYear searchYear(TListYear listY, int year)
{
    int value;
    if (listY == NULL || (value = compareYear(listY->year, year)) > 0)
    {
        //Implica que la lista es vacia, o que no encontrare el anio mas adelante (orden)
        return NULL;
    }

    if (value == 0)
    {
        //Hemos encontrado el anio buscado
        return listY;
    }

    //Seguimos buscando el anio en la sublista
    return searchYear(listY->next, year);
}

static TListYear addToGenreRec(TListGenre listG, titleTypeG type, char *genre, int year, int *flag)
{
    int value;
    if (listG == NULL || (value = strcmp(listG->genre, genre)) > 0)
    {
        //Debemos agregar el nuevo genero
        TListGenre newGenre = malloc(sizeof(TGenre));

        //Debemos validar si se ha logrado reservar memoria
        if (newGenre == NULL) //ERRNO???
        {
            //Si no es posible crear el nuevo nodo, entonces debo devolver la lista de todas formas
            return listG;
        }

        newGenre->genre = malloc(strlen(genre) + 1);

        //Debemos validar si se ha logrado reservar memoria
        if (newGenre->genre == NULL)
        {
            //Como el nuevo nodo se ha creado, debemos liberar la memoria que se ha reservado
            free(newGenre);
            //Si no es posible crear el nuevo nodo, entonces debo devolver la lista de todas formas
            return listG;
        }

        strcpy(newGenre->genre, genre);

        newGenre->types[type] = 1;
        //Encadenamos el nuevo nodo con la lista
        newGenre->next = listG;
        (*flag) = 1;
        return newGenre;
    }

    if (value == 0)
    {
        (*flag) = 1;
        listG->types[type]++;
        return listG;
    }

    //(value < 0) Seguimos buscando en la sublista de generos
    listG->next = addToGenreRec(listG->next, type, genre, year, flag);
    return listG;
}

//Agrega la pelicula o serie a sus generos
//Devuelve 1 si agrego, 0 si no
int addToGenre(imdbADT imdb, titleTypeG type, char *genre, int year)
{
    TListYear yearAux = searchYear(imdb->first, year);

    if (yearAux == NULL)
    {
        //No se ha encontrado el anio al que queremos agregar el genero
        return 0;
    }
    int flag = 0;
    yearAux->firstGenre = addToGenreRec(yearAux->firstGenre, type, genre, year, &flag);
    return flag;
}

//Devuelve la cantidad de peliculas que hay en ese anio
size_t filmsInYear(imdbADT imdb, size_t year);

//Devuelve la cantidad de serie que hay en ese anio
size_t seriesInYear(imdbADT imdb, size_t year);

//Devuelve la cantidad de shorts que hay en ese anio
size_t shortsInYear(imdbADT imdb, size_t year);

//Devuelve la cantidad de peliculas que hay en ese genero
size_t filmsInGenre(imdbADT imdb, char *genre);

//Devuelve la cantidad de series que hay en ese genero
size_t seriesInGenre(imdbADT imdb, char *genre);

//Devuelve las mejores rated peliculas de ese anio con su nro de votos, rating y generos
char **topFive(imdbADT imdb, size_t year);

//Libera todos los recursos del imdb
void freeImdb(imdbADT imdb)
{
    
}
