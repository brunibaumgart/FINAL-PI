#include <stdio.h>
#include <errno.h>
#include "imbd\imdbADT.h"

#define ARGMTS 3       //Cantidad de argumentos a leer
#define QUERIES 3      //Cantidad de queries
#define MAXGEN 32      //Cantidad maxima de generos
#define GEN_SIZE 64    //Cantidad maxima de caracteres de un genero (arbitrario)
#define MAXBUFF 512    //Cantidad maxima de caracteres de una linea (arbitrario)
#define DELIM ";"      //Delimitador de lectura de base de datos
#define DELIMGENRE "," //Delimitador de lectura de generos
#define NONE "\\N"      //Indicador sin datos

void errorOut(const char *message, int code);

void checkFiles(FILE *files[], size_t fileCount);

void closeFiles(FILE *files[], size_t fileCount);

void closeAll(imdbADT imdb, FILE **files, int fileCount, const char *message, int code);

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
    /*
    ** Primero debemos chequear que se han podido abrir correctamente los archivos
    */
    checkFiles(files, fileCount);

    imdbADT imdb = newImdb();

    char *allGens[MAXGEN], gen[GEN_SIZE], buff[MAXBUFF], *title, *type, *genres, *token;
    int year,rating, votes, limit = 0;

    if (fgets(gen, GEN_SIZE, genresF) == NULL)
    {
        closeAll(imdb, files, fileCount, "El archivo se encuentra vacio", EINVAL);
    }

    while (fgets(gen, GEN_SIZE, genresF) != NULL && limit != MAXGEN)
    {
        allGens[limit++] = gen; //CHECKEAR (bruno)
    }

    while (fgets(buff, MAXBUFF, filmsF) != NULL)
    {
        int invalidYear = 0;
        token = strtok(buff, DELIM);
        for (size_t place = 0; place < CANTDATA && token != NULL && !invalidYear; place++, token = (NULL, DELIM))
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

            case RATING: rating = atof(token);break;

            case VOTES:  votes = atoi(token); break;
            }
        }
        if(!invalidYear){
            
        }
    }
}

void errorOut(const char *message, int code)
{
    fprintf(stderr, "%s Exit code: %d", message, code);
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