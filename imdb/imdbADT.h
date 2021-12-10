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

//Crea un nuevo imdbADT vacio, devolviendo un puntero a la memoria asignada, o NULL si no se ha logrado asignar
imdbADT newImdb();

//Agrega la pelicula/serie/corto a su anio
//En caso de ser una pelicula, se verifica si la misma debe agregarse al top 5 o no,
//teniendo en cuenta la cantidad de votos o el orden alfabetico de ser necesario
int addToYear(imdbADT imdb, titleTypeY type, char *title, size_t year, char *genre, float rating, size_t votes);

//Agrega la pelicula o serie a sus generos
//Devuelve 1 si se ha podido agregar, o 0 en caso contrario
int addToGenre(imdbADT imdb, titleTypeG type, char *genres, int year);

//Inicia el iterador por anios
void toBeginYear(imdbADT imdb);

//Verifica si existe un anio despues del actual, si es asi devuelve 1, o 0 en caso contrario
int hasNextYear(imdbADT imdb);

//Avanza el iterador al siguiente anio, retorna 0 en caso de algun error, 1 si no existio tal problema
int nextYear(imdbADT imdb);

//Inicia el iterador por generos (lo hace en el anio correspondiente donde se esta iterando)
void toBeginGenre(imdbADT imdb);

//Verifica si existe un siguiente genero, si es asi devuelve 1, o 0 en caso contrario
int hasNextGenre(imdbADT imdb);

//Avanza el iterador al siguiente genero, retorna 0 si se produjo un , 1 en caso contrario
int nextGenre(imdbADT imdb);

//Inicia el iterador en la primer pelicula del top por anio
void toBeginMovieTop(imdbADT imdb);

//Verifica si existe una siguiente pelicula en el top, si es asi devuelve 1, o 0 en caso contrario
int hasNextMovieTop(imdbADT imdb);

//Avanza el iterador a la siguiente pelicula retorna 0 si se produjo un error, 1 en caso contrario
int nextMovieInTop(imdbADT imdb);

//Devuelve la cantidad de cortos/peliculas/series (segun su tipo) que hubo en el anio donde se encuentra el iterador de anios
//En caso de producirse un error, devuelve 0
size_t getTypeCant(imdbADT imdb, titleTypeY type);

//Devuelve una copia del titulo de la pelicula mas votada del anio en el que se encuentra el iterador de peliculas
//En caso de producirse un error, devuelve 0
char *getMovie(imdbADT imdb);

//Devuelve los votos de la pelicula mas votada del anio en el que se encuentra el iterador de peliculas
size_t getVotes(imdbADT imdb);

//Devuelve el rating de la pelicula mas votada del anio en el que se encuentra el iterador de peliculas
float getRating(imdbADT imdb);

//Devuelve los generos a los que pertenece la pelicula mas votada del anio en el que se encuentra el iterador de peliculas
char *getGenre(imdbADT imdb);

//Libera todos los recursos del imdb
void freeImdb(imdbADT imdb);
