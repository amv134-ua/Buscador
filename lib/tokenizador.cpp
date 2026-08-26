#include <array>
#include <iostream>
#include <list>
#include <string>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdlib>

#include "tokenizador.h"

using std::array;
using std::ostream;
using std::string;
using std::list;

static string FiltraDelimitadores(const string& entrada){
    array<bool, 256> seen{}; 
    string out;
    out.reserve(entrada.size());

    for (unsigned char c : entrada) {
        if (!seen[c]) {
            seen[c] = true;
            out.push_back(static_cast<char>(c));
        }
    }
    return out;
}

ostream& operator<<(ostream& os, const Tokenizador& t){
    os << "DELIMITADORES: " << t.delimiters
       << " TRATA CASOS ESPECIALES: " << t.casosEspeciales
       << " PASAR A MINUSCULAS Y SIN ACENTOS: " << t.pasarAminuscSinAcentos;
    return os;
}

Tokenizador::Tokenizador(const string& delimitadoresPalabra,const bool& kcasosEspeciales,const bool& minuscSinAcentos){
    delimiters = FiltraDelimitadores(delimitadoresPalabra);
    casosEspeciales = kcasosEspeciales;
    pasarAminuscSinAcentos = minuscSinAcentos;
}

Tokenizador::Tokenizador(const Tokenizador& other){
    delimiters = other.delimiters;
    casosEspeciales = other.casosEspeciales;
    pasarAminuscSinAcentos = other.pasarAminuscSinAcentos;
}

Tokenizador::Tokenizador(){
    delimiters = ",;:.-/+*\\ '\"{}[]()<>ï¿½!ï¿½?&#=\t@";
    casosEspeciales = true;
    pasarAminuscSinAcentos = false;
}

Tokenizador::~Tokenizador(){
    delimiters.clear(); 
}

Tokenizador& Tokenizador::operator=(const Tokenizador& other){
    if (this != &other) {
        delimiters = other.delimiters;
        casosEspeciales = other.casosEspeciales;
        pasarAminuscSinAcentos = other.pasarAminuscSinAcentos;
    }
    return *this;
}

void Tokenizador::DelimitadoresPalabra(const string& nuevoDelimiters){
    delimiters = FiltraDelimitadores(nuevoDelimiters);
}

void Tokenizador::AnyadirDelimitadoresPalabra(const string& nuevoDelimiters){
    array<bool, 256> visto{};

    for (unsigned char c : delimiters) visto[c] = true;

    for (unsigned char c : nuevoDelimiters) {
        if (!visto[c]) {
            visto[c] = true;
            delimiters.push_back(static_cast<char>(c));
        }
    }
}

string Tokenizador::DelimitadoresPalabra() const{
    return delimiters;
}

void Tokenizador::CasosEspeciales(const bool& nuevoCasosEspeciales){
    casosEspeciales = nuevoCasosEspeciales;
}

bool Tokenizador::CasosEspeciales(){
    return casosEspeciales;
}

void Tokenizador::PasarAminuscSinAcentos(const bool& nuevoPasarAminuscSinAcentos){
    pasarAminuscSinAcentos = nuevoPasarAminuscSinAcentos;
}

bool Tokenizador::PasarAminuscSinAcentos(){
    return pasarAminuscSinAcentos;
}

static void PasarAMinusculasYSinAcentos(string& s){
    static const array<unsigned char, 256> m = []{
        array<unsigned char, 256> t{};
        for (int i = 0; i < 256; ++i) t[i] = (unsigned char)i;

        for (unsigned char c = 'A'; c <= 'Z'; ++c)
            t[c] = (unsigned char)(c - 'A' + 'a');

        auto set = [&](unsigned char from, unsigned char to){ t[from] = to; };

        for (unsigned char c : {'Á','À','Â','Ã','Ä','Å'}) set(c, 'a');
        for (unsigned char c : {'á','à','â','ã','ä','å'}) set(c, 'a');

        for (unsigned char c : {'É','È','Ê','Ë'}) set(c, 'e');
        for (unsigned char c : {'é','è','ê','ë'}) set(c, 'e');

        for (unsigned char c : {'Í','Ì','Î','Ï'}) set(c, 'i');
        for (unsigned char c : {'í','ì','î','ï'}) set(c, 'i');

        for (unsigned char c : {'Ó','Ò','Ô','Õ','Ö'}) set(c, 'o');
        for (unsigned char c : {'ó','ò','ô','õ','ö'}) set(c, 'o');

        for (unsigned char c : {'Ú','Ù','Û','Ü'}) set(c, 'u');
        for (unsigned char c : {'ú','ù','û','ü'}) set(c, 'u');

        set((unsigned char)'Ñ', (unsigned char)'ñ');
        set((unsigned char)'Ç', (unsigned char)'ç');

        return t;
    }();  

    for (size_t i = 0; i < s.size(); ++i) {
        unsigned char c = (unsigned char)s[i];
        s[i] = (char)m[c];
    }
}

bool Tokenizador::TokenizarCasosEspeciales(const string& txt,size_t& i,const std::array<bool,256>& isDelim,list<string>& tokens) const{
    if (!casosEspeciales) return false;

    const size_t n = txt.size();

    ///////////////URL///////////////
    auto prefURL = [&](size_t pos) -> size_t {
        if (pos + 5 <= n && txt.compare(pos, 5, "http:")  == 0) return 5;
        if (pos + 6 <= n && txt.compare(pos, 6, "https:") == 0) return 6;
        if (pos + 4 <= n && txt.compare(pos, 4, "ftp:")   == 0) return 4;
        return 0;
    };

    static const std::array<bool,256> urlOK = []{
        std::array<bool,256> a{};
        for (unsigned char c : string("_:/.?&-=#@%+")) a[c] = true;
        return a;
    }();

    size_t pref = prefURL(i);
    if (pref != 0 && (i + pref) < n) {
        size_t j = i;
        while (j < n) {
            unsigned char c = (unsigned char)txt[j];
            if (c == ' ' || c == '\n' || c == '\r') break;
            if (isDelim[c] && !urlOK[c]) break;
            ++j;
        }
        if (j > i) {
            tokens.emplace_back(txt.substr(i, j - i));
            i = j;
            return true;
        }
    }

    ///////////////Nï¿½meros decimales///////////////
    if (isDelim[(unsigned char)'.'] && isDelim[(unsigned char)',']) {
        size_t limit = i;
        bool tiene_letras = false;
        auto esDig = [&](unsigned char ch){ return ch >= '0' && ch <= '9'; };
        
        while (limit < n) {
            unsigned char c = (unsigned char)txt[limit];
            if (c == ' ' || c == '\n' || c == '\r') break;
            if (isDelim[c] && c != '.' && c != ',' && c != '%' && c != '$') break;
            if (!esDig(c) && c != '.' && c != ',' && c != '%' && c != '$') {
                tiene_letras = true;
            }
            ++limit;
        }

        if (!tiene_letras && limit > i) {
            size_t j = i;
            bool prefijoCero = false;
            bool isValidNum = false;

            if (j < limit && (txt[j] == '.' || txt[j] == ',') && j + 1 < limit && esDig((unsigned char)txt[j + 1])) {
                prefijoCero = true;
                isValidNum = true;
                ++j;
            } else if (j < limit && esDig((unsigned char)txt[j])) {
                isValidNum = true;
                ++j;
            }

            if (isValidNum) {
                while (j < limit) {
                    unsigned char c = (unsigned char)txt[j];
                    if (esDig(c)) { 
                        ++j; 
                        continue; 
                    }
                    if ((c == '.' || c == ',') && j + 1 < limit && esDig((unsigned char)txt[j + 1])) {
                        ++j; 
                        continue;
                    }
                    break;
                }
                
                if (prefijoCero) {
                    string tok = "0";
                    tok.append(txt, i, j - i);
                    tokens.emplace_back(std::move(tok));
                } else {
                    tokens.emplace_back(txt.substr(i, j - i));
                }
                i = j;
                return true;
            }
        }
    }

    ///////////////E-mail///////////////
    auto esAlnumU = [&](unsigned char ch){
        return (ch >= '0' && ch <= '9') ||
               (ch >= 'a' && ch <= 'z') ||
               (ch >= 'A' && ch <= 'Z');
    };

    if (isDelim[(unsigned char)'@'] && esAlnumU((unsigned char)txt[i])) {
        size_t limit = i;
        bool tiene_arroba = false;
        bool invalido = false;
        size_t pos_arroba = 0;
        
        while (limit < n) {
            unsigned char c = (unsigned char)txt[limit];
            if (c == ' ' || c == '\n' || c == '\r') break;

            if (!tiene_arroba) {
                if (c == '@') {
                    tiene_arroba = true;
                    pos_arroba = limit;
                } else if (isDelim[c]) {
                    break;
                }
            } else {
                if (c == '@') {
                    invalido = true; 
                    break;
                }
                if (isDelim[c] && c != '.' && c != '-' && c != '_') {
                    break;
                }
            }
            ++limit;
        }

        if (!invalido && tiene_arroba && pos_arroba > i) {
            size_t j = pos_arroba + 1;
            while (j < limit) {
                unsigned char c = txt[j];

                if (c == '.' || c == '-' || c == '_') {
                    if (j == pos_arroba + 1) break; 
                    if (j + 1 >= n) break;      
                    
                    unsigned char next_c = txt[j + 1];
                    if (next_c == ' ' || next_c == '\n' || next_c == '\r') break;
                    if (isDelim[next_c]) break; 
                }
                ++j;
            }

            if (j > pos_arroba + 1) {
                tokens.emplace_back(txt.substr(i, j - i));
                i = j;
                return true;
            }
        }
    }

    ///////////////Acrï¿½nimos///////////////

    if (isDelim[(unsigned char)'.']) {
        size_t limit = i;
        while (limit < n) {
            unsigned char c = (unsigned char)txt[limit];
            if (c == ' ' || c == '\n' || c == '\r') break;
            if (isDelim[c] && c != '.') break;
            ++limit;
        }

        bool tiene_punto = false;
        for (size_t k = i; k < limit; ++k) {
            if (txt[k] == '.') {
                if (k > i && txt[k-1] == '.') {
                    limit = k; 
                    break;
                }
                tiene_punto = true;
            }
        }

        if (tiene_punto) {
            size_t a = i;
            while (a < limit && txt[a] == '.') ++a; 
            size_t b = limit;
            while (b > a && txt[b - 1] == '.') --b; 

            if (a < b && b > a) {
                bool punto_interno = false;
                for (size_t k = a; k < b; ++k) {
                    if (txt[k] == '.') { punto_interno = true; break; }
                }
                if (punto_interno) {
                    tokens.emplace_back(txt.substr(a, b - a));
                    i = limit; 
                    return true;
                }
            }
        }
    }

    ///////////////Guiones///////////////
    if (isDelim[(unsigned char)'-'] && esAlnumU((unsigned char)txt[i])) {
        size_t limit = i;
        while (limit < n) {
            unsigned char c = (unsigned char)txt[limit];
            if (c == ' ' || c == '\n' || c == '\r') break;
            if (isDelim[c] && c != '-') break;
            
            if (c == '-') {
                if (limit + 1 < n && txt[limit + 1] == '-') {
                    break; 
                }
            }
            ++limit;
        }

        bool tiene_guion = false;
        for (size_t k = i; k < limit; ++k) {
            if (txt[k] == '-') { tiene_guion = true; break; }
        }

        if (tiene_guion) {
            size_t a = i;
            size_t b = limit;
            while (b > a && txt[b - 1] == '-') --b; 
            
            if (b > a) {
                tokens.emplace_back(txt.substr(a, b - a));
                i = limit;
                return true;
            }
        }
    }

    return false;
}

void Tokenizador::Tokenizar(const string& str, list<string>& tokens) const{
    tokens.clear();

    const string* p = &str;
    string copia_trabajo ;
    if (pasarAminuscSinAcentos) {
        copia_trabajo  = str;
        PasarAMinusculasYSinAcentos(copia_trabajo );
        p = &copia_trabajo ;
    }
    const string& txt = *p;

    array<bool, 256> isDelim{};
    for (unsigned char c : delimiters) isDelim[c] = true;

    isDelim[(unsigned char)'\n'] = true;
    isDelim[(unsigned char)'\r'] = true;

    if (casosEspeciales) isDelim[(unsigned char)' '] = true;

    const size_t n = txt.size();
    size_t i = 0;

    while (i < n) {
        size_t old_i = i;

        if (casosEspeciales) {
            if (TokenizarCasosEspeciales(txt, i, isDelim, tokens)) continue;
        }

        if (i < n && isDelim[(unsigned char)txt[i]]) {
            ++i;
            continue;
        }

        size_t start = i;
        while (i < n && !isDelim[(unsigned char)txt[i]]) ++i;
        if (start < i) tokens.emplace_back(txt.substr(start, i - start));

        if (i == old_i) ++i;
    }
}

///////////////Mï¿½TODOS DE FICHEROS Y DIRECTORIOS///////////////

bool Tokenizador::Tokenizar(const string& i, const string& f) const {
    std::ifstream in(i);
    if (!in) {
        std::cerr << "ERROR: No existe el archivo i: " << i << "\n"; 
        return false;
    }

    std::ofstream out(f);
    if (!out) {
        std::cerr << "ERROR: No se pudo crear el archivo f: " << f << "\n";
        return false;
    }

    string linea;
    list<string> tokens_linea;
    
    // Leemos lï¿½nea a lï¿½nea para tener una eficiencia espacial ï¿½ptima 
    while (std::getline(in, linea)) {
        if (linea.empty()) continue;
        
        Tokenizar(linea, tokens_linea);
        
        for (const auto& token : tokens_linea) {
            out << token << "\n";
        }
    }
    return true;
}

bool Tokenizador::Tokenizar(const string& i) const {
    return Tokenizar(i, i + ".tk");
}

bool Tokenizador::TokenizarListaFicheros(const string& i) const {
    std::ifstream in(i);
    if (!in) {
        std::cerr << "ERROR: No existe el archivo con la lista: " << i << "\n";
        return false;
    }

    string ruta_fichero;
    bool exito_total = true;

    while (std::getline(in, ruta_fichero)) {
        if (ruta_fichero.empty()) continue;

        struct stat info_fichero;
        if (stat(ruta_fichero.c_str(), &info_fichero) == -1) {
            std::cerr << "ERROR: No existe el archivo: " << ruta_fichero << "\n";
            exito_total = false;
            continue; 
        }
        
        if (S_ISDIR(info_fichero.st_mode)) {
            std::cerr << "ERROR: Es un directorio, no un archivo: " << ruta_fichero << "\n";
            exito_total = false;
            continue; 
        }

        if (!Tokenizar(ruta_fichero)) {
            exito_total = false;
        }
    }

    return exito_total; 
}

bool Tokenizador::TokenizarDirectorio(const string& dirAIndexar) const {
    struct stat dir;
    int err = stat(dirAIndexar.c_str(), &dir);
    if (err == -1 || !S_ISDIR(dir.st_mode)) {
        std::cerr << "ERROR: No existe el directorio: " << dirAIndexar << "\n";
        return false;
    }

    string cmd = "find " + dirAIndexar + " -type f -not -name \"*.tk\" -follow | sort > .lista_fich";
    system(cmd.c_str());

    bool resultado = TokenizarListaFicheros(".lista_fich");
    
    system("rm -f .lista_fich");

    return resultado;
}