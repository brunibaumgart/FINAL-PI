#include "imdbADT.h"
#include <stdio.h>
#define TOP 5

typedef enum titleTypeY
{
    SHORT_Y = 0,
    FILM_Y,
    SERIES_Y,
    CANT_TYPES_Y
} titleTypeY;

typedef enum titleTypeG
{
    FILM_G = 0,
    SERIES_G,
    CANT_TYPES_G
} titleTypeG;

typedef struct film
{
    char *name;
    size_t votes;
    float rating;
    char **genres; //Generos a los que pertence la pelicula (puede ser + de 1)
} TFilm;

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
    TFilm topFilms[TOP];        //TOP 5 peliculas mas votadas del anio
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
int addToYear(imdbADT imdb, char *type, char *title, size_t year, char *genres, double rating, long votes)
{
}

//Devuelve el anio a buscar, en caso de no ecnontrarlo retorna NULL
static TListYear searchYear(TListYear listY, int year)
{
    int value;
    if (listY == NULL || (value = compareYear(listY->year, year)) > 0)
    {
        //implica que la lista es vacia, o que no encontrare el anio mas adelante (orden)
        return NULL;
    }

    if (value == 0)
    {
        //hemos encontrado el anio buscado
        return listY;
    }

    //seguimos buscando el anio en la sublista
    return searchYear(listY->next, year);
}

//Agrega la pelicula o serie a sus generos
int addToGenre(imdbADT imdb, char *type, char *genres, int year)
{
    TListYear yearAux = searchYear(imdb->first, year);

    if (yearAux == NULL)
    {
        return 0;
    }

    imdb->first = addToGenreRec(imdb->first, type, genres, year);
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
void freeImdb(imdbADT imdb);
