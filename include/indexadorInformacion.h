#ifndef INDEXADORINFORMACION_H
#define INDEXADORINFORMACION_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>      
#include <algorithm>   
#include <ctime>

using namespace std;

typedef time_t Fecha;

class InfTermDoc {
    friend class IndexadorHash;
    friend class Buscador;
    friend ostream& operator<<(ostream& s, const InfTermDoc& p);
public:
    InfTermDoc(const InfTermDoc &);
    InfTermDoc();
    ~InfTermDoc();
    InfTermDoc & operator=(const InfTermDoc &);
private:
    int ft;
    vector<int> posTerm;   
    friend class IndexadorHash;
};

class InformacionTermino {
    friend class IndexadorHash;
    friend class Buscador;
    friend ostream& operator<<(ostream& s, const InformacionTermino& p);
public:
    InformacionTermino(const InformacionTermino &);
    InformacionTermino();
    ~InformacionTermino();
    InformacionTermino & operator=(const InformacionTermino &);
private:
    int ftc;
    
    vector<pair<int, InfTermDoc>> l_docs;
    friend class IndexadorHash;
};



inline ostream& operator<<(ostream& s, const InformacionTermino& p) {
    s << "Frecuencia total: " << p.ftc << "\tfd: " << p.l_docs.size();
    // A continuaci�n se mostrar�an todos los elementos de p.l_docs: s << "\tId.Doc: " << idDoc << "\t" << InfTermDoc;
    for (const auto& par : p.l_docs) {
        s << "\tId.Doc: " << par.first << "\t" << par.second;
    }
    return s;
}

inline ostream& operator<<(ostream& s, const InfTermDoc& p) {
    s << "ft: " << p.ft;
    // A continuaci�n se mostrar�an todos los elementos de p.posTerm ("posicion TAB posicion TAB ... posicion, es decir nunca finalizar� en un TAB"): s << "\t" << posicion;
    for (const auto& posicion : p.posTerm) {
        s << "\t" << posicion;
    }
    return s;
}

class InfDoc { 
    friend class IndexadorHash;
    friend class Buscador;
    friend ostream& operator<<(ostream& s, const InfDoc& p);
public:
    InfDoc (const InfDoc &);
    InfDoc ();	
    ~InfDoc ();
    InfDoc & operator= (const InfDoc &);

    // A�adir cuantos m�todos se consideren necesarios para manejar la parte privada de la clase
private:
    int idDoc;	
    // Identificador del documento. El primer documento indexado en la colecci�n ser� el identificador 1
    int numPal;	// N� total de palabras del documento
    int numPalSinParada;	// N� total de palabras sin stop-words del documento
    int numPalDiferentes;	
    // N� total de palabras diferentes que no sean stop-words (sin acumular la frecuencia de cada una de ellas)
    int tamBytes;	// Tama�o en bytes del documento
    Fecha fechaModificacion;
    // Atributo correspondiente a la fecha y hora (completa) de modificaci�n del documento. El tipo "Fecha/hora" lo elegir�/implementar� el alumno
    
    friend class IndexadorHash;
};

inline ostream& operator<<(ostream& s, const InfDoc& p) {
    s << "idDoc: " << p.idDoc << "\tnumPal: " << p.numPal << "\tnumPalSinParada: " << p.numPalSinParada << "\tnumPalDiferentes: " << p.numPalDiferentes << "\ttamBytes: " << p.tamBytes;
    return s;
}

class InfColeccionDocs { 
    friend class IndexadorHash;
    friend class Buscador;
    friend ostream& operator<<(ostream& s, const InfColeccionDocs& p);
public:
    InfColeccionDocs (const InfColeccionDocs &);
    InfColeccionDocs ();
    ~InfColeccionDocs ();
    InfColeccionDocs & operator= (const InfColeccionDocs &);

    // A�adir cuantos m�todos se consideren necesarios para manejar la parte privada de la clase
private:
    int numDocs;	// N� total de documentos en la colecci�n
    int numTotalPal;	
    // N� total de palabras en la colecci�n 
    int numTotalPalSinParada;
    // N� total de palabras sin stop-words en la colecci�n 
    int numTotalPalDiferentes;	
    // N� total de palabras diferentes en la colecci�n que no sean stop-words (sin acumular la frecuencia de cada una de ellas)
    int tamBytes;	// Tama�o total en bytes de la colecci�n
    
    friend class IndexadorHash;
};

inline ostream& operator<<(ostream& s, const InfColeccionDocs& p) {
    s << "numDocs: " << p.numDocs << "\tnumTotalPal: " << p.numTotalPal << "\tnumTotalPalSinParada: " << p.numTotalPalSinParada << "\tnumTotalPalDiferentes: " << p.numTotalPalDiferentes << "\ttamBytes: " << p.tamBytes;
    return s;
}

class InformacionTerminoPregunta { 
    friend class IndexadorHash;
    friend class Buscador;
    friend ostream& operator<<(ostream& s, const InformacionTerminoPregunta& p);
public:
    InformacionTerminoPregunta (const InformacionTerminoPregunta &);
    InformacionTerminoPregunta ();
    ~InformacionTerminoPregunta ();
    InformacionTerminoPregunta & operator= (const InformacionTerminoPregunta &);

    // A�adir cuantos m�todos se consideren necesarios para manejar la parte privada de la clase
private:
    int ft;
    vector<int> posTerm;
    friend class IndexadorHash;
};

inline ostream& operator<<(ostream& s, const InformacionTerminoPregunta& p) {
    s << "ft: " << p.ft;
    // A continuaci�n se mostrar�an todos los elementos de p.posTerm ("posicion TAB posicion TAB ... posicion, es decir nunca finalizar� en un TAB"): s << "\t" << posicion;
    for (const auto& posicion : p.posTerm) {
        s << "\t" << posicion;
    }
    return s;
}

class InformacionPregunta { 
    friend class IndexadorHash;
    friend class Buscador;
    friend ostream& operator<<(ostream& s, const InformacionPregunta& p);
public:
    InformacionPregunta (const InformacionPregunta &);
    InformacionPregunta ();	
    ~InformacionPregunta ();
    InformacionPregunta & operator= (const InformacionPregunta &);

    // A�adir cuantos m�todos se consideren necesarios para manejar la parte privada de la clase
private:
    int numTotalPal;	
    // N� total de palabras en la pregunta
    int numTotalPalSinParada;
    // N� total de palabras sin stop-words en la pregunta
    int numTotalPalDiferentes;	
    // N� total de palabras diferentes en la pregunta que no sean stop-words (sin acumular la frecuencia de cada una de ellas)
    
    friend class IndexadorHash;
};

inline ostream& operator<<(ostream& s, const InformacionPregunta& p) {
    s << "numTotalPal: " << p.numTotalPal << "\tnumTotalPalSinParada: "<< p.numTotalPalSinParada << "\tnumTotalPalDiferentes: " << p.numTotalPalDiferentes;
    return s;
}

#endif