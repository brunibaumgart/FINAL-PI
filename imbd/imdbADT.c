#include "imdbADT.h"
#include <stdio.h>
#define TOP 5

typedef struct film
{
    char *name;
    long votes;
    double rating;
    char **genres; //Generos a los que pertence la pelicula (puede ser + de 1)
} TFilm;

typedef struct genre
{
    char *genre;         //Nombre del genero actual
    size_t qtyFilms;     //Cantidad de peliculas en el genero
    size_t qtySeries;    //Cantidad de series en el genero
    struct genres *next; //Puntero al siguiente genero
} TGenre;

typedef TGenre *TListGenre;

typedef struct year
{
    size_t year;           //Anio actual
    TListGenre firstGenre; //Puntero al primer nodo da la lista de generos
    size_t qtyFilms;       //Cantidad de peliculas en el anio
    size_t qtySeries;      //Cantidad de series en el anio
    size_t qtyShorts;      //Cantidad de cortos en el anio
    TFilm topFilms[TOP];   //TOP 5 peliculas mas votadas del anio
    struct year *next;     //Puntero al siguiente anio
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
int addToYear(imdbADT imdb, char *type, char *title, size_t year, char **genres, double rating, long votes)
{

}

//Agrega la pelicula o serie a sus generos
int addToGenre(imdbADT imdb, char *type, char **genres);

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
char ** topFive(imdbADT imdb, size_t year);

//Libera todos los recursos del imdb
void freeImdb(imdbADT imdb);
