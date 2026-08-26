#ifndef BUSCADOR_H
#define BUSCADOR_H

#include <iostream>
#include <queue>
#include <string>

#include "indexadorHash.h"

using namespace std;


/*
 * Clase ResultadoRI
 *
 * Contiene los trios:
 * - valor de similitud del documento
 * - identificador del documento
 * - numero de pregunta
 */
class ResultadoRI {
    friend ostream& operator<<(ostream& os, const ResultadoRI& res);

public:
    ResultadoRI(const double& kvSimilitud = 0.0,
                const long int& kidDoc = 0,
                const int& np = 0);

    double VSimilitud() const;

    long int IdDoc() const;

    int NumPregunta() const;

    bool operator< (const ResultadoRI& lhs) const;

private:
    double vSimilitud;
    long int idDoc;
    int numPregunta;
};


class Buscador: public IndexadorHash {

    friend ostream& operator<<(ostream& s, const Buscador& p) {
        string preg;

        s << "Buscador: " << endl;

        if(p.DevuelvePregunta(preg))
            s << "\tPregunta indexada: " << preg << endl;
        else
            s << "\tNo hay ninguna pregunta indexada" << endl;

        s << "\tDatos del indexador: " << endl << (IndexadorHash) p;
        // Invoca a la sobrecarga de la salida del Indexador

        return s;
    }

public:
    Buscador(const string& directorioIndexacion, const int& f);
    // Constructor para inicializar Buscador a partir de la indexacion generada previamente y almacenada en "directorioIndexacion".
    // En caso que no exista el directorio o que no contenga los datos de la indexacion se enviara a cerr la excepcion correspondiente
    // y se continuara la ejecucion del programa manteniendo los indices vacios.
    // Inicializara la variable privada "formSimilitud" a "f" y las constantes de cada modelo:
    // "c = 2; k1 = 1.2; b = 0.75;"

    Buscador(const Buscador&);

    ~Buscador();

    Buscador& operator= (const Buscador&);

    // En los metodos de "Buscar" solo se evaluaran TODOS los documentos que contengan alguno de los terminos de la pregunta
    // tras eliminar las palabras de parada.
    bool Buscar(const int& numDocumentos = 99999);
    // Devuelve true si en IndexadorHash.pregunta hay indexada una pregunta no vacia con algun termino con contenido,
    // y si sobre esa pregunta se finaliza la busqueda correctamente con la formula de similitud indicada en "formSimilitud".
    // Se guardaran los primeros "numDocumentos" documentos mas relevantes en "docsOrdenados".
    // Se almacenaran solo los documentos que compartan algun termino no de parada con la query.
    // Como numero de pregunta en "ResultadoRI.numPregunta" se almacenara el valor 0.

    bool Buscar(const string& dirPreguntas,
                const int& numDocumentos,
                const int& numPregInicio,
                const int& numPregFin);
    // Realizara la busqueda entre el numero de pregunta "numPregInicio" y "numPregFin", ambas incluidas.
    // El corpus de preguntas estara en el directorio "dirPreguntas", con cada pregunta en un fichero independiente:
    // 1.txt, 2.txt, 3.txt, ...
    // Se indexara cada pregunta por separado y se ejecutara una busqueda por cada pregunta.
    // Los resultados de cada pregunta se anyadiran en "docsOrdenados" junto con su numero de pregunta.
    // Se supone que previamente se mantiene la indexacion del corpus.
    // Se guardaran los primeros "numDocumentos" documentos mas relevantes para cada pregunta.
    // Se almacenaran solo los documentos que compartan algun termino no de parada con la query.
    // La busqueda se realiza con la formula de similitud indicada en "formSimilitud".

    void ImprimirResultadoBusqueda(const int& numDocumentos = 99999) const;
    // Imprimira por pantalla los resultados de la ultima busqueda.
    // Se imprimira un numero maximo de "numDocumentos" por cada pregunta.
    // Los resultados estaran almacenados en "docsOrdenados" en orden decreciente segun la relevancia.
    // Formato:
    // NumPregunta FormulaSimilitud NomDocumento Posicion PuntuacionDoc PreguntaIndexada
    // Donde:
    // NumPregunta seria el numero de pregunta almacenado en "ResultadoRI.numPregunta"
    // FormulaSimilitud seria: "DFR" si "formSimilitud == 0"; "BM25" si es 1.
    // NomDocumento seria el nombre del documento sin directorio ni extension.
    // Posicion empezaria desde 0.
    // PuntuacionDoc seria el valor numerico de la formula de similitud.
    // PreguntaIndexada se corresponde con IndexadorHash.pregunta si "ResultadoRI.numPregunta == 0".
    // En caso contrario se imprimira "ConjuntoDePreguntas".
    // Por ejemplo:
    // 1 BM25 EFE19950609-05926 0 64.7059 ConjuntoDePreguntas
    // 1 BM25 EFE19950614-08956 1 63.9759 ConjuntoDePreguntas
    // 1 BM25 EFE19950610-06424 2 62.6695 ConjuntoDePreguntas
    // 2 BM25 EFE19950610-00234 0 0.11656233535972 ConjuntoDePreguntas
    // 2 BM25 EFE19950610-06000 1 0.10667871616613 ConjuntoDePreguntas

    bool ImprimirResultadoBusqueda(const int& numDocumentos,
                                   const string& nombreFichero) const;
    // Lo mismo que "ImprimirResultadoBusqueda()" pero guardando la salida en el fichero "nombreFichero".
    // Devolvera false si no consigue crear correctamente el archivo.

    int DevolverFormulaSimilitud() const;
    // Devuelve el valor del campo privado "formSimilitud"

    bool CambiarFormulaSimilitud(const int& f);
    // Cambia el valor de "formSimilitud" a "f" si contiene un valor correcto: f == 0 || f == 1.
    // Devolvera false si "f" no contiene un valor correcto.

    void CambiarParametrosDFR(const double& kc);
    // Cambia el valor de "c = kc"

    double DevolverParametrosDFR() const;
    // Devuelve el valor de "c"

    void CambiarParametrosBM25(const double& kk1, const double& kb);
    // Cambia el valor de "k1 = kk1; b = kb;"

    void DevolverParametrosBM25(double& kk1, double& kb) const;
    // Devuelve el valor de "k1" y "b"

private:
    Buscador();
    // Este constructor se pone en la parte privada porque no se permitira crear un buscador sin inicializarlo convenientemente.
    // Se inicializara con todos los campos vacios, "formSimilitud" con valor 0 y las constantes:
    // "c = 2; k1 = 1.2; b = 0.75"

    priority_queue< ResultadoRI > docsOrdenados;
    // Contendra los resultados de la ultima busqueda realizada en orden decreciente segun la relevancia sobre la pregunta.
    // El tipo "priority_queue" podra modificarse por cuestiones de eficiencia.

    int formSimilitud;
    // 0: DFR, 1: BM25

    double c;
    // Constante del modelo DFR

    double k1;
    // Constante del modelo BM25

    double b;
    // Constante del modelo BM25
};

#endif