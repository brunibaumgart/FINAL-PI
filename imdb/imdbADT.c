#include "imdbADT.h"
#include <stdio.h>
#include <errno.h>

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
    size_t qtyYears;      //Cantidad de anios
    TListYear iterYear;   //Iterador para acceder a los campos correspondientes por anio
    TListGenre iterGenre; //Iterador para acceder a los campos correspondiente por genero
} imdbCDT;

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
        //vamos a agrandar de a bloques
        if (dim % BLOCK == 0)
        {
            aux = realloc(aux, (dim + BLOCK) * sizeof(char));
            //Si no pudimos reservar memoria,
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

static void deleteLastTop(TListMovie listM)
{
    if (listM == NULL || listM->next == NULL)
    {
        free(listM);
    }
    deleteLastTop(listM->next);
}

static int compareTop(size_t v1, size_t v2, char *t1, char *t2)
{
    int out = compare(v1, v2);
    if (out == 0)
    {
        out = strcmp(t1, t2);
    }
    return out;
}

static TListMovie updateTopRec(TListMovie listM, TListMovie *lastM, char *title, char *genres, float rating, size_t votes, size_t top)
{
    int c;
    //Hice TOP recorridos
    if (top == 0)
    {
        return listM;
    }
    //Inserto en la ultima posicion o en orden
    if (listM == NULL || (c = compareTop(listM->votes, votes, listM->name, title)) > 0)
    {
        TListMovie newMovie = malloc(sizeof(TMovie));
        newMovie->name = copy(title);
        newMovie->votes = votes;
        newMovie->rating = rating;
        newMovie->genres = copy(genres);
        newMovie->next = listM;
        *lastM = newMovie;
        return newMovie;
    }
    //Siempre inserto
    listM->next = updateTopRec(listM, lastM, title, genres, rating, votes, top - 1);
    return listM;
}

static TListYear addToYearRec(TListYear listY, titleTypeY type, char *title, size_t year, char *genres, float rating, size_t votes)
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
        newYear->firstInTop = updateTopRec(newYear->firstInTop, &newYear->lastAddedInTop, title, genres, rating, votes, TOP);
        newYear->sizeTop = 1;
        newYear->types[type] = 1;
        newYear->next = listY;
    }
    if (c == 0)
    {
        listY->firstInTop = updateTopRec(listY->firstInTop, &listY->lastAddedInTop, title, genres, rating, votes, TOP);
        listY->sizeTop++;
        if (listY->sizeTop > TOP)
        {
            //Si supero el maximo, al haber desplazado las peliculas hacia la derecha, debo eliminar la ultima del top
            deleteLastTop(listY->lastAddedInTop);
        }
        listY->types[type]++;
    }
    listY->next = addToYearRec(listY->next, type, title, year, genres, rating, votes);
    return listY;
}

//Agrega la pelicula/serie/corto a su anio
int addToYear(imdbADT imdb, titleTypeY type, char *title, size_t year, char *genres, float rating, size_t votes)
{
    int flag = 0; //Se prende si se crea un nuevo anio
    imdb->firstYear = addToYearRec(imdb->firstYear, type, title, year, genres, rating, votes);
    return flag;
}

//Devuelve el anio a buscar, en caso de no encontrarlo retorna NULL
static TListYear searchYear(TListYear listY, int year)
{
    int c;
    if (listY == NULL || (c = compareYear(listY->year, year)) < 0)
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

//Agrega la pelicula o serie a sus generos
//Devuelve 1 si se ha podido agregar, 0 en caso contrario
int addToGenre(imdbADT imdb, titleTypeG type, char *genre, int year)
{
    TListYear yearAux = searchYear(imdb->firstYear, year);

    if (yearAux == NULL)
    {
        //No se ha encontrado el anio al que queremos agregar el genero
        return 0;
    }
    int flag = 0; //Indica si se ha podido agregar o no al genero
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

void toBeginGenre(imdbADT imdb)
{
    imdb->iterGenre = imdb->firstYear->firstGenre;
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

size_t getTypeCant(imdbADT imdb, titleTypeY type)
{
    if (imdb == NULL || (type < 0 || type > CANT_TYPES_Y))
    {
        return 0;
    }
    return imdb->iterYear->types[type];
}

static void freeGenreRec(TListGenre listG)
{
    if (listG == NULL)
    {
        return;
    }
    freeGenreRec(listG->next);
    free(listG);
}

static void freeTopRec(TListMovie listM)
{
    if (listM == NULL)
    {
        return;
    }
    freeTopRec(listM->next);
    free(listM);
}

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
