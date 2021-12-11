#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "imdbADT/imdbADT.h"

#define FIRSTLINE1 "year;films;series;shorts"
#define FIRSTLINE2 "year;genre;films;series"
#define FIRSTLINE3 "year;film;votes;rating;genres"

#define ARGMTS 3       //Cantidad de argumentos a leer
#define QUERIES 3      //Cantidad de queries
#define MAXGEN 32      //Cantidad maxima de generos
#define GEN_SIZE 32    //Cantidad maxima de caracteres de un genero (arbitrario)
#define MAXBUFF 512    //Cantidad maxima de caracteres de una linea (arbitrario)
#define DELIM ";"      //Delimitador de lectura de base de datos
#define DELIMGENRE "," //Delimitador de lectura de generos
#define NONE "\\N"     //Indicador sin datos

//Envia un mensaje indicando el error que se produjo, y aborta el programa
void errorOut(const char *message, int code);

//Verifica si se han logrado abrir correctamente los archivos recibidos, si se produjo un error cierra los mismos y envia el mensaje correspondiente
void checkFiles(FILE *files[], size_t fileCount);

//Cierra los archivos recibidos
void closeFiles(FILE *files[], size_t fileCount);

//Libera la memoria reservada para el TAD, cierra los archivos e indica el error que se produjo
void closeAll(imdbADT imdb, FILE **files, int fileCount, const char *message, int code);

//Verifica que el genero trabajado sea valido (se encuentre entre los primeros 32 obtenidos)
int checkGenre(char *genre, char genList[MAXGEN][GEN_SIZE], size_t limit, int n);

void toQuery1(imdbADT imdb, FILE *query, size_t year);

void toQuery2(imdbADT imdb, FILE *query, size_t year);

void toQuery3(imdbADT imdb, FILE *query, size_t year);

typedef enum DATA
{
    ID = 0,
    TYPE,
    TITLE,
    STYEAR,
    ENDYEAR,
    GENRES,
    RATING,
    VOTES,
    MINUTES,
    CANTDATA
} DATA;

//argc representa la cantidad de argumentos y argv los parametros
int main(int argc, char *argv[])
{
    //La cantidad de argumentos debe ser 3 (el ejecutable, base de datos de peliculas,
    //base de datos de generos), caso contrario se debe imprimir un mensaje indicando el error.
    if (argc != ARGMTS)
    {
        errorOut("Cantidad de argumentos invalida", EINVAL);
    }

    errno = 0;
    FILE *filmsF = fopen(argv[1], "r");
    FILE *genresF = fopen(argv[2], "r");
    FILE *query1 = fopen("query1.csv", "w+");
    FILE *query2 = fopen("query2.csv", "w+");
    FILE *query3 = fopen("query3.csv", "w+");
    FILE *files[] = {filmsF, genresF, query1, query2, query3};
    size_t fileCount = QUERIES + argc - 1;

    //Primero debemos chequear que se han podido abrir correctamente los archivos
    checkFiles(files, fileCount);

    imdbADT imdb = newImdb();

    char allGens[MAXGEN][GEN_SIZE], gen[GEN_SIZE], buff[MAXBUFF], *title, *type, *genres, *token;
    size_t year, votes, limit = 0;
    float rating;

    if (fgets(gen, GEN_SIZE, genresF) == NULL)
    {
        closeAll(imdb, files, fileCount, "El archivo genres se encuentra vacio", EINVAL);
    }

    while (fgets(gen, GEN_SIZE, genresF) != NULL && limit != MAXGEN)
    {
        strcpy(allGens[limit++], gen);
    }

    if (fgets(buff, MAXBUFF, filmsF) == NULL)
    {
        closeAll(imdb, files, fileCount, "El archivo imdb se encuentra vacio", EINVAL);
    }

    while (fgets(buff, MAXBUFF, filmsF) != NULL)
    {
        int invalidYear = 0;  //Si el filme no tiene anio de comienzo, se ignora
        int invalidVotes = 0; //Si el filme no tiene votos, se ignora
        token = strtok(buff, DELIM);
        size_t place; //Representa el lugar/campo en el que me encuentro en la linea (id,type,etc)
        for (place = 0; place < CANTDATA && token != NULL && !invalidYear && !invalidVotes; place++, token = strtok(NULL, DELIM))
        {
            switch (place)
            {
            case TYPE:
                type = token;
                break;
            case TITLE:
                title = token;
                break;
            case STYEAR:
                if (strcmp(token, NONE) == 0)
                    invalidYear = 1;
                year = atoi(token);
                break;
            case GENRES:
                genres = token;

            case RATING:
                rating = atof(token);
                break;

            case VOTES:
                votes = atoi(token);
                if (votes == 0)
                    invalidVotes = 1;
                break;
            }
        }

        //Debemos verificar que el anio y los votos sean valido (startYear != \N && votes != 0)
        if (!invalidYear && !invalidVotes)
        {
            titleTypeG typeFlag = CANT_TYPES_G; //Indicador de pelicula o serie para generos (ahorramos volver a hacer las condiciones de los if's del anio)
            //Debemos inicializar el flag con el fin de que, en caso de no ser una pelicula o una serie, el mismo no tome valores arbitrarios
            if (strcmp(type, TSHORT) == 0)
            {
                addToYear(imdb, SHORT_Y, title, year, genres, rating, votes);
            }

            if (strcmp(type, TMOVIE) == 0)
            {
                addToYear(imdb, MOVIE_Y, title, year, genres, rating, votes);
                typeFlag = MOVIE_G;
            }

            if (strcmp(type, TSERIES) == 0 || strcmp(type, TMSERIES) == 0)
            {
                addToYear(imdb, SERIES_Y, title, year, genres, rating, votes);
                typeFlag = SERIES_G;
            }

            char *genToBack = strtok(genres, DELIMGENRE);
            while (genToBack != NULL)
            {
                //Debemos verificar que el genero se encuentre dentro de los primeros MAXGEN generos
                //En caso de que no haya genero (\N), se ignora unicamente para addToGenre
                if (checkGenre(genToBack, allGens, limit, strlen(genToBack)) && typeFlag < CANT_TYPES_G)
                {
                    addToGenre(imdb, typeFlag, genToBack, year);
                }
                genToBack = strtok(NULL, DELIMGENRE);
            }
        }
        if (errno == ENOMEM)
        {
            closeAll(imdb, files, fileCount, "No hay memoria disponible en el heap", ENOMEM);
        }
    }

    fprintf(query1, "%s\n", FIRSTLINE1);
    fprintf(query2, "%s\n", FIRSTLINE2);
    fprintf(query3, "%s\n", FIRSTLINE3);

    toBeginYear(imdb);
    while (hasNextYear(imdb))
    {
        year = getYear(imdb);
        toBeginGenre(imdb, year);
        toBeginMovieTop(imdb, year);
        toQuery1(imdb, query1, year);
        while (hasNextGenre(imdb))
        {
            toQuery2(imdb, query2, year);
            nextGenre(imdb);
        }
        while (hasNextMovieTop(imdb))
        {
            toQuery3(imdb, query3, year);
            nextMovieInTop(imdb);
        }
        nextYear(imdb);
    }
    freeImdb(imdb);
}

void toQuery1(imdbADT imdb, FILE *query, size_t year)
{
    int cants[CANT_TYPES_Y];
    for (int i = 0; i < CANT_TYPES_Y; i++)
    {
        cants[i] = getTypeCant(imdb, i);
    }
    fprintf(query, "%zu%s%d%s%d%s%d\n", year, DELIM, cants[MOVIE_Y], DELIM, cants[SERIES_Y], DELIM, cants[SHORT_Y]);
}

void toQuery2(imdbADT imdb, FILE *query, size_t year)
{
    int cants[CANT_TYPES_G];
    for (int i = 0; i < CANT_TYPES_G; i++)
    {
        cants[i] = getTypeInGenre(imdb, i);
    }
    char *genre = getGenre(imdb);
    fprintf(query, "%zu%s%s%s%d%s%d\n", year, DELIM, genre, DELIM, cants[MOVIE_G], DELIM, cants[SERIES_G]);
    free(genre);
}

void toQuery3(imdbADT imdb, FILE *query, size_t year)
{
    char *genres = getTopGenres(imdb);
    char *movie = getMovie(imdb);
    //chequear si los titulos/generos son null (lo mismo con votos,rating)
    fprintf(query, "%zu%s%s%s%d%s%.2f%s%s\n", year, DELIM, movie, DELIM, getVotes(imdb), DELIM, getRating(imdb), DELIM, genres);
    free(genres);
    free(movie);
}

void errorOut(const char *message, int code)
{
    fprintf(stderr, "%s\nExit code: %d", message, code);
    exit(code);
}

void checkFiles(FILE *files[], size_t fileCount)
{
    for (int i = 0; i < fileCount; i++)
    {
        if (files[i] == NULL)
        {
            closeFiles(files, fileCount);
            errorOut("Hubo un error al abrir un archivo", errno);
        }
    }
}

void closeFiles(FILE *files[], size_t fileCount)
{
    for (int i = 0; i < fileCount; i++)
    {
        if (files[i] != NULL)
            fclose(files[i]);
    }
}

void closeAll(imdbADT imdb, FILE **files, int fileCount, const char *message, int code)
{
    freeImdb(imdb);
    closeFiles(files, fileCount);
    errorOut(message, code);
}

int checkGenre(char *genre, char genList[MAXGEN][GEN_SIZE], size_t limit, int n)
{
    int c;
    for (int i = 0; i < limit; i++)
    {
        if ((c = strncmp(genre, genList[i], n)) == 0)
        {
            return 1;
        }
        if (c < 0)
            return 0;
    }
    return 0;
}
