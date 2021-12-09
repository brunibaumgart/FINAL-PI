#include <stdio.h>

typedef struct imdbCDT *imdbADT;

#define TMOVIE "movie"
#define TSHORT "short"
#define TSERIES "tvSeries"
#define TMSERIES "tvMiniSeries"

typedef enum titleTypeG
{
    MOVIE_G = 0,
    SERIES_G,
    CANT_TYPES_G
} titleTypeG;

typedef enum titleTypeY
{
    SHORT_Y = 0,
    MOVIE_Y,
    SERIES_Y,
    CANT_TYPES_Y
} titleTypeY;

//Crea un nuevo imdbADT vacio
imdbADT newImdb();

//Agrega la pelicula/serie/corto a su anio
int addToYear(imdbADT imdb, titleTypeY type, char *title, size_t year, char *genres, double rating, long votes);

//Agrega la pelicula o serie a sus generos
int addToGenre(imdbADT imdb, titleTypeG type, char *genres, int year);

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
