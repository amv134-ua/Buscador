#include "buscador.h"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <algorithm>
#include <unordered_map>

using namespace std;


/*
 * Clase ResultadoRI
 */

ResultadoRI::ResultadoRI(const double& kvSimilitud,
                         const long int& kidDoc,
                         const int& np)
{
    vSimilitud = kvSimilitud;
    idDoc = kidDoc;
    numPregunta = np;
}

double
ResultadoRI::VSimilitud() const
{
    return vSimilitud;
}

long int
ResultadoRI::IdDoc() const
{
    return idDoc;
}

int
ResultadoRI::NumPregunta() const
{
    return numPregunta;
}

bool
ResultadoRI::operator< (const ResultadoRI& lhs) const
{
    if(numPregunta == lhs.numPregunta)
    {
        if(vSimilitud == lhs.vSimilitud)
            return idDoc > lhs.idDoc;

        return vSimilitud < lhs.vSimilitud;
    }
    else
    {
        return numPregunta > lhs.numPregunta;
    }
}
ostream&
operator<<(ostream& os, const ResultadoRI& res)
{
    os << res.vSimilitud << "\t\t" << res.idDoc << "\t" << res.numPregunta << endl;
    return os;
}

static double Log2(const double& x)
{
    return log(x) / log(2.0);
}

static string NombreDocSinRutaNiExtension(const string& ruta)
{
    size_t posBarra = ruta.find_last_of("/\\");
    string nombre = (posBarra == string::npos) ? ruta : ruta.substr(posBarra + 1);

    size_t posPunto = nombre.find_last_of('.');
    if(posPunto != string::npos)
        nombre = nombre.substr(0, posPunto);

    return nombre;
}
static bool LeerFicheroCompleto(const string& nombreFichero, string& contenido)
{
    ifstream fich(nombreFichero.c_str());

    if(!fich)
        return false;

    ostringstream buffer;
    buffer << fich.rdbuf();

    contenido = buffer.str();

    fich.close();

    return true;
}
/*
 * Clase Buscador
 */

 Buscador::Buscador()
    : IndexadorHash("", "", false, false, "", 0, false)
{
    formSimilitud = 0;
    c = 2;
    k1 = 1.2;
    b = 0.75;
}
Buscador::Buscador(const string& directorioIndexacion, const int& f)
    : IndexadorHash(directorioIndexacion)
{
    if(f == 0 || f == 1)
        formSimilitud = f;
    else
        formSimilitud = 0;

    c = 2;
    k1 = 1.2;
    b = 0.75;
}

Buscador::Buscador(const Buscador& busc)
    : IndexadorHash(busc)
{
    docsOrdenados = busc.docsOrdenados;

    formSimilitud = busc.formSimilitud;

    c = busc.c;
    k1 = busc.k1;
    b = busc.b;
}

Buscador::~Buscador()
{
    while(!docsOrdenados.empty())
        docsOrdenados.pop();
}

Buscador&
Buscador::operator= (const Buscador& busc)
{
    if(this != &busc)
    {
        IndexadorHash::operator=(busc);

        docsOrdenados = busc.docsOrdenados;

        formSimilitud = busc.formSimilitud;

        c = busc.c;
        k1 = busc.k1;
        b = busc.b;
    }

    return *this;
}


/*
 * Métodos de búsqueda
 * 
 * De momento los dejamos como esqueleto para que compile.
 * Luego aquí meteremos BM25 y DFR.
 */

bool
Buscador::Buscar(const int& numDocumentos)
{
    while(!docsOrdenados.empty())
        docsOrdenados.pop();

    string preg;

    if(!DevuelvePregunta(preg))
        return false;

    if(indicePregunta.empty())
        return false;

    if(informacionColeccionDocs.numDocs == 0)
        return false;

    double N = informacionColeccionDocs.numDocs;

    double avgdl = 0.0;

    if(informacionColeccionDocs.numDocs > 0)
        avgdl = (double) informacionColeccionDocs.numTotalPalSinParada /
                (double) informacionColeccionDocs.numDocs;

    if(avgdl == 0.0)
        return false;

    unordered_map<int, double> puntuaciones;

    unordered_map<int, InfDoc> docsPorId;

    for(auto itDoc = indiceDocs.begin(); itDoc != indiceDocs.end(); ++itDoc)
    {
        docsPorId[itDoc->second.idDoc] = itDoc->second;
    }

    try
    {
        for(auto itPreg = indicePregunta.begin(); itPreg != indicePregunta.end(); ++itPreg)
        {
            const string& termino = itPreg->first;
            const InformacionTerminoPregunta& infoPreg = itPreg->second;

            auto itIndice = indice.find(termino);

            if(itIndice == indice.end())
                continue;

            const InformacionTermino& infoTerm = itIndice->second;

            double ft = infoTerm.ftc;
            double nt = infoTerm.l_docs.size();

            if(ft <= 0 || nt <= 0)
                continue;

            for(auto itPosting = infoTerm.l_docs.begin(); itPosting != infoTerm.l_docs.end(); ++itPosting)
            {
                int idDoc = itPosting->first;
                const InfTermDoc& infoTermDoc = itPosting->second;

                auto itInfDoc = docsPorId.find(idDoc);

                if(itInfDoc == docsPorId.end())
                    continue;

                const InfDoc& infoDoc = itInfDoc->second;

                double ftd = infoTermDoc.ft;
                double ld = infoDoc.numPalSinParada;

                if(ftd <= 0 || ld <= 0)
                    continue;

                double scoreTermino = 0.0;

                if(formSimilitud == 0)
                {
                    /*
                     * DFR
                     *
                     * sim(q,d) = sumatorio wi,q * wi,d
                     */

                    double k = infPregunta.numTotalPalSinParada;

                    if(k <= 0)
                        continue;

                    double ftq = infoPreg.ft;
                    double wiq = ftq / k;

                    double lambda = ft / N;

                    if(lambda <= 0)
                        continue;

                    double ftdPrima = ftd * Log2(1.0 + (c * avgdl / ld));

                    double wid = (Log2(1.0 + lambda) +
                                  ftdPrima * Log2((1.0 + lambda) / lambda))
                                 *
                                 ((ft + 1.0) / (nt * (ftdPrima + 1.0)));

                    scoreTermino = wiq * wid;
                }
                else
                {
                    /*
                     * BM25
                     */

                    double idf = Log2((N - nt + 0.5) / (nt + 0.5));

                    double denominador = ftd + k1 * (1.0 - b + b * (ld / avgdl));

                    if(denominador == 0.0)
                        continue;

                    scoreTermino = idf * ((ftd * (k1 + 1.0)) / denominador);
                }

                puntuaciones[idDoc] += scoreTermino;
            }
        }
    }
    catch(bad_alloc&)
    {
        cerr << "ERROR: falta de memoria durante la busqueda" << endl;
        return false;
    }

    vector<ResultadoRI> resultados;

    for(auto it = puntuaciones.begin(); it != puntuaciones.end(); ++it)
    {
        resultados.push_back(ResultadoRI(it->second, it->first, 0));
    }

    sort(resultados.begin(), resultados.end(),
         [](const ResultadoRI& a, const ResultadoRI& b)
         {
             if(a.VSimilitud() == b.VSimilitud())
                 return a.IdDoc() < b.IdDoc();

             return a.VSimilitud() > b.VSimilitud();
         });

    int limite = numDocumentos;

    if(limite < 0)
        limite = 0;

    for(int i = 0; i < (int) resultados.size() && i < limite; i++)
    {
        docsOrdenados.push(resultados[i]);
    }

    return true;
}

bool
Buscador::Buscar(const string& dirPreguntas,
                 const int& numDocumentos,
                 const int& numPregInicio,
                 const int& numPregFin)
{
    while(!docsOrdenados.empty())
        docsOrdenados.pop();

    if(numPregInicio > numPregFin)
        return false;

    if(informacionColeccionDocs.numDocs == 0)
        return false;

    double N = informacionColeccionDocs.numDocs;

    double avgdl = 0.0;

    if(informacionColeccionDocs.numDocs > 0)
        avgdl = (double) informacionColeccionDocs.numTotalPalSinParada /
                (double) informacionColeccionDocs.numDocs;

    if(avgdl == 0.0)
        return false;

    unordered_map<int, InfDoc> docsPorId;

    for(auto itDoc = indiceDocs.begin(); itDoc != indiceDocs.end(); ++itDoc)
    {
        docsPorId[itDoc->second.idDoc] = itDoc->second;
    }

    try
    {
        for(int numPreg = numPregInicio; numPreg <= numPregFin; numPreg++)
        {
            string rutaPregunta = dirPreguntas;

            if(!rutaPregunta.empty() &&
               rutaPregunta[rutaPregunta.size() - 1] != '/' &&
               rutaPregunta[rutaPregunta.size() - 1] != '\\')
            {
                rutaPregunta += "/";
            }

            rutaPregunta += to_string(numPreg);
            rutaPregunta += ".txt";

            string textoPregunta;

            if(!LeerFicheroCompleto(rutaPregunta, textoPregunta))
            {
                cerr << "ERROR: no se pudo abrir el fichero de pregunta: "
                     << rutaPregunta << endl;
                return false;
            }

            if(!IndexarPregunta(textoPregunta))
            {
                continue;
            }

            unordered_map<int, double> puntuaciones;

            for(auto itPreg = indicePregunta.begin(); itPreg != indicePregunta.end(); ++itPreg)
            {
                const string& termino = itPreg->first;
                const InformacionTerminoPregunta& infoPreg = itPreg->second;

                auto itIndice = indice.find(termino);

                if(itIndice == indice.end())
                    continue;

                const InformacionTermino& infoTerm = itIndice->second;

                double ft = infoTerm.ftc;
                double nt = infoTerm.l_docs.size();

                if(ft <= 0 || nt <= 0)
                    continue;

                for(auto itPosting = infoTerm.l_docs.begin(); itPosting != infoTerm.l_docs.end(); ++itPosting)
                {
                    int idDoc = itPosting->first;
                    const InfTermDoc& infoTermDoc = itPosting->second;

                    auto itInfDoc = docsPorId.find(idDoc);

                    if(itInfDoc == docsPorId.end())
                        continue;

                    const InfDoc& infoDoc = itInfDoc->second;

                    double ftd = infoTermDoc.ft;
                    double ld = infoDoc.numPalSinParada;

                    if(ftd <= 0 || ld <= 0)
                        continue;

                    double scoreTermino = 0.0;

                    if(formSimilitud == 0)
                    {
                        /*
                         * DFR
                         *
                         * sim(q,d) = sumatorio wi,q * wi,d
                         */

                        double k = infPregunta.numTotalPalSinParada;

                        if(k <= 0)
                            continue;

                        double ftq = infoPreg.ft;
                        double wiq = ftq / k;

                        double lambda = ft / N;

                        if(lambda <= 0)
                            continue;

                        double ftdPrima = ftd * Log2(1.0 + (c * avgdl / ld));

                        double wid = (Log2(1.0 + lambda) +
                                      ftdPrima * Log2((1.0 + lambda) / lambda))
                                     *
                                     ((ft + 1.0) / (nt * (ftdPrima + 1.0)));

                        scoreTermino = wiq * wid;
                    }
                    else
                    {
                        /*
                         * BM25
                         */

                        double idf = Log2((N - nt + 0.5) / (nt + 0.5));

                        double denominador = ftd + k1 * (1.0 - b + b * (ld / avgdl));

                        if(denominador == 0.0)
                            continue;

                        scoreTermino = idf * ((ftd * (k1 + 1.0)) / denominador);
                    }

                    puntuaciones[idDoc] += scoreTermino;
                }
            }

            vector<ResultadoRI> resultados;

            for(auto it = puntuaciones.begin(); it != puntuaciones.end(); ++it)
            {
                resultados.push_back(ResultadoRI(it->second, it->first, numPreg));
            }

            sort(resultados.begin(), resultados.end(),
                 [](const ResultadoRI& a, const ResultadoRI& b)
                 {
                     if(a.VSimilitud() == b.VSimilitud())
                         return a.IdDoc() < b.IdDoc();

                     return a.VSimilitud() > b.VSimilitud();
                 });

            int limite = numDocumentos;

            if(limite < 0)
                limite = 0;

            for(int i = 0; i < (int) resultados.size() && i < limite; i++)
            {
                docsOrdenados.push(resultados[i]);
            }
        }
    }
    catch(bad_alloc&)
    {
        cerr << "ERROR: falta de memoria durante la busqueda por lote" << endl;
        return false;
    }

    return true;
}


/*
 * Métodos de impresión de resultados
 * 
 * De momento los dejamos preparados.
 * Cuando implementemos Buscar(), estos imprimirán los documentos ordenados.
 */

void
Buscador::ImprimirResultadoBusqueda(const int& numDocumentos) const
{
    priority_queue<ResultadoRI> copia = docsOrdenados;

    unordered_map<int, string> nombresPorId;

    for(auto itDoc = indiceDocs.begin(); itDoc != indiceDocs.end(); ++itDoc)
    {
        nombresPorId[itDoc->second.idDoc] = NombreDocSinRutaNiExtension(itDoc->first);
    }

    string formula;

    if(formSimilitud == 0)
        formula = "DFR";
    else
        formula = "BM25";

    unordered_map<int, int> posicionesPorPregunta;

    while(!copia.empty())
    {
        ResultadoRI res = copia.top();
        copia.pop();

        int numPregunta = res.NumPregunta();

        int posicion = posicionesPorPregunta[numPregunta];

        if(posicion >= numDocumentos)
            continue;

        posicionesPorPregunta[numPregunta]++;

        string nombreDoc = "";

        auto itNombre = nombresPorId.find(res.IdDoc());

        if(itNombre != nombresPorId.end())
            nombreDoc = itNombre->second;

        string textoPregunta;

        if(numPregunta == 0)
            textoPregunta = pregunta;
        else
            textoPregunta = "ConjuntoDePreguntas";

        cout << numPregunta << " "
             << formula << " "
             << nombreDoc << " "
             << posicion << " "
             << fixed << setprecision(6) << res.VSimilitud() << " "
             << textoPregunta << endl;
    }
}

bool
Buscador::ImprimirResultadoBusqueda(const int& numDocumentos,
                                    const string& nombreFichero) const
{
    ofstream salida(nombreFichero.c_str());

    if(!salida)
        return false;

    priority_queue<ResultadoRI> copia = docsOrdenados;

    unordered_map<int, string> nombresPorId;

    for(auto itDoc = indiceDocs.begin(); itDoc != indiceDocs.end(); ++itDoc)
    {
        nombresPorId[itDoc->second.idDoc] = NombreDocSinRutaNiExtension(itDoc->first);
    }

    string formula;

    if(formSimilitud == 0)
        formula = "DFR";
    else
        formula = "BM25";

    unordered_map<int, int> posicionesPorPregunta;

    while(!copia.empty())
    {
        ResultadoRI res = copia.top();
        copia.pop();

        int numPregunta = res.NumPregunta();

        int posicion = posicionesPorPregunta[numPregunta];

        if(posicion >= numDocumentos)
            continue;

        posicionesPorPregunta[numPregunta]++;

        string nombreDoc = "";

        auto itNombre = nombresPorId.find(res.IdDoc());

        if(itNombre != nombresPorId.end())
            nombreDoc = itNombre->second;

        string textoPregunta;

        if(numPregunta == 0)
            textoPregunta = pregunta;
        else
            textoPregunta = "ConjuntoDePreguntas";

        salida << numPregunta << " "
               << formula << " "
               << nombreDoc << " "
               << posicion << " "
               << fixed << setprecision(6) << res.VSimilitud() << " "
               << textoPregunta << endl;
    }

    salida.close();

    return true;
}


/*
 * Métodos de fórmula de similitud
 */

int
Buscador::DevolverFormulaSimilitud() const
{
    return formSimilitud;
}

bool
Buscador::CambiarFormulaSimilitud(const int& f)
{
    if(f == 0 || f == 1)
    {
        formSimilitud = f;
        return true;
    }

    return false;
}


/*
 * Parámetros DFR
 */

void
Buscador::CambiarParametrosDFR(const double& kc)
{
    c = kc;
}

double
Buscador::DevolverParametrosDFR() const
{
    return c;
}


/*
 * Parámetros BM25
 */

void
Buscador::CambiarParametrosBM25(const double& kk1, const double& kb)
{
    k1 = kk1;
    b = kb;
}

void
Buscador::DevolverParametrosBM25(double& kk1, double& kb) const
{
    kk1 = k1;
    kb = b;
}