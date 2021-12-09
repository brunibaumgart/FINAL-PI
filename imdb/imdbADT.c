#include "imdbADT.h"
#include <stdio.h>
#include <errno.h>

#define TOP 5
#define BLOCK 10

typedef struct movie
{
    char *name;
    size_t votes;
    float rating;
    char *genres; //Generos a los que pertence la pelicula (puede ser + de 1 separados con coma)
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
    TMovie *topFilms;           //TOP 5 peliculas mas votadas del anio
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

static int compareYear(int y1, int y2)
{
    return y1 - y2;
}

static char *copy(const char *s)
{
    char *aux = NULL;
    int dim = 0;
    for (int i = 0; s[i]; i++)
    {
        //vamos a agrandar de a bloques
        if (dim % BLOCK == 0)
        {
            aux = realloc(aux, (dim + BLOCK) * sizeof(char));

            if (aux == NULL)
            {
                return NULL;
            }
        }
        aux[dim++] = s[i];
    }
    aux = realloc(aux, (dim + 1) * sizeof(char));
    aux[dim] = '\0';
    return aux;
}

static TMovie copyMovie(char *title, double rating, long votes, char *genres)
{
    TMovie movie;
    movie.name = malloc(strlen(title) + 1);
    strcpy(movie.name, title);
    movie.genres = malloc(strlen(genres) + 1);
    strcpy(movie.genres, genres);
    movie.rating = rating;
    movie.votes = votes;
    return movie;
}

static TMovie *updateTopFive(TMovie *topFive, char *title, double rating, long votes, char *genres)
{
    TMovie *topFiveAux = NULL;
    size_t newDim = 0;
    size_t votoMax;

    //FIJARSE SI TOPFIVE ESTA VACIO
    for (int i = 0; i < TOP; i++)
    {
        if (topFive[i].votes <= votes)
        {
            topFiveAux = realloc(topFiveAux, sizeof(TMovie) * TOP);
            int j;
            for (j = 0; j < i; j++)
            {
                topFiveAux[j] = copyMovie(topFive[j].name, topFive[j].rating, topFive[j].votes, topFive[j].genres);
            }
            if (topFive[i].votes == votes && strcmp(topFive[j].name, title) > 0)
            {
                topFiveAux[j++] = copyMovie(title, rating, votes, genres);
                topFiveAux[j++] = copyMovie(topFive[i].name, topFive[i].rating, topFive[i].votes, topFive[i].genres);
            }
            else
            {
                topFiveAux[j++] = copyMovie(title, rating, votes, genres);
            }
            for (; j < TOP; j++)
            {
                topFiveAux[j] = copyMovie(topFive[j + 1].name, topFive[j + 1].rating, topFive[j + 1].votes, topFive[j + 1].genres);
            }
        }
    }
}

static TListYear addToYearRec(TListYear listY, titleTypeY type, char *title, size_t year, char *genre, double rating, long votes)
{
    int c;
    if (listY == NULL || (c = compareYear(listY->year, year)) > 0)
    {
        TListYear newYear = calloc(1, sizeof(TYear));
        //Chequear si hay lugar
        if (newYear == NULL) //FALTA ERRNO
        {
            return listY;
        }
        newYear->year = year;
        newYear->topFilms = updateTopFive(newYear->topFilms, title, rating, votes, genre);
        newYear->types[type] = 1;
        newYear->next = listY;
    }
    if (c == 0)
    {
        listY->topFilms = updateTopFive(listY->topFilms, title, rating, votes, genre);
        listY->types[type]++;
    }
    listY->next = addToYearRec(listY->next, type, title, year, genre, rating, votes);
    return listY;
}

//Agrega la pelicula/serie/corto a su anio
int addToYear(imdbADT imdb, titleTypeY type, char *title, size_t year, char *genres, double rating, long votes)
{
    int flag = 0; //Se prende si se crea un nuevo anio
    imdb->first = addToYearRec(imdb->first, type, title, year, genres, rating, votes);
}

//Devuelve el anio a buscar, en caso de no encontrarlo retorna NULL
static TListYear searchYear(TListYear listY, int year)
{
    int c;
    if (listY == NULL || (c = compareYear(listY->year, year)) > 0)
    {
        //Implica que la lista es vacia, o que no encontrare el anio mas adelante (orden)
        return NULL;
    }

    if (c == 0)
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
        TListGenre newGenre = calloc(1, sizeof(TGenre));

        //Debemos validar si se ha logrado reservar memoria
        if (newGenre == NULL || errno == ENOMEM) //ERRNO???
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
        listG->types[type]++;
        (*flag) = 1;
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
