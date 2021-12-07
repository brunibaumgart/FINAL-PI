#include <stdio.h>

typedef struct imdbCDT *imdbADT;

//Crea un nuevo imdbADT vacio
imdbADT newImdb();

//Agrega la pelicula/serie/corto a su anio
int addToYear(imdbADT imdb, char *type, char *title, size_t year, char **genres, double rating, long votes);

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
