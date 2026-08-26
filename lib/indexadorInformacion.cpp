#include "indexadorInformacion.h"

using namespace std;

InfTermDoc::InfTermDoc() : ft(0) {}

InfTermDoc::InfTermDoc(const InfTermDoc &other) : ft(other.ft), posTerm(other.posTerm) {}

InfTermDoc::~InfTermDoc() {
    ft = 0;
    posTerm.clear();
}

InfTermDoc& InfTermDoc::operator=(const InfTermDoc &other) {
    if (this != &other) {
        ft = other.ft;
        posTerm = other.posTerm;
    }
    return *this;
}

InformacionTermino::InformacionTermino() : ftc(0) {}

InformacionTermino::InformacionTermino(const InformacionTermino &other) : ftc(other.ftc), l_docs(other.l_docs) {}

InformacionTermino::~InformacionTermino() {
    ftc = 0;
    l_docs.clear();
}

InformacionTermino& InformacionTermino::operator=(const InformacionTermino &other) {
    if (this != &other) {
        ftc = other.ftc;
        l_docs = other.l_docs;
    }
    return *this;
}

InfDoc::InfDoc() : idDoc(0), numPal(0), numPalSinParada(0), numPalDiferentes(0), tamBytes(0), fechaModificacion(0) {}

InfDoc::InfDoc(const InfDoc &other) : idDoc(other.idDoc), numPal(other.numPal), numPalSinParada(other.numPalSinParada), numPalDiferentes(other.numPalDiferentes), tamBytes(other.tamBytes), fechaModificacion(other.fechaModificacion) {}

InfDoc::~InfDoc() {
    idDoc = 0;
    numPal = 0;
    numPalSinParada = 0;
    numPalDiferentes = 0;
    tamBytes = 0;
    fechaModificacion = 0;
}

InfDoc& InfDoc::operator=(const InfDoc &other) {
    if (this != &other) {
        idDoc = other.idDoc;
        numPal = other.numPal;
        numPalSinParada = other.numPalSinParada;
        numPalDiferentes = other.numPalDiferentes;
        tamBytes = other.tamBytes;
        fechaModificacion = other.fechaModificacion;
    }
    return *this;
}

InfColeccionDocs::InfColeccionDocs() : numDocs(0), numTotalPal(0), numTotalPalSinParada(0), numTotalPalDiferentes(0), tamBytes(0) {}

InfColeccionDocs::InfColeccionDocs(const InfColeccionDocs &other) : numDocs(other.numDocs), numTotalPal(other.numTotalPal), numTotalPalSinParada(other.numTotalPalSinParada), numTotalPalDiferentes(other.numTotalPalDiferentes), tamBytes(other.tamBytes) {}

InfColeccionDocs::~InfColeccionDocs() {
    numDocs = 0;
    numTotalPal = 0;
    numTotalPalSinParada = 0;
    numTotalPalDiferentes = 0;
    tamBytes = 0;
}

InfColeccionDocs& InfColeccionDocs::operator=(const InfColeccionDocs &other) {
    if (this != &other) {
        numDocs = other.numDocs;
        numTotalPal = other.numTotalPal;
        numTotalPalSinParada = other.numTotalPalSinParada;
        numTotalPalDiferentes = other.numTotalPalDiferentes;
        tamBytes = other.tamBytes;
    }
    return *this;
}

InformacionTerminoPregunta::InformacionTerminoPregunta() : ft(0) {}

InformacionTerminoPregunta::InformacionTerminoPregunta(const InformacionTerminoPregunta &other) : ft(other.ft), posTerm(other.posTerm) {}

InformacionTerminoPregunta::~InformacionTerminoPregunta() {
    ft = 0;
    posTerm.clear();
}

InformacionTerminoPregunta& InformacionTerminoPregunta::operator=(const InformacionTerminoPregunta &other) {
    if (this != &other) {
        ft = other.ft;
        posTerm = other.posTerm;
    }
    return *this;
}

InformacionPregunta::InformacionPregunta() : numTotalPal(0), numTotalPalSinParada(0), numTotalPalDiferentes(0) {}

InformacionPregunta::InformacionPregunta(const InformacionPregunta &other) : numTotalPal(other.numTotalPal), numTotalPalSinParada(other.numTotalPalSinParada), numTotalPalDiferentes(other.numTotalPalDiferentes) {}

InformacionPregunta::~InformacionPregunta() {
    numTotalPal = 0;
    numTotalPalSinParada = 0;
    numTotalPalDiferentes = 0;
}

InformacionPregunta& InformacionPregunta::operator=(const InformacionPregunta &other) {
    if (this != &other) {
        numTotalPal = other.numTotalPal;
        numTotalPalSinParada = other.numTotalPalSinParada;
        numTotalPalDiferentes = other.numTotalPalDiferentes;
    }
    return *this;
}