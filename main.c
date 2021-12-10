#include <stdio.h>
#include <errno.h>
#include "imdb/imdbADT.h"

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

void errorOut(const char *message, int code);

void checkFiles(FILE *files[], size_t fileCount);

void closeFiles(FILE *files[], size_t fileCount);

void closeAll(imdbADT imdb, FILE **files, int fileCount, const char *message, int code);

int checkGenre(char *genre, char genList[MAXGEN][GEN_SIZE], size_t limit);

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

//argc es la cantidad de argumentos y argv son los parametros
int main(int argc, char *argv[])
{
    /* 
    ** La cantidad de argumentos debe ser 3 (el ejecutable, base de datos de peliculas, 
    ** base de datos de generos), caso contrario se debe imprimir un mensaje indicando el error.
    */
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
        int invalidYear = 0; //Si el filme no tiene anio de comienzo, debemos
        token = strtok(buff, DELIM);
        size_t place; //Representa el lugar/campo en el que me encuentro en la linea (id,type,etc)
        for (place = 0; place < CANTDATA && token != NULL && !invalidYear; place++, token = strtok(NULL, DELIM))
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
                {
                    invalidYear = 1;
                }
                year = atoi(token);
                break;
            case GENRES:
                genres = token;

            case RATING:
                rating = atof(token);
                break;

            case VOTES:
                votes = atoi(token);
                break;
            }
        }
        
        //Debemos verificar que el anio sea valido (startYear != \N)
        if (!invalidYear)
        {
            titleTypeG typeFlag; //Indicador de pelicula o serie para generos
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
                if (checkGenre(genToBack, allGens, limit))
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
        toBeginGenre(imdb);
        toQuery1();
        toQuery2();
        while (hasNextGenre(imdb))
        {
            
            nextGenre(imdb);
        }
        nextYear(imdb);
    }
}

void errorOut(const char *message, int code)
{
    fprintf(stderr, "%s Exit code: %d", message, code);
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

int checkGenre(char *genre, char genList[MAXGEN][GEN_SIZE], size_t limit)
{
    for (int i = 0; i < limit; i++)
    {
        if (strcmp(genre, genList[i]) == 0)
        {
            return 1;
        }
    }
    return 0;
}