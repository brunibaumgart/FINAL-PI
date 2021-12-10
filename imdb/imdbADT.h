#include <stdio.h>

typedef struct imdbCDT *imdbADT;

#define TOP 5
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
int addToYear(imdbADT imdb, titleTypeY type, char *title, size_t year, char *genre, float rating, size_t votes);

//Agrega la pelicula o serie a sus generos
int addToGenre(imdbADT imdb, titleTypeG type, char *genres, int year);

//Inicia el iterador por anios
void toBeginYear(imdbADT imdb);

//Verifica si existe un año despues del actual, devuelve 1 si lo hay, 0 si no
int hasNextYear(imdbADT imdb);

//Avanza el iterador al siguiente año, retorna 0 en caso de algun error, 1 si no existio tal problema
int nextYear(imdbADT imdb);

//Inicia el iterador por generos (lo hace en el anio correspondiente donde se esta iterando)
void toBeginGenre(imdbADT imdb);

//Verifica si existe un siguiente genero, si es asi devuelve 1 sino 0
int hasNextGenre(imdbADT imdb);

//Avanza el iterador al siguiente genero, retorna 0 en caso de algun error, 1 si no existio tal problema
int nextGenre(imdbADT imdb);

//Inicia el iterador en la primer pelicula del top por año
void toBeginMovieTop(imdbADT imdb);

//Verifica si existe un siguiente pelicula en el top, si es asi devuelve 1 sino 0
int hasNextMovieTop(imdbADT imdb);

//Avanza el iterador a la siguiente pelicula retorna 0 en caso de algun error, 1 si no existio tal problema
int nextMovieInTop(imdbADT imdb);

size_t getTypeCant(imdbADT imdb, titleTypeY type);

char * getFilm(imdbADT imdb, titleTypeY type);

size_t getVotes(imdbADT imdb, titleTypeY type);

//Libera todos los recursos del imdb
void freeImdb(imdbADT imdb);
