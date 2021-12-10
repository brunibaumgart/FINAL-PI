#include "imdbADT.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK 10

typedef struct movie
{
    char *name;
    size_t votes;
    float rating;
    char *genres; //Generos a los que pertence la pelicula (puede ser + de 1 separados con coma)
    struct movie *next;
} TMovie;

typedef TMovie *TListMovie;

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
    TListGenre firstGenre;      //Puntero al primer nodo de la lista de generos
    size_t types[CANT_TYPES_Y]; //En cada posicion se guarda la cantidad de cortos/peliculas/series del anio
    TListMovie firstInTop;      //Puntero al primer nodo de la lista de las TOP mejores peliculas del anio
    TListMovie lastAddedInTop;  //Para recorrer desde la ultima pelicula aniadida hasta el final en caso de necesitar liberar el ultimo (por si el usuario quiere poner TOP en algun valor grande)
    size_t sizeTop;             //Cantidad de peliculas en el top (maximo TOP)
    struct year *next;          //Puntero al siguiente anio
} TYear;

typedef TYear *TListYear;

typedef struct imdbCDT
{
    TListYear firstYear;  //Puntero al primer nodo da la lista de anios
    TListYear iterYear;   //Iterador para acceder a los campos correspondientes por anio
    TListGenre iterGenre; //Iterador para acceder a los campos correspondiente por genero
    TListMovie iterMovie; ////Iterador para acceder a los campos correspondiente por pelicula
} imdbCDT;

imdbADT newImdb()
{
    return calloc(1, sizeof(imdbCDT));
}

//Devuelve < 0, = 0 o > 0 con el fin de comparar tanto anios como votos
static int compare(size_t e1, size_t e2)
{
    return e1 - e2;
}

//Devuelve una copia del string s, o NULL en caso de que no se haya logrado reservar memoria
static char *copy(const char *s)
{
    char *aux = NULL;
    int dim = 0;
    for (int i = 0; s[i]; i++)
    {
        //Vamos a agrandar de a bloques
        if (dim % BLOCK == 0)
        {
            aux = realloc(aux, (dim + BLOCK) * sizeof(char));
            //Si no pudimos reservar memoria
            if (aux == NULL || errno == ENOMEM)
            {
                return NULL;
            }
        }
        aux[dim++] = s[i];
    }
    aux = realloc(aux, (dim + 1) * sizeof(char));

    if (aux == NULL || errno == ENOMEM)
    {
        return NULL;
    }

    aux[dim] = '\0';
    return aux;
}

//Elimina el ultimo elemento del top (el menos votado o aquel que tiene menor orden alfabetico)
static void deleteLastTop(TListMovie listM)
{
    if (listM == NULL)
        return;
    if (listM->next == NULL)
    {
        free(listM);
        return;
    }
    deleteLastTop(listM->next);
}

//Comapara por cantidad de votos, y por orden alfabetico en caso de ser necesario
static int compareTop(size_t v1, size_t v2, char *t1, char *t2)
{
    int out = compare(v1, v2);
    if (out == 0)
    {
        //Solo tomo en cuenta el orden alfabetico si la cantidad de votos es la misma
        out = strcmp(t2, t1);
    }
    return out;
}

//Actualiza el top 5 de peliculas
//Devuelve una lista ordenada de las peliculas con mas votos
static TListMovie updateTopRec(TListMovie listM, TListMovie *lastM, char *title, char *genres, float rating, size_t votes, size_t top, int *flag)
{
    int c;
    //Hice TOP recorridos
    if (top == 0)
    {
        return listM;
    }
    //Inserto en orden descendente por cantidad de votos, o en caso de ser iguales, por orden alfabetico
    if (listM == NULL || (c = compareTop(listM->votes, votes, listM->name, title)) < 0)
    {
        TListMovie newMovie = malloc(sizeof(TMovie));

        //Debemos chequear si se ha podido reservar memoria
        if (newMovie == NULL || errno == ENOMEM)
        {
            return listM;
        }
        newMovie->name = copy(title);
        newMovie->votes = votes;
        newMovie->rating = rating;
        newMovie->genres = copy(genres);
        newMovie->next = listM;
        *lastM = newMovie;
        (*flag) = 1;
        return newMovie;
    }
    //Siempre inserto
    listM->next = updateTopRec(listM, lastM, title, genres, rating, votes, top - 1, flag);
    return listM;
}

//Carga los datos a un nuevo anio, o los actualiza en caso de que ya existiera
static TListYear addToYearRec(TListYear listY, titleTypeY type, char *title, size_t year, char *genres, float rating, size_t votes, int *added)
{
    int c;
    if (listY == NULL || (c = compare(listY->year, year)) < 0)
    {
        TListYear newYear = calloc(1, sizeof(TYear));
        //Debemos chequear si se ha podido reservar memoria
        if (newYear == NULL || errno == ENOMEM)
        {
            return listY;
        }
        newYear->year = year;
        if (type == MOVIE_Y)
        {
            int flag = 0;
            newYear->firstInTop = updateTopRec(newYear->firstInTop, &newYear->lastAddedInTop, title, genres, rating, votes, TOP, &flag);
            newYear->sizeTop = 1;
        }
        (*added) = 1;
        newYear->types[type] = 1;
        newYear->next = listY;
        return newYear;
    }
    if (c == 0)
    {
        if (type == MOVIE_Y)
        {
            int flag = 0;
            listY->firstInTop = updateTopRec(listY->firstInTop, &listY->lastAddedInTop, title, genres, rating, votes, TOP, &flag);
            listY->sizeTop += flag;
            if (listY->sizeTop > TOP)
            {
                //Si supero el maximo, al haber desplazado las peliculas en una posicion, debo eliminar la ultima del top
                deleteLastTop(listY->lastAddedInTop);
                listY->sizeTop--;
            }
        }
        listY->types[type]++;
        return listY;
    }
    listY->next = addToYearRec(listY->next, type, title, year, genres, rating, votes, added);
    return listY;
}

int addToYear(imdbADT imdb, titleTypeY type, char *title, size_t year, char *genres, float rating, size_t votes)
{
    int flag = 0; //Indica si se ha agregado un nuevo anio
    imdb->firstYear = addToYearRec(imdb->firstYear, type, title, year, genres, rating, votes, &flag);
    return flag;
}

//Devuelve el anio a buscar, en caso de no encontrarlo retorna NULL
static TListYear searchYear(TListYear listY, int year)
{
    int c;
    if (listY == NULL || (c = compare(listY->year, year)) < 0)
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

//Carga los datos a un nuevo genero, o los actualiza en caso de que ya existiera
static TListGenre addToGenreRec(TListGenre listG, titleTypeG type, char *genre, int year, int *flag)
{
    int value;
    if (listG == NULL || (value = strcmp(listG->genre, genre)) > 0)
    {
        //Debemos agregar el nuevo genero
        TListGenre newGenre = calloc(1, sizeof(TGenre));

        //Debemos chequear si se ha logrado reservar memoria
        if (newGenre == NULL || errno == ENOMEM)
        {
            //Si no es posible crear el nuevo nodo, entonces debo devolver la lista de todas formas
            return listG;
        }

        newGenre->genre = copy(genre);

        //Debemos chequear si se ha logrado reservar memoria
        if (newGenre->genre == NULL || errno == ENOMEM) //ERRNO = ENOMEM ES NECESARIO?
        {
            //Como el nuevo nodo se ha creado, debemos liberar la memoria que se ha reservado
            free(newGenre);
            //Si no es posible crear el nuevo nodo, entonces debo devolver la lista de todas formas
            return listG;
        }
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

int addToGenre(imdbADT imdb, titleTypeG type, char *genre, int year)
{
    TListYear yearAux = searchYear(imdb->firstYear, year);

    if (yearAux == NULL)
    {
        //No se ha encontrado el anio al que queremos agregar el genero
        return 0;
    }
    int flag = 0; //Indica si se ha agregado un nuevo genero
    yearAux->firstGenre = addToGenreRec(yearAux->firstGenre, type, genre, year, &flag);
    return flag;
}

void toBeginYear(imdbADT imdb)
{
    imdb->iterYear = imdb->firstYear;
}

int hasNextYear(imdbADT imdb)
{
    return imdb->iterYear != NULL;
}

int nextYear(imdbADT imdb)
{
    if (!hasNextYear(imdb))
    {
        return 0;
    }
    imdb->iterYear = imdb->iterYear->next;
}

void toBeginGenre(imdbADT imdb, size_t year)
{
    TListYear listY = searchYear(imdb->firstYear, year);
    imdb->iterGenre = listY->firstGenre;
}

int hasNextGenre(imdbADT imdb)
{
    return imdb->iterGenre != NULL;
}

int nextGenre(imdbADT imdb)
{
    if (!hasNextYear(imdb))
    {
        return 0;
    }
    imdb->iterGenre = imdb->iterGenre->next;
}

void toBeginMovieTop(imdbADT imdb, size_t year)
{
    TListYear listY = searchYear(imdb->firstYear, year);
    imdb->iterMovie = listY->firstInTop;
}

int hasNextMovieTop(imdbADT imdb)
{
    return imdb->iterMovie != NULL;
}

int nextMovieInTop(imdbADT imdb)
{
    if (!hasNextMovieTop(imdb))
        return 0;
    imdb->iterMovie = imdb->iterMovie->next;
}

int getYear(imdbADT imdb)
{
    if (imdb == NULL || imdb->iterYear == NULL)
        return ERROR;
    return imdb->iterYear->year;
}

int getTypeCant(imdbADT imdb, titleTypeY type)
{
    if (imdb == NULL || imdb->iterYear == NULL)
        return ERROR;
    return imdb->iterYear->types[type];
}

char *getGenre(imdbADT imdb)
{
    if (imdb == NULL || imdb->iterGenre == NULL)
        return NULL;
    char *out = copy(imdb->iterGenre->genre);
    return out;
}

int getTypeInGenre(imdbADT imdb, titleTypeG type)
{
    if (imdb == NULL || imdb->iterGenre == NULL)
        return ERROR;
    return imdb->iterGenre->types[type];
}

char *getMovie(imdbADT imdb)
{
    if (imdb == NULL || imdb->iterMovie == NULL)
        return NULL;
    char *title = copy(imdb->iterMovie->name);
    return title;
}

int getVotes(imdbADT imdb)
{
    if (imdb == NULL || imdb->iterMovie == NULL)
        return ERROR;
    return imdb->iterMovie->votes;
}

float getRating(imdbADT imdb)
{
    if (imdb == NULL || imdb->iterMovie == NULL)
        return ERROR;
    return imdb->iterMovie->rating;
}

char *getTopGenres(imdbADT imdb)
{
    if (imdb == NULL || imdb->iterMovie == NULL)
        return NULL;
    char *out = copy(imdb->iterMovie->genres);
    return out;
}

//Libera la memoria reservada para los generos de manera recursiva (listas)
static void freeGenreRec(TListGenre listG)
{
    if (listG == NULL)
    {
        return;
    }
    freeGenreRec(listG->next);
    free(listG);
}

//Libera la memoria reservada para el top de peliculas de manera recursiva (listas)
static void freeTopRec(TListMovie listM)
{
    if (listM == NULL)
    {
        return;
    }
    freeTopRec(listM->next);
    free(listM);
}

//Libera la memoria reservada para los anios de manera recursiva (listas)
static void freeYearRec(TListYear listY)
{
    if (listY == NULL)
    {
        return;
    }
    freeTopRec(listY->firstInTop);
    freeGenreRec(listY->firstGenre);
    freeYearRec(listY->next);
    free(listY);
}

void freeImdb(imdbADT imdb)
{
    freeYearRec(imdb->firstYear);
    free(imdb);
}
