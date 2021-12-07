#include <stdio.h>
#define TOP 5

typedef struct film{
    char * name;
    long votes;
    double rating;
    char ** genres;
}TFilm;

typedef struct genres{
    char * genre;
    size_t qtyFilms;
    size_t qtySeries;
    struct genres * next;
}TGenres;

typedef TGenres * TListGenre;

typedef struct year{
    size_t year;
    TListGenre firstGenre;
    size_t qtyFilms;
    size_t qtySeries;
    size_t qtyShorts;
    TFilm topFilms[TOP];
    struct year * next;
}TYear;

typedef TYear * TListYear;

typedef struct imdbCDT{
    TListYear first;
    size_t qtyYears;
}imdbCDT;