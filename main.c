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

//Las tres funciones toQueryi (i = 1,2,3), se encargan de realizar cada una la consulta (query) correspondiente
//En caso de producirse un error, las mismas devuelven ERRORB, indicando que hubo un problema al acceder a datos del back
int toQuery1(imdbADT imdb, FILE *query, size_t year);

int toQuery2(imdbADT imdb, FILE *query, size_t year);

int toQuery3(imdbADT imdb, FILE *query, size_t year);

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

//argc representa la cantidad de argumentos y argv los parametros recibidos por linea de comando
int main(int argc, char *argv[])
{
    //La cantidad de argumentos debe ser 3 (el ejecutable, base de datos de peliculas,
    //base de datos de generos), caso contrario se debe imprimir un mensaje indicando el error.
    if (argc != ARGMTS)
    {
        errorOut("Cantidad de argumentos invalida", EINVAL);
    }
    //Seteamos errno a 0
    errno = 0;
    //Creamos los FILE * que vamos a usar
    //filmsF es imdb.csv y genresF es genres.csv. Solo tienen permisos de lectura
    //query1, query2 y query3 van a ser las salidas de cada query. Tienen permisos de escritura y actualizacion
    FILE *filmsF = fopen(argv[1], "r");
    FILE *genresF = fopen(argv[2], "r");
    FILE *query1 = fopen("query1.csv", "w+");
    FILE *query2 = fopen("query2.csv", "w+");
    FILE *query3 = fopen("query3.csv", "w+");
    
    FILE *files[] = {filmsF, genresF, query1, query2, query3}; //guarda todos los archivos para ser mas eficiente al momento de cerrarlos
    size_t fileCount = QUERIES + argc - 1; //cantidad de archivos que hay en *files[]

    //Primero debemos chequear que se han podido abrir correctamente los archivos
    checkFiles(files, fileCount);

    //Se crea el imdbADT
    imdbADT imdb = newImdb();

    /*
    ** allGens actua como el buffer de genres, copiando en cada posicion un genero distinto y buff como el buffer de imdb
    ** title, type, genres, year, votes y rating son variables que guardan datos de imdb para pasarle luego a los structs
    ** token es una variable que va a ser utilizada con strtok para poder separar los datos dentro de cada linea
    ** limit es la dimension de allGens y gen va a recibir un genero por vez y pasarselo a allGens
    */
    char allGens[MAXGEN][GEN_SIZE], gen[GEN_SIZE], buff[MAXBUFF], *title, *type, *genres, *token;
    size_t year, votes, limit = 0;
    float rating;

    //Chequear si el archivo genres es vacio, en caso de serlo, aborta el programa
    if (fgets(gen, GEN_SIZE, genresF) == NULL)
    {
        closeAll(imdb, files, fileCount, "El archivo genres se encuentra vacio", EINVAL);
    }

    //Se guarda primero cada genero en gen con fgets y luego se lo copia a allGens
    while (fgets(gen, GEN_SIZE, genresF) != NULL && limit != MAXGEN)
    {
        strcpy(allGens[limit++], gen);
    }

    //Chequear si el archivo imdb es vacio, en caso de serlo, aborta el programa
    if (fgets(buff, MAXBUFF, filmsF) == NULL)
    {
        closeAll(imdb, files, fileCount, "El archivo imdb se encuentra vacio", EINVAL);
    }

    //Carga los datos de imdb y genres al TAD
    //Cada vez que entra al ciclo va a encargarse de una pelicula por vez, es decir, una linea por vez
    while (fgets(buff, MAXBUFF, filmsF) != NULL)
    {
        int invalidYear = 0;         //Si el titleType no tiene anio de comienzo, se ignora
        int invalidVotes = 0;        //Si el titleType no tiene votos, se ignora
        token = strtok(buff, DELIM); //Carga en buff cada dato por separado
        size_t place;                //Representa el lugar/campo en el que me encuentro en la linea (id,type,etc)
        //Cada vez que entra obtiene un nuevo dato de la linea. Usa place para fijarse cual es el dato que se esta obteniendo
        for (place = 0; place < CANTDATA && token != NULL && !invalidYear && !invalidVotes; place++, token = strtok(NULL, DELIM))
        {
            //Carga en cada variable el respectivo dato usando un switch de place y el enum de CANTDATA
            ///Los datos que no van a ser cargados al ADT (id, endyear, minutes) son descartados
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
            titleTypeG typeFlag = CANT_TYPES_G; //Indicador de pelicula/serie para generos (ahorramos volver a hacer las condiciones de los if's del anio)
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

            char *genToBack = strtok(genres, DELIMGENRE); //Carga en genToBack cada genero de manera individual
            while (genToBack != NULL)
            {
                //Debemos verificar que el genero se encuentre dentro de los primeros MAXGEN generos
                //En caso de que no haya genero (string \N), se ignora (unicamente en addToGenre)
                if (checkGenre(genToBack, allGens, limit, strlen(genToBack)) && typeFlag < CANT_TYPES_G)
                {
                    addToGenre(imdb, typeFlag, genToBack, year);
                }
                genToBack = strtok(NULL, DELIMGENRE); //Pasamos al siguiente genero
            }
        }
        //Si no hay memoria en el heap, aborta
        if (errno == ENOMEM)
        {
            closeAll(imdb, files, fileCount, "No hay memoria disponible en el heap", ENOMEM);
        }
    }

    //Colocamos los titulos/headers en cada archivo (segun la query)
    fprintf(query1, "%s\n", FIRSTLINE1);
    fprintf(query2, "%s\n", FIRSTLINE2);
    fprintf(query3, "%s\n", FIRSTLINE3);

    //Inicializamos el iterador de anios
    toBeginYear(imdb);
    size_t currentYear;
    while (hasNextYear(imdb))
    {
        //Obtenemos el anio
        currentYear = getYear(imdb);
        //Si al obtener el anio se produce un error, aborta
        if (currentYear == ERRORB)
        {
            closeAll(imdb, files, fileCount, "Se produjo un error al acceder a memoria del TAD", ERRORB);
        }
        //Inicializamos el iterador de generos
        toBeginGenre(imdb, currentYear);
        //Inicializamos el iterador de las 5 peliculas mas votadas
        toBeginMovieTop(imdb, currentYear);
        //Si se produce un error en toQuery1, aborta
        if ((toQuery1(imdb, query1, currentYear)) == ERRORB)
        {
            closeAll(imdb, files, fileCount, "Se produjo un error al cargar la informacion de la query 1", ERRORQ);
        }
        while (hasNextGenre(imdb))
        {
            //Si se produce un error en toQuery2, aborta
            if ((toQuery2(imdb, query2, currentYear)) == ERRORB)
            {
                closeAll(imdb, files, fileCount, "Se produjo un error al cargar la informacion de la query 2", ERRORQ);
            }
            nextGenre(imdb); //Actualizamos el iterador de generos
        }
        while (hasNextMovieTop(imdb))
        {
            //Si se produce un error en toQuery3, aborta
            if ((toQuery3(imdb, query3, currentYear)) == ERRORB)
            {
                closeAll(imdb, files, fileCount, "Se produjo un error al cargar la informacion de la query 3", ERRORQ);
            }
            nextMovieInTop(imdb); //Actualizamos el iterador de las 5 peliculas mas votadas
        }
        nextYear(imdb); //Actualizamos el iterador de anios
    }
    //Se cierran los archivos y se libera la memoria
    closeFiles(files, fileCount);
    freeImdb(imdb);
    return 0;
}

int toQuery1(imdbADT imdb, FILE *query, size_t year)
{
    int cants[CANT_TYPES_Y];
    for (int i = 0; i < CANT_TYPES_Y; i++)
    {
        //Si algun tipo de dato devuelve ERRORB significa que hubo un error
        if ((cants[i] = getTypeCant(imdb, i)) == ERRORB)
        {
            return ERRORB;
        }
    }
    //Se imprime cada linea con el anio, la cantidad de peliculas, la cantidad de series y la cantidad de cortos
    fprintf(query, "%zu%s%d%s%d%s%d\n", year, DELIM, cants[MOVIE_Y], DELIM, cants[SERIES_Y], DELIM, cants[SHORT_Y]);
    return 1;
}

int toQuery2(imdbADT imdb, FILE *query, size_t year)
{
    int cants[CANT_TYPES_G];
    for (int i = 0; i < CANT_TYPES_G; i++)
    {
        //Si algun tipo de dato devuelve ERRORB significa que hubo un error
        if ((cants[i] = getTypeInGenre(imdb, i)) == ERRORB)
            return ERRORB;
    }
    char *genre = getGenre(imdb);
    //Si genre es NULL hubo un error. Si no hubiese genre devolveria \N
    if (genre == NULL)
    {
        return ERRORB;
    }
    //Se imprime cada linea con el anio, el genero y la cantidad de peliculas y series en dicho genero, separadas por DELIM
    fprintf(query, "%zu%s%s%s%d%s%d\n", year, DELIM, genre, DELIM, cants[MOVIE_G], DELIM, cants[SERIES_G]);
    free(genre); //Se libera el string genre
    return 1;
}

int toQuery3(imdbADT imdb, FILE *query, size_t year)
{
    char *genres, *movie;
    int votes;
    float rating;
    //Si movie/genres es NULL, o votes/rating toma el valor de ERRORB, significa que hubo un error.
    //A modo de aclaracion, si la pelicula no tuviese generos, entonces genres seria el string \N
    if ((movie = getMovie(imdb)) == NULL || (genres = getTopGenres(imdb)) == NULL || (votes = getVotes(imdb)) == ERRORB || (rating = getRating(imdb)) == ERRORB)
    {
        return ERRORB;
    }
    //Se imprime cada linea con el anio, el nombre de la pelicula, la cantidad de votos, el rating y los generos a los que pertenece la pelicula
    fprintf(query, "%zu%s%s%s%d%s%.2f%s%s\n", year, DELIM, movie, DELIM, votes, DELIM, rating, DELIM, genres);
    //Se libera la memoria de los strings genres y movie
    free(genres);
    free(movie);
    return 1;
}

void errorOut(const char *message, int code)
{
    //Imprimimos el mensaje de error correspondiente y su codigo
    fprintf(stderr, "%s\nExit code: %d", message, code);
    exit(code);
}

void checkFiles(FILE *files[], size_t fileCount)
{
    for (int i = 0; i < fileCount; i++)
    {
        //Si algun archivo es vacio, se cierran todos los archicos y se aborta el programa
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
        //Se cierra el programa solo si es distinto a NULL
        if (files[i] != NULL)
            fclose(files[i]);
    }
}

void closeAll(imdbADT imdb, FILE **files, int fileCount, const char *message, int code)
{
    freeImdb(imdb);               //Liberamos todos los recursos del TAD
    closeFiles(files, fileCount); //Cerramos los archivos
    errorOut(message, code);      //Enviamos el mensaje de error junto con su codigo
}

int checkGenre(char *genre, char genList[MAXGEN][GEN_SIZE], size_t limit, int n)
{
    int c;
    for (int i = 0; i < limit; i++)
    {
        //Si el genero buscado coincide con el vector de generos, entonces se trata de un genero valido
        if ((c = strncmp(genre, genList[i], n)) == 0)
        {
            return 1;
        }
        //Como genList esta en orden alfabetico, si genre es menor que genList[i] ya no se va a encontrar genre en genList
        if (c < 0)
        {
            return 0;
        }
    }
    return 0;
}
