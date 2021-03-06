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
    struct genre *next;         //Puntero al siguiente genero
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

//Devuelve < 0, = 0 o > 0 con el fin de comparar tanto anios como votos
static int compare(size_t e1, size_t e2);

//Devuelve una copia del string s, o NULL en caso de que no se haya logrado reservar memoria
static char *copy(const char *s);

//Elimina el ultimo elemento del top (el menos votado o aquel que tiene menor orden alfabetico)
static TListMovie deleteLastTop(TListMovie listM);

//Compara por cantidad de votos, y por orden alfabetico en caso de ser necesario
static int compareTop(size_t v1, size_t v2, char *t1, char *t2);

//Actualiza el top 5 de peliculas
//Devuelve una lista ordenada de las peliculas con mas votos
static TListMovie updateTopRec(TListMovie listM, TListMovie *lastM, char *title, char *genres, float rating, size_t votes, size_t top, int *flag);

//Carga los datos a un nuevo anio, o los actualiza en caso de que ya existiera
static TListYear addToYearRec(TListYear listY, titleTypeY type, char *title, size_t year, char *genres, float rating, size_t votes, int *added);

//Devuelve el anio a buscar, en caso de no encontrarlo retorna NULL
static TListYear searchYear(TListYear listY, int year);

//Carga los datos a un nuevo genero, o los actualiza en caso de que ya existiera
static TListGenre addToGenreRec(TListGenre listG, titleTypeG type, char *genre, int year, int *flag);

//Libera la memoria reservada para los generos de manera recursiva (listas)
static void freeGenreRec(TListGenre listG);

//Libera la memoria reservada para el top de peliculas de manera recursiva (listas)
static void freeTopRec(TListMovie listM);

//Libera la memoria reservada para los anios de manera recursiva (listas)
static void freeYearRec(TListYear listY);

imdbADT newImdb()
{
    return calloc(1, sizeof(imdbCDT));
}

static int compare(size_t e1, size_t e2)
{
    return e1 - e2;
}

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
            //Si no pudimos reservar memoria en el heap, retorno NULL
            if (aux == NULL || errno == ENOMEM)
            {
                return NULL;
            }
        }
        aux[dim++] = s[i];
    }
    aux = realloc(aux, (dim + 1) * sizeof(char));

    //Si no pudimos reservar memoria en el heap, retorno NULL
    if (aux == NULL || errno == ENOMEM)
    {
        return NULL;
    }

    aux[dim] = '\0';
    return aux;
}

static TListMovie deleteLastTop(TListMovie listM)
{
    //Caso base
    if (listM == NULL)
        return listM;
    //Si es la ultima posicion, se borra la pelicula de la lista
    if (listM->next == NULL)
    {
        freeTopRec(listM);
        return NULL;
    }
    listM->next = deleteLastTop(listM->next);
    return listM;
}

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
        *lastM = newMovie; //Actualizamos la ultima pelicula agregada al top 5
        (*flag) = 1;       //Seteamos el flag, pues se ha logrado agregar al top 5
        return newMovie;
    }
    //Sigo buscando en la sublista
    listM->next = updateTopRec(listM->next, lastM, title, genres, rating, votes, top - 1, flag);
    return listM;
}

static TListYear addToYearRec(TListYear listY, titleTypeY type, char *title, size_t year, char *genres, float rating, size_t votes, int *added)
{
    int c, flag = 0; //El flag indica si se agrega la pelicula al top 5. De esa manera podemos aumentar el size
    if (listY == NULL || (c = compare(listY->year, year)) < 0)
    {
        TListYear newYear = calloc(1, sizeof(TYear));

        //Si no pudimos reservar memoria en el heap, devuelvo la lista
        if (newYear == NULL || errno == ENOMEM)
        {
            return listY;
        }

        newYear->year = year;

        //En caso de ser una pelicula, debemos actualizar (o no) el top 5 de las mas votadas
        if (type == MOVIE_Y)
        {
            newYear->firstInTop = updateTopRec(newYear->firstInTop, &newYear->lastAddedInTop, title, genres, rating, votes, TOP, &flag);
            newYear->sizeTop = flag; //firstInTop va a estar vacio, por lo que igualamos el size a flag en vez de sumarlo
        }
        (*added) = 1; //Seteamos el flag added en 1, pues se ha agregado un anio
        newYear->types[type] = 1;
        newYear->next = listY;
        return newYear;
    }
    if (c == 0)
    {
        if (type == MOVIE_Y)
        {
            listY->firstInTop = updateTopRec(listY->firstInTop, &listY->lastAddedInTop, title, genres, rating, votes, TOP, &flag);
            listY->sizeTop += flag;

            //Si la cantidad del top 5 supera el maximo (TOP), al haber desplazado las peliculas en una posicion, debo eliminar la ultima del top
            if (listY->sizeTop > TOP)
            {
                listY->lastAddedInTop = deleteLastTop(listY->lastAddedInTop);
                listY->sizeTop--;
            }
        }
        listY->types[type]++; //Aumento el type que se agrego
        return listY;
    }
    listY->next = addToYearRec(listY->next, type, title, year, genres, rating, votes, added);
    return listY;
}

int addToYear(imdbADT imdb, titleTypeY type, char *title, size_t year, char *genres, float rating, size_t votes)
{
    int added = 0; //Indica si se ha agregado un nuevo anio
    imdb->firstYear = addToYearRec(imdb->firstYear, type, title, year, genres, rating, votes, &added);
    return added;
}

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
        if (newGenre->genre == NULL || errno == ENOMEM)
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
    return 1;
}

void toBeginGenre(imdbADT imdb, size_t year)
{
    TListYear listY = searchYear(imdb->firstYear, year);
    if (listY == NULL)
    {
        imdb->iterGenre = NULL;
    }
    else
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
    return 1;
}

void toBeginMovieTop(imdbADT imdb, size_t year)
{
    TListYear listY = searchYear(imdb->firstYear, year);
    if (listY == NULL)
    {
        return;
    }
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
    return 1;
}

int getYear(imdbADT imdb)
{
    if (imdb == NULL || imdb->iterYear == NULL)
        return ERRORB;
    return imdb->iterYear->year;
}

int getTypeCant(imdbADT imdb, titleTypeY type)
{
    if (imdb == NULL || imdb->iterYear == NULL)
        return ERRORB;
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
        return ERRORB;
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
        return ERRORB;
    return imdb->iterMovie->votes;
}

float getRating(imdbADT imdb)
{
    if (imdb == NULL || imdb->iterMovie == NULL)
        return ERRORB;
    return imdb->iterMovie->rating;
}

char *getTopGenres(imdbADT imdb)
{
    if (imdb == NULL || imdb->iterMovie == NULL)
        return NULL;
    char *out = copy(imdb->iterMovie->genres);
    return out;
}

static void freeGenreRec(TListGenre listG)
{
    if (listG == NULL)
    {
        return;
    }
    freeGenreRec(listG->next);//Liberamos la sublista de generos
    free(listG->genre); //Liberamos el string del genero de la lista
    free(listG);
}

static void freeTopRec(TListMovie listM)
{
    if (listM == NULL)
    {
        return;
    }
    freeTopRec(listM->next); //Liberamos la sublista de top 5 peliculas
    free(listM->name);//Liberamos el string del titulo de la pelicula
    free(listM->genres); //Liberamos el string de los generos de la pelicula
    free(listM);
}

static void freeYearRec(TListYear listY)
{
    if (listY == NULL)
    {
        return;
    }
    freeTopRec(listY->firstInTop); //Liberamos la lista de top 5 peliculas
    freeGenreRec(listY->firstGenre);//Liberamos la lista de generos
    freeYearRec(listY->next);//Liberamos la sublista de anios
    free(listY);
}

void freeImdb(imdbADT imdb)
{
    freeYearRec(imdb->firstYear);//Liberamos la lista de anios
    free(imdb);
}
