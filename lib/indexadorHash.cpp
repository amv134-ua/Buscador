#include "indexadorHash.h"
#include "stemmer.h"
#include <fstream>
#include <sys/stat.h>
#include <dirent.h>

using namespace std;

IndexadorHash::IndexadorHash(const string& fichStopWords, const string& delimitadores, const bool& detectComp, const bool& minuscSinAcentos, const string& dirIndice, const int& tStemmer, const bool& almPosTerm)
    : ficheroStopWords(fichStopWords), tok(delimitadores, detectComp, minuscSinAcentos), directorioIndice(dirIndice), tipoStemmer(tStemmer), almacenarPosTerm(almPosTerm)
{

    ifstream ifs(fichStopWords);
    if(ifs) {
        string palabra;
        stemmerPorter stemmer;
        list<string> tokens;

        while(getline(ifs, palabra)) {
                    if(palabra.empty()) continue;

                    tok.Tokenizar(palabra, tokens);

                    for(auto it = tokens.begin(); it != tokens.end(); ++it) {
                        stopWords.insert(*it);
                    }
                }
                ifs.close();
    } else {
        cerr << "ERROR: No se pudo abrir el archivo de StopWords: " << fichStopWords << endl;
    }
}

IndexadorHash::IndexadorHash(const IndexadorHash& other)
    : indice(other.indice), indiceDocs(other.indiceDocs), informacionColeccionDocs(other.informacionColeccionDocs),
      pregunta(other.pregunta), indicePregunta(other.indicePregunta), infPregunta(other.infPregunta),
      stopWords(other.stopWords), ficheroStopWords(other.ficheroStopWords), tok(other.tok),
      directorioIndice(other.directorioIndice), tipoStemmer(other.tipoStemmer), almacenarPosTerm(other.almacenarPosTerm) {}

IndexadorHash::~IndexadorHash() {
    indice.clear();
    indiceDocs.clear();
    stopWords.clear();
    indicePregunta.clear();
}

IndexadorHash& IndexadorHash::operator=(const IndexadorHash& other) {
    if (this != &other) {
        indice = other.indice;
        indiceDocs = other.indiceDocs;
        informacionColeccionDocs = other.informacionColeccionDocs;
        pregunta = other.pregunta;
        indicePregunta = other.indicePregunta;
        infPregunta = other.infPregunta;
        stopWords = other.stopWords;
        ficheroStopWords = other.ficheroStopWords;
        tok = other.tok;
        directorioIndice = other.directorioIndice;
        tipoStemmer = other.tipoStemmer;
        almacenarPosTerm = other.almacenarPosTerm;
    }
    return *this;
}

bool IndexadorHash::BorraDoc(const string& nomDoc) {
    auto itDoc = indiceDocs.find(nomDoc);
    if (itDoc == indiceDocs.end()) {
        return false;
    }

    int id_a_borrar = itDoc->second.idDoc;

    informacionColeccionDocs.numDocs--;
    informacionColeccionDocs.numTotalPal -= itDoc->second.numPal;
    informacionColeccionDocs.numTotalPalSinParada -= itDoc->second.numPalSinParada;
    informacionColeccionDocs.tamBytes -= itDoc->second.tamBytes;

    for (auto it = indice.begin(); it != indice.end(); ) {
        auto &postings = it->second.l_docs;

        for (auto pit = postings.begin(); pit != postings.end(); ++pit) {
            if (pit->first == id_a_borrar) {
                it->second.ftc -= pit->second.ft;
                postings.erase(pit);
                break;
            }
        }

        if (it->second.l_docs.empty()) {
            it = indice.erase(it);
        } else {
            ++it;
        }
    }

    indiceDocs.erase(itDoc);
    informacionColeccionDocs.numTotalPalDiferentes = indice.size();

    return true;
}

static InfTermDoc* BuscarPosting(vector<pair<int, InfTermDoc>>& l_docs, int idDoc) {
    for (auto &par : l_docs) {
        if (par.first == idDoc) return &par.second;
    }
    return nullptr;
}

static const InfTermDoc* BuscarPosting(const vector<pair<int, InfTermDoc>>& l_docs, int idDoc) {
    for (const auto &par : l_docs) {
        if (par.first == idDoc) return &par.second;
    }
    return nullptr;
}

bool IndexadorHash::Indexar(const string& ficheroDocumentos) {
    ifstream listaDocs(ficheroDocumentos);
    if (!listaDocs) {
        cerr << "ERROR: No existe el archivo con la lista de documentos: " << ficheroDocumentos << "\n";
        return false;
    }

    string ruta_doc;
    stemmerPorter stemmer;

    while (getline(listaDocs, ruta_doc)) {
        if (ruta_doc.empty()) continue;

        struct stat info_fichero;
        if (stat(ruta_doc.c_str(), &info_fichero) == -1) {
            cerr << "ERROR: No existe el documento a indexar: " << ruta_doc << "\n";
            continue;
        }

        int id_doc_actual = 0;

        auto itExistente = indiceDocs.find(ruta_doc);
        if (itExistente != indiceDocs.end()) {
            if (info_fichero.st_mtime > itExistente->second.fechaModificacion) {
                id_doc_actual = itExistente->second.idDoc;
                BorraDoc(ruta_doc);
            } else {
                continue;
            }
        } else {
            id_doc_actual = informacionColeccionDocs.numDocs + 1;
        }

        ifstream doc(ruta_doc);
        if (!doc) continue;

        InfDoc info_documento;
        info_documento.idDoc = id_doc_actual;
        info_documento.tamBytes = info_fichero.st_size;
        info_documento.fechaModificacion = info_fichero.st_mtime;

        string linea;
        int posicion_palabra = 0;
        list<string> tokens;

        while (getline(doc, linea)) {
            tokens.clear();
            tok.Tokenizar(linea, tokens);

            for (auto it = tokens.begin(); it != tokens.end(); ++it) {
                string termino = *it;
                info_documento.numPal++;

                if (stopWords.find(termino) == stopWords.end()) {
                    info_documento.numPalSinParada++;

                    if (tipoStemmer > 0) {
                        stemmer.stemmer(termino, tipoStemmer);
                    }

                    InformacionTermino& infoTerm = indice[termino];
                    infoTerm.ftc++;

                    InfTermDoc* infoTermDoc = nullptr;

                    if (!infoTerm.l_docs.empty() && infoTerm.l_docs.back().first == id_doc_actual) {
                        infoTermDoc = &infoTerm.l_docs.back().second;
                    } else {
                        infoTerm.l_docs.emplace_back(id_doc_actual, InfTermDoc());
                        infoTermDoc = &infoTerm.l_docs.back().second;
                        info_documento.numPalDiferentes++;
                    }

                    infoTermDoc->ft++;

                    if (almacenarPosTerm) {
                        infoTermDoc->posTerm.push_back(posicion_palabra);
                    }
                }

                posicion_palabra++;
            }
        }

        doc.close();

        indiceDocs[ruta_doc] = info_documento;
        informacionColeccionDocs.numDocs++;
        informacionColeccionDocs.numTotalPal += info_documento.numPal;
        informacionColeccionDocs.numTotalPalSinParada += info_documento.numPalSinParada;
        informacionColeccionDocs.tamBytes += info_documento.tamBytes;
    }

    informacionColeccionDocs.numTotalPalDiferentes = indice.size();

    listaDocs.close();
    return true;
}

bool IndexadorHash::Devuelve(const string& word, InformacionTermino& inf) const {
    string termino = word;
    list<string> t;
    tok.Tokenizar(word, t);
    if (t.empty()) return false;
    termino = t.front();

    if (tipoStemmer > 0) {
        stemmerPorter st;
        st.stemmer(termino, tipoStemmer);
    }

    auto it = indice.find(termino);
    if (it != indice.end()) {
        inf = it->second;
        return true;
    }
    return false;
}

bool IndexadorHash::Devuelve(const string& word, const string& nomDoc, InfTermDoc& InfoDoc) const {
    InformacionTermino infTerm;

    if (Devuelve(word, infTerm)) {
        auto itDoc = indiceDocs.find(nomDoc);
        if (itDoc != indiceDocs.end()) {
            int idBuscado = itDoc->second.idDoc;

            const InfTermDoc* itTermDoc = BuscarPosting(infTerm.l_docs, idBuscado);
            if (itTermDoc != nullptr) {
                InfoDoc = *itTermDoc;
                return true;
            }
        }
    }

    InfoDoc = InfTermDoc();
    return false;
}

bool IndexadorHash::Existe(const string& word) const {
    InformacionTermino dummy;
    return Devuelve(word, dummy);
}

int IndexadorHash::NumPalIndexadas() const { return indice.size(); }

string IndexadorHash::DevolverFichPalParada() const { return ficheroStopWords; }

int IndexadorHash::NumPalParada() const { return stopWords.size(); }

string IndexadorHash::DevolverDelimitadores() const { return tok.DelimitadoresPalabra(); }

bool IndexadorHash::DevolverCasosEspeciales() const {
    return const_cast<Tokenizador&>(tok).CasosEspeciales();
}

bool IndexadorHash::DevolverPasarAminuscSinAcentos() const {
    return const_cast<Tokenizador&>(tok).PasarAminuscSinAcentos();
}

bool IndexadorHash::DevolverAlmacenarPosTerm() const { return almacenarPosTerm; }

string IndexadorHash::DevolverDirIndice() const { return directorioIndice; }

int IndexadorHash::DevolverTipoStemming() const { return tipoStemmer; }

void IndexadorHash::ListarPalParada() const {
    for (auto it = stopWords.begin(); it != stopWords.end(); ++it) {
        cout << *it << endl;
    }
}

void IndexadorHash::ListarInfColeccDocs() const {
    cout << informacionColeccionDocs << endl;
}

void IndexadorHash::ListarTerminos() const {
    for (auto it = indice.begin(); it != indice.end(); ++it) {
        cout << it->first << '\t' << it->second << endl;
    }
}

bool IndexadorHash::ListarTerminos(const string& nomDoc) const {
    auto itDoc = indiceDocs.find(nomDoc);
    if (itDoc == indiceDocs.end()) return false;

    int idBuscado = itDoc->second.idDoc;

    for (auto it = indice.begin(); it != indice.end(); ++it) {
        const InfTermDoc* itLDocs = BuscarPosting(it->second.l_docs, idBuscado);
        if (itLDocs != nullptr) {
            cout << it->first << '\t' << it->second << endl;
        }
    }
    return true;
}

void IndexadorHash::ListarDocs() const {
    for (auto it = indiceDocs.begin(); it != indiceDocs.end(); ++it) {
        cout << it->first << '\t' << it->second << endl;
    }
}

bool IndexadorHash::ListarDocs(const string& nomDoc) const {
    auto it = indiceDocs.find(nomDoc);
    if (it != indiceDocs.end()) {
        cout << it->first << '\t' << it->second << endl;
        return true;
    }
    return false;
}

void IndexadorHash::VaciarIndiceDocs() {
    indice.clear();
    indiceDocs.clear();

    informacionColeccionDocs = InfColeccionDocs();
}

void IndexadorHash::VaciarIndicePreg() {
    pregunta = "";
    indicePregunta.clear();

    infPregunta = InformacionPregunta();
}

bool IndexadorHash::IndexarPregunta(const string& preg) {

    VaciarIndicePreg();

    list<string> tokens;
    tok.Tokenizar(preg, tokens);

    if (tokens.empty()) return false;

    pregunta = preg;
    stemmerPorter stemmer;
    int posicion_palabra = 0;

for (auto it = tokens.begin(); it != tokens.end(); ++it) {
        string termino = *it;
        infPregunta.numTotalPal++;

        if (stopWords.find(termino) == stopWords.end()) {
            infPregunta.numTotalPalSinParada++;

            if (tipoStemmer > 0) {
                stemmer.stemmer(termino, tipoStemmer);
            }

            InformacionTerminoPregunta& infoTermPreg = indicePregunta[termino];
            if (infoTermPreg.ft == 0) {
                infPregunta.numTotalPalDiferentes++;
            }
            infoTermPreg.ft++;

            if (almacenarPosTerm) {
                infoTermPreg.posTerm.push_back(posicion_palabra);
            }
        }
        posicion_palabra++;
    }

    if (infPregunta.numTotalPalSinParada == 0) {
        return false;
    }

    return true;
}

bool IndexadorHash::DevuelvePregunta(string& preg) const {
    if (indicePregunta.empty()) {
        preg = "";
        return false;
    }
    preg = pregunta;
    return true;
}

bool IndexadorHash::DevuelvePregunta(const string& word, InformacionTerminoPregunta& inf) const {
    string termino = word;
    list<string> tokens;

    tok.Tokenizar(word, tokens);
    if (tokens.empty()) {
        inf = InformacionTerminoPregunta();
        return false;
    }
    termino = tokens.front();

    if (tipoStemmer > 0) {
        stemmerPorter stemmer;
        stemmer.stemmer(termino, tipoStemmer);
    }

    auto it = indicePregunta.find(termino);
    if (it != indicePregunta.end()) {
        inf = it->second;
        return true;
    }

    inf = InformacionTerminoPregunta();
    return false;
}

bool IndexadorHash::DevuelvePregunta(InformacionPregunta& inf) const {
    if (indicePregunta.empty()) {
        inf = InformacionPregunta();
        return false;
    }
    inf = infPregunta;
    return true;
}

bool IndexadorHash::IndexarDirectorio(const string& dirAIndexar) {
    struct stat dir_stat;

    if (stat(dirAIndexar.c_str(), &dir_stat) == -1 || !S_ISDIR(dir_stat.st_mode)) {
        return false;
    }

    DIR *dirp = opendir(dirAIndexar.c_str());
    if (dirp == NULL) {
        return false;
    }

    struct dirent *direntp;
    stemmerPorter stemmer;

    while ((direntp = readdir(dirp)) != NULL) {
        string nombreFichero = direntp->d_name;

        if (nombreFichero == "." || nombreFichero == "..") continue;

        string ruta_completa = dirAIndexar + "/" + nombreFichero;
        struct stat info_fichero;

        if (stat(ruta_completa.c_str(), &info_fichero) == -1) continue;

        if (S_ISDIR(info_fichero.st_mode)) {

            IndexarDirectorio(ruta_completa);
        } else if (S_ISREG(info_fichero.st_mode)) {

            int id_doc_actual = 0;

            auto itExistente = indiceDocs.find(ruta_completa);
            if (itExistente != indiceDocs.end()) {
                if (info_fichero.st_mtime > itExistente->second.fechaModificacion) {
                    id_doc_actual = itExistente->second.idDoc;
                    BorraDoc(ruta_completa);
                } else {
                    continue;
                }
            } else {
                id_doc_actual = informacionColeccionDocs.numDocs + 1;
            }

            ifstream doc(ruta_completa);
            if (!doc) continue;

            InfDoc info_documento;
            info_documento.idDoc = id_doc_actual;
            info_documento.tamBytes = info_fichero.st_size;
            info_documento.fechaModificacion = info_fichero.st_mtime;

            string linea;
            int posicion_palabra = 0;
            list<string> tokens;

            while (getline(doc, linea)) {
                tokens.clear();
                tok.Tokenizar(linea, tokens);

                for (auto it = tokens.begin(); it != tokens.end(); ++it) {
                    string termino = *it;
                    info_documento.numPal++;

                    if (stopWords.find(termino) == stopWords.end()) {
                        info_documento.numPalSinParada++;

                        if (tipoStemmer > 0) {
                            stemmer.stemmer(termino, tipoStemmer);
                        }

                        InformacionTermino& infoTerm = indice[termino];
                        infoTerm.ftc++;

                        InfTermDoc* infoTermDoc = nullptr;

                        if (!infoTerm.l_docs.empty() && infoTerm.l_docs.back().first == id_doc_actual) {
                            infoTermDoc = &infoTerm.l_docs.back().second;
                        } else {
                            infoTerm.l_docs.emplace_back(id_doc_actual, InfTermDoc());
                            infoTermDoc = &infoTerm.l_docs.back().second;
                            info_documento.numPalDiferentes++;
                        }

                        infoTermDoc->ft++;
                        if (almacenarPosTerm) {
                            infoTermDoc->posTerm.push_back(posicion_palabra);
                        }
                    }
                    posicion_palabra++;
                }
            }
            doc.close();

            indiceDocs[ruta_completa] = info_documento;
            informacionColeccionDocs.numDocs++;
            informacionColeccionDocs.numTotalPal += info_documento.numPal;
            informacionColeccionDocs.numTotalPalSinParada += info_documento.numPalSinParada;
            informacionColeccionDocs.tamBytes += info_documento.tamBytes;
        }
    }
    closedir(dirp);

    informacionColeccionDocs.numTotalPalDiferentes = indice.size();

    return true;
}

bool IndexadorHash::GuardarIndexacion() const {
    string rutaDir = directorioIndice.empty() ? "." : directorioIndice;
    struct stat st;
    if (stat(rutaDir.c_str(), &st) != 0) {
        if (mkdir(rutaDir.c_str(), 0777) == -1) return false;
    }

    string rutaArchivo = rutaDir + "/indexador_backup.txt";
    ofstream out(rutaArchivo);
    if (!out) return false;

    out << ficheroStopWords << "\n";
    out << directorioIndice << "\n";
    out << tipoStemmer << " " << almacenarPosTerm << "\n";
    out << const_cast<Tokenizador&>(tok).DelimitadoresPalabra() << "\n";
    out << const_cast<Tokenizador&>(tok).CasosEspeciales() << " " << const_cast<Tokenizador&>(tok).PasarAminuscSinAcentos() << "\n";

    out << stopWords.size() << "\n";
    for (const string& sw : stopWords) {
        out << sw << "\n";
    }

    out << informacionColeccionDocs.numDocs << " "
        << informacionColeccionDocs.numTotalPal << " "
        << informacionColeccionDocs.numTotalPalSinParada << " "
        << informacionColeccionDocs.numTotalPalDiferentes << " "
        << informacionColeccionDocs.tamBytes << "\n";

    out << indiceDocs.size() << "\n";
    for (const auto& par : indiceDocs) {
        out << par.first << "\n";
        out << par.second.idDoc << " " << par.second.numPal << " "
            << par.second.numPalSinParada << " " << par.second.numPalDiferentes << " "
            << par.second.tamBytes << " " << par.second.fechaModificacion << "\n";
    }

    out << indice.size() << "\n";
    for (const auto& par : indice) {
        out << par.first << "\n";
        out << par.second.ftc << " " << par.second.l_docs.size() << "\n";

        for (const auto& doc_par : par.second.l_docs) {

            out << doc_par.first << " " << doc_par.second.ft << " " << doc_par.second.posTerm.size();
            for (int pos : doc_par.second.posTerm) {
                out << " " << pos;
            }
            out << "\n";
        }
    }

    out << pregunta << "\n";
    out << infPregunta.numTotalPal << " " << infPregunta.numTotalPalSinParada << " " << infPregunta.numTotalPalDiferentes << "\n";

    out << indicePregunta.size() << "\n";
    for (const auto& par : indicePregunta) {
        out << par.first << "\n";
        out << par.second.ft << " " << par.second.posTerm.size();
        for (int pos : par.second.posTerm) {
            out << " " << pos;
        }
        out << "\n";
    }

    out.close();
    return true;
}

bool IndexadorHash::RecuperarIndexacion(const string& directorioIndexacion) {
    string rutaDir = directorioIndexacion.empty() ? "." : directorioIndexacion;
    string rutaArchivo = rutaDir + "/indexador_backup.txt";

    ifstream in(rutaArchivo);
    if (!in) return false;

    VaciarIndiceDocs();
    VaciarIndicePreg();
    stopWords.clear();

    getline(in >> ws, ficheroStopWords);
    getline(in >> ws, directorioIndice);
    in >> tipoStemmer >> almacenarPosTerm;

    in.ignore(256, '\n');
    string delims;
    getline(in, delims);

    bool casosEsp, pasarMin;
    in >> casosEsp >> pasarMin;
    tok = Tokenizador(delims, casosEsp, pasarMin);

    size_t sw_size = 0;
    if (!(in >> sw_size)) return false;
    for (size_t i = 0; i < sw_size; ++i) {
        string sw;
        in >> sw;
        stopWords.insert(sw);
    }

    in >> informacionColeccionDocs.numDocs >> informacionColeccionDocs.numTotalPal
       >> informacionColeccionDocs.numTotalPalSinParada >> informacionColeccionDocs.numTotalPalDiferentes
       >> informacionColeccionDocs.tamBytes;

    size_t docs_size = 0;
    if (!(in >> docs_size)) return false;
    for (size_t i = 0; i < docs_size; ++i) {
        string rutaDoc;
        getline(in >> ws, rutaDoc);
        InfDoc inf;
        in >> inf.idDoc >> inf.numPal >> inf.numPalSinParada >> inf.numPalDiferentes >> inf.tamBytes >> inf.fechaModificacion;
        indiceDocs[rutaDoc] = inf;
    }

    size_t indice_size = 0;
    if (!(in >> indice_size)) return false;
    for (size_t i = 0; i < indice_size; ++i) {
        string termino;
        in >> termino;

        InformacionTermino infTerm;
        size_t l_docs_size = 0;
        in >> infTerm.ftc >> l_docs_size;

        for (size_t j = 0; j < l_docs_size; ++j) {
            int idDoc;
            InfTermDoc infTermDoc;
            size_t pos_size = 0;
            in >> idDoc >> infTermDoc.ft >> pos_size;

            for (size_t k = 0; k < pos_size; ++k) {
                int pos;
                in >> pos;
                infTermDoc.posTerm.push_back(pos);
            }
            infTerm.l_docs.emplace_back(idDoc, infTermDoc);
        }
        indice[termino] = infTerm;
    }

    getline(in >> ws, pregunta);
    if (in >> infPregunta.numTotalPal >> infPregunta.numTotalPalSinParada >> infPregunta.numTotalPalDiferentes) {
        size_t preg_size = 0;
        if (in >> preg_size) {
            for (size_t i = 0; i < preg_size; ++i) {
                string termino;
                in >> termino;

                InformacionTerminoPregunta infTermPreg;
                size_t pos_size = 0;
                in >> infTermPreg.ft >> pos_size;

                for (size_t j = 0; j < pos_size; ++j) {
                    int pos;
                    in >> pos;
                    infTermPreg.posTerm.push_back(pos);
                }
                indicePregunta[termino] = infTermPreg;
            }
        }
    }

    in.close();
    return true;
}

IndexadorHash::IndexadorHash(const string& directorioIndexacion) {
    tipoStemmer = 0;
    almacenarPosTerm = false;
    RecuperarIndexacion(directorioIndexacion);
}
