#ifndef INDEXADORHASH_H
#define INDEXADORHASH_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <algorithm>
#include "indexadorInformacion.h"
#include "tokenizador.h"

using namespace std;

class IndexadorHash {

    friend ostream& operator<<(ostream& s, const IndexadorHash& p) {
        s << "Fichero con el listado de palabras de parada: " << p. ficheroStopWords << endl;
        s << "Tokenizador: " << p.tok << endl;
        s << "Directorio donde se almacenara el indice generado: " << p.directorioIndice << endl;
        s << "Stemmer utilizado: " << p.tipoStemmer << endl;
        s << "Informacion de la coleccion indexada: " << p.informacionColeccionDocs << endl;
        s << "Se almacenaran las posiciones de los terminos: " << p.almacenarPosTerm;

        return s;
    }

public:

    IndexadorHash(const string& fichStopWords, const string& delimitadores, const bool& detectComp, const bool& minuscSinAcentos, const string& dirIndice, const int& tStemmer, const bool& almPosTerm);
    // "fichStopWords" ser� el nombre del archivo que contendr� todas las palabras de parada (una palabra por cada l�nea del fichero) y se almacenar� en el campo privado "ficheroStopWords". Asimismo, almacenar� todas las palabras de parada que contenga el archivo en el campo privado "stopWords", el �ndice de palabras de parada. 
    // "delimitadores" ser� el string que contiene todos los delimitadores utilizados por el tokenizador (campo privado "tok")
    // detectComp y minuscSinAcentos ser�n los par�metros que se pasar�n al tokenizador
    // "dirIndice" ser� el directorio del disco duro donde se almacenar� el �ndice (campo privado "directorioIndice"). Si dirIndice="" entonces se almacenar� en el directorio donde se ejecute el programa
    // "tStemmer" inicializar� la variable privada "tipoStemmer": 
    // 0 = no se aplica stemmer: se indexa el t�rmino tal y como aparece tokenizado
    // 1 = stemmer de Porter para espa�ol
    // 2 = stemmer de Porter para ingl�s
    // "almPosTerm" inicializar� la variable privada "almacenarPosTerm"
    // Los �ndices (p.ej. �ndice, indiceDocs e informacionColeccionDocs) quedar�n vac�os

    IndexadorHash(const string& directorioIndexacion);
    // Constructor para inicializar IndexadorHash a partir de una indexaci�n previamente realizada que habr� sido almacenada en "directorioIndexacion" mediante el m�todo "bool GuardarIndexacion()". Con ello toda la parte privada se inicializar� convenientemente, igual que si se acabase de indexar la colecci�n de documentos. En caso que no exista el directorio o que no contenga los datos de la indexaci�n se tratar� la excepci�n correspondiente

    IndexadorHash(const IndexadorHash&);

    ~IndexadorHash();

    IndexadorHash& operator= (const IndexadorHash&);

    bool Indexar(const string& ficheroDocumentos);
    // Devuelve true si consigue crear el �ndice para la colecci�n de documentos detallada en ficheroDocumentos, el cual contendr� un nombre de documento por l�nea. Los a�adir� a los ya existentes anteriormente en el �ndice.
    // Devuelve falso si no finaliza la indexaci�n (p.ej. por falta de memoria), mostrando el mensaje de error correspondiente, indicando el documento y t�rmino en el que se ha quedado, dejando en memoria lo que se haya indexado hasta ese momento.
    // En el caso que aparezcan documentos repetidos, documentos que no existen o que ya estuviesen previamente indexados (ha de coincidir el nombre del documento y el directorio en que se encuentre), se devolver� true, mostrando el mensaje de excepci�n correspondiente, y se re-indexar�n (borrar el documento previamente indexado e indexar el nuevo) en caso que la fecha de modificaci�n del documento sea m�s reciente que la almacenada previamente (class "InfDoc" campo "fechaModificacion"). Los casos de reindexaci�n mantendr�n el mismo idDoc.


    bool IndexarDirectorio(const string& dirAIndexar);
    // Devuelve true si consigue crear el �ndice para la colecci�n de documentos que se encuentra en el directorio (y subdirectorios que contenga) dirAIndexar (independientemente de la extensi�n de los mismos). Se considerar� que todos los documentos del directorio ser�n ficheros de texto. Los a�adir� a los ya existentes anteriormente en el �ndice.
    // Devuelve falso si no finaliza la indexaci�n (p.ej. por falta de memoria o porque no exista "dirAIndexar"), mostrando el mensaje de error correspondiente, indicando el documento y t�rmino en el que se ha quedado, dejando en memoria lo que se haya indexado hasta ese momento.
    // En el caso que aparezcan documentos repetidos o que ya estuviesen previamente indexados (ha de coincidir el nombre del documento y el directorio en que se encuentre), se mostrar� el mensaje de excepci�n correspondiente, y se re-indexar�n (borrar el documento previamente indexado e indexar el nuevo) en caso que la fecha de modificaci�n del documento sea m�s reciente que la almacenada previamente (class "InfDoc" campo "fechaModificacion"). Los casos de reindexaci�n mantendr�n el mismo idDoc.


    bool GuardarIndexacion() const;
    // Se guardar� en disco duro (directorio contenido en la variable privada "directorioIndice") la indexaci�n actualmente en memoria (incluidos todos los par�metros de la parte privada, incluida la indexaci�n de la pregunta). La forma de almacenamiento la determinar� el alumno. El objetivo es que esta indexaci�n se pueda recuperar posteriormente mediante el constructor "IndexadorHash(const string& directorioIndexacion)". Por ejemplo, supongamos que se ejecuta esta secuencia de comandos: "IndexadorHash a("./fichStopWords.txt", "[ ,.", "./dirIndexPrueba", 0, false); a.Indexar("./fichConDocsAIndexar.txt"); a.GuardarIndexacion();", entonces mediante el comando: "IndexadorHash b("./dirIndexPrueba");" se recuperar� la indexaci�n realizada en la secuencia anterior, carg�ndola en "b"
    // Devuelve falso si no finaliza la operaci�n (p.ej. por falta de memoria, o el nombre del directorio contenido en "directorioIndice" no es correcto), mostrando el mensaje de error correspondiente, vaciando los ficheros generados.
    // En caso que no existiese el directorio directorioIndice, habr�a que crearlo previamente

    bool RecuperarIndexacion (const string& directorioIndexacion);
    // Vac�a la indexaci�n que tuviese en ese momento e inicializa IndexadorHash a partir de una indexaci�n previamente realizada que habr� sido almacenada en "directorioIndexacion" mediante el m�todo "bool GuardarIndexacion()". Con ello toda la parte privada se inicializar� convenientemente, igual que si se acabase de indexar la colecci�n de documentos. En caso que no exista el directorio o que no contenga los datos de la indexaci�n se tratar� la excepci�n correspondiente, y se devolver� false, dejando la indexaci�n vac�a.

    void ImprimirIndexacion() const {
        cout << "Terminos indexados: " << endl;
        // A continuaci�n aparecer� un listado del contenido del campo privado "�ndice" donde para cada t�rmino indexado se imprimir�: cout << termino << '\t' << InformacionTermino << endl;
        for (auto it = indice.begin(); it != indice.end(); ++it) {
            cout << it->first << '\t' << it->second << endl;
        }
        cout << "Documentos indexados: " << endl;
        // A continuaci�n aparecer� un listado del contenido del campo privado "indiceDocs" donde para cada documento indexado se imprimir�: cout << nomDoc << '\t' << InfDoc << endl;
        for (auto it = indiceDocs.begin(); it != indiceDocs.end(); ++it) {
            cout << it->first << '\t' << it->second << endl;
        }
    }

    bool IndexarPregunta(const string& preg);
    // Devuelve true si consigue crear el �ndice para la pregunta "preg". Antes de realizar la indexaci�n vaciar� los campos privados indicePregunta e infPregunta
    // Generar� la misma informaci�n que en la indexaci�n de documentos, pero dej�ndola toda accesible en memoria principal (mediante las variables privadas "pregunta, indicePregunta, infPregunta")
    // Devuelve falso si no finaliza la operaci�n (p.ej. por falta de memoria o bien si la pregunta no contiene ning�n t�rmino con contenido), mostrando el mensaje de error correspondiente

    bool DevuelvePregunta(string& preg) const; 
    // Devuelve true si hay una pregunta indexada (con al menos un t�rmino que no sea palabra de parada, o sea, que haya alg�n t�rmino indexado en indicePregunta), devolvi�ndo "pregunta" en "preg"

    bool DevuelvePregunta(const string& word, InformacionTerminoPregunta& inf) const; 
    // Devuelve true si word (aplic�ndole el tratamiento de stemming y may�sculas correspondiente) est� indexado en la pregunta, devolviendo su informaci�n almacenada "inf". En caso que no est�, devolver�a "inf" vac�o

    bool DevuelvePregunta(InformacionPregunta& inf) const; 
    // Devuelve true si hay una pregunta indexada, devolviendo su informaci�n almacenada (campo privado "infPregunta") en "inf". En caso que no est�, devolver�a "inf" vac�o

    void ImprimirIndexacionPregunta() {
        cout << "Pregunta indexada: " << pregunta << endl;
        cout << "Terminos indexados en la pregunta: " << endl;
        // A continuaci�n aparecer� un listado del contenido de "indicePregunta" donde para cada t�rmino indexado se imprimir�: cout << termino << '\t' << InformacionTerminoPregunta << endl;
        for (auto it = indicePregunta.begin(); it != indicePregunta.end(); ++it) {
            cout << it->first << '\t' << it->second << endl;
        }
        cout << "Informacion de la pregunta: " << infPregunta << endl;
    }

    void ImprimirPregunta() {
        cout << "Pregunta indexada: " << pregunta << endl;
        cout << "Informacion de la pregunta: " << infPregunta << endl;
    }

    bool Devuelve(const string& word, InformacionTermino& inf) const; 
    // Devuelve true si word (aplic�ndole el tratamiento de stemming y may�sculas correspondiente) est� indexado, devolviendo su informaci�n almacenada "inf". En caso que no est�, devolver�a "inf" vac�o

    bool Devuelve(const string& word, const string& nomDoc, InfTermDoc& InfDoc) const;  
    // Devuelve true si word (aplic�ndole el tratamiento de stemming y may�sculas correspondiente) est� indexado y aparece en el documento de nombre nomDoc, en cuyo caso devuelve la informaci�n almacenada para word en el documento. En caso que no est�, devolver�a "InfDoc" vac�o

    bool Existe(const string& word) const;	 
    // Devuelve true si word (aplic�ndole el tratamiento de stemming y may�sculas correspondiente) aparece como t�rmino indexado


    bool BorraDoc(const string& nomDoc); 
    // Devuelve true si nomDoc est� indexado y se realiza el borrado de todos los t�rminos del documento y del documento en los campos privados "indiceDocs" e "informacionColeccionDocs"

    void VaciarIndiceDocs(); 
    // Borra todos los t�rminos del �ndice de documentos: toda la indexaci�n de documentos.

    void VaciarIndicePreg(); 
    // Borra todos los t�rminos del �ndice de la pregunta: toda la indexaci�n de la pregunta actual.



    int NumPalIndexadas() const; 
    // Devolver� el n�mero de t�rminos diferentes indexados (cardinalidad de campo privado "�ndice")

    string DevolverFichPalParada () const; 
    // Devuelve el contenido del campo privado "ficheroStopWords"

    void ListarPalParada() const; 
    // Mostrar� por pantalla las palabras de parada almacenadas (originales, sin aplicar stemming): una palabra por l�nea (salto de l�nea al final de cada palabra)

    int NumPalParada() const; 
    // Devolver� el n�mero de palabras de parada almacenadas

    string DevolverDelimitadores () const; 
    // Devuelve los delimitadores utilizados por el tokenizador

    bool DevolverCasosEspeciales () const;
    // Devuelve si el tokenizador analiza los casos especiales

    bool DevolverPasarAminuscSinAcentos () const;
    // Devuelve si el tokenizador pasa a min�sculas y sin acentos

    bool DevolverAlmacenarPosTerm () const;
    // Devuelve el valor de almacenarPosTerm

    string DevolverDirIndice () const; 
    // Devuelve "directorioIndice" (el directorio del disco duro donde se almacenar� el �ndice)

    int DevolverTipoStemming () const; 
    // Devolver� el tipo de stemming realizado en la indexaci�n de acuerdo con el valor indicado en la variable privada "tipoStemmer"

    void ListarInfColeccDocs() const; 
    // Mostrar por pantalla: cout << informacionColeccionDocs << endl;

    void ListarTerminos() const; 
    // Mostrar por pantalla el contenido el contenido del campo privado "�ndice": cout << termino << '\t' << InformacionTermino << endl;

    bool ListarTerminos(const string& nomDoc) const; 
    // Devuelve true si nomDoc existe en la colecci�n y muestra por pantalla todos los t�rminos indexados del documento con nombre "nomDoc": cout << termino << '\t' << InformacionTermino << endl; . Si no existe no se muestra nada

    void ListarDocs() const; 
    // Mostrar por pantalla el contenido el contenido del campo privado "indiceDocs": cout << nomDoc << '\t' << InfDoc << endl;

    bool ListarDocs(const string& nomDoc) const; 
    // Devuelve true si nomDoc existe en la colecci�n y muestra por pantalla el contenido del campo privado "indiceDocs" para el documento con nombre "nomDoc": cout << nomDoc << '\t' << InfDoc << endl; . Si no existe no se muestra nada

private:	
    IndexadorHash();	
    // Este constructor se pone en la parte privada porque no se permitirá crear un indexador sin inicializarlo convenientemente. La inicialización la decidirá el alumno

protected:

    unordered_map<string, InformacionTermino> indice;	 
    // Índice de términos indexados accesible por el término

    unordered_map<string, InfDoc> indiceDocs;	 
    // Índice de documentos indexados accesible por el nombre del documento


    InfColeccionDocs informacionColeccionDocs;	
    // Información recogida de la colección de documentos indexada

    string pregunta;
    // Pregunta indexada actualmente. Si no hay ninguna indexada, contendría el valor ""

    unordered_map<string, InformacionTerminoPregunta> indicePregunta;	 
    // Índice de términos indexados en una pregunta

    InformacionPregunta infPregunta;	
    // Información recogida de la pregunta indexada

private:

    unordered_set<string> stopWords;
    // Palabras de parada. El filtrado de palabras de parada se realizará, tanto en la pregunta como en los documentos, teniendo en cuenta el parámetro minuscSinAcentos y tipoStemmer. Es decir que se aplicará el mismo proceso a las palabras de parada almacenadas en el fichero antes de realizar el filtrado (p.ej. si se aplica el pasar a minúsculas los términos del documento/pregunta a indexar, para comprobar si se ha de eliminar el término, éste se comparará con la versión de palabras de parada en minúsculas). Esto se pide así para casos en los que en el documento/pregunta aparezca: "La casa de él" y estén almacenadas como stopWords "la, el", si se activa el parámetro minuscSinAcentos, entonces debería filtrar "La, él", si no hubiese estado activo ese parámetro, entonces no se hubiesen filtrado.

    string ficheroStopWords;
    // Nombre del fichero que contiene las palabras de parada

    Tokenizador tok;	
    // Tokenizador que se usará en la indexación. Se inicializará con los parámetros del constructor: detectComp y minuscSinAcentos, los cuales determinarán qué término se ha de indexar (p.ej. si se activa minuscSinAcentos, entonces se guardarán los términos en minúsculas y sin acentos)


    string directorioIndice;
    // "directorioIndice" será el directorio del disco duro donde se almacenará el índice. En caso que contenga la cadena vacía se creará en el directorio donde se ejecute el indexador

    int tipoStemmer;
    // 0 = no se aplica stemmer: se indexa el término tal y como aparece tokenizado
    // Los siguientes valores harán que los términos a indexar se les aplique el stemmer y se almacene solo dicho stem.
    // 1 = stemmer de Porter para español
    // 2 = stemmer de Porter para inglés
    // Para el stemmer de Porter se utilizarán los archivos stemmer.cpp y stemmer.h, concretamente las funciones de nombre "stemmer"

    bool almacenarPosTerm;	
    // Si es true se almacenará la posición en la que aparecen los términos dentro del documento en la clase InfTermDoc
};

#endif