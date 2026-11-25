#include <windows.h>
#include <string>
#include <vector>
#include <sstream>
#include <cctype>

using namespace std;

// ============================================================
// MODELO LEXICO Y DE INSTRUCCION
// ============================================================
enum class TipoToken {
    MOV,
    ADD,
    STO,
    END_,
    REGISTRO,
    COMA,
    NUMERO
};

struct Token {
    TipoToken tipo;
    string lexema;
};

struct Instruccion {
    string operacion;
    vector<string> registros;
    int direccion = -1;
    bool tieneDireccion = false;
};

struct ResultadoAFD {
    bool valida = false;
    string error;
    vector<Token> tokens;
    Instruccion instruccion;
};

string nombreToken(TipoToken tipo) {
    switch (tipo) {
        case TipoToken::MOV: return "MOV";
        case TipoToken::ADD: return "ADD";
        case TipoToken::STO: return "STO";
        case TipoToken::END_: return "END";
        case TipoToken::REGISTRO: return "REGISTRO";
        case TipoToken::COMA: return "COMA";
        case TipoToken::NUMERO: return "NUMERO";
    }
    return "DESCONOCIDO";
}

// ============================================================
// AFD
// ============================================================
class AnalizadorAFD {
public:
    ResultadoAFD analizar(const string& entrada) {
        ResultadoAFD r;

        if (entrada.empty()) {
            r.error = "La instruccion esta vacia.";
            return r;
        }

        size_t i = 0;

        auto saltarEspacios = [&]() {
            while (i < entrada.size() && entrada[i] == ' ') ++i;
        };

        auto leerPalabra = [&]() -> string {
            string s;
            while (i < entrada.size() && isalpha(static_cast<unsigned char>(entrada[i]))) {
                s += entrada[i++];
            }
            return s;
        };

        auto leerNumero = [&]() -> string {
            string s;
            while (i < entrada.size() && isdigit(static_cast<unsigned char>(entrada[i]))) {
                s += entrada[i++];
            }
            return s;
        };

        auto soloEspaciosHastaFin = [&]() -> bool {
            size_t j = i;
            while (j < entrada.size() && entrada[j] == ' ') ++j;
            return j == entrada.size();
        };

        if (!isalpha(static_cast<unsigned char>(entrada[i]))) {
            r.error = "Se esperaba un mnemonico al inicio.";
            return r;
        }

        string op = leerPalabra();

        if (op == "MOV") {
            r.tokens.push_back({TipoToken::MOV, op});
            r.instruccion.operacion = "MOV";

            if (i >= entrada.size() || entrada[i] != ' ') {
                r.error = "Se esperaba uno o mas espacios despues de MOV.";
                return r;
            }

            saltarEspacios();
            string reg = leerPalabra();
            if (reg != "AL" && reg != "BL") {
                r.error = reg.empty() ? "Falta el registro destino despues de MOV."
                                      : "Registro invalido: " + reg + ". Solo se permite AL o BL.";
                return r;
            }

            r.tokens.push_back({TipoToken::REGISTRO, reg});
            r.instruccion.registros.push_back(reg);

            saltarEspacios();
            if (i >= entrada.size() || entrada[i] != ',') {
                r.error = "Falta la coma entre el registro y la direccion.";
                return r;
            }
            r.tokens.push_back({TipoToken::COMA, ","});
            ++i;

            saltarEspacios();
            if (i >= entrada.size() || !isdigit(static_cast<unsigned char>(entrada[i]))) {
                r.error = "Falta una direccion decimal no negativa despues de la coma.";
                return r;
            }

            string numero = leerNumero();
            r.tokens.push_back({TipoToken::NUMERO, numero});
            try {
                r.instruccion.direccion = stoi(numero);
                r.instruccion.tieneDireccion = true;
            } catch (...) {
                r.error = "La direccion numerica es demasiado grande.";
                return r;
            }

            if (!soloEspaciosHastaFin()) {
                r.error = "Sobran componentes despues de la direccion.";
                return r;
            }

            r.valida = true;
            return r;
        }

        if (op == "ADD") {
            r.tokens.push_back({TipoToken::ADD, op});
            r.instruccion.operacion = "ADD";

            if (i >= entrada.size() || entrada[i] != ' ') {
                r.error = "Se esperaba uno o mas espacios despues de ADD.";
                return r;
            }

            saltarEspacios();
            string reg1 = leerPalabra();
            if (reg1 != "AL") {
                r.error = reg1.empty() ? "Falta el primer registro. Se esperaba AL."
                                       : "El primer registro de ADD debe ser AL.";
                return r;
            }
            r.tokens.push_back({TipoToken::REGISTRO, reg1});
            r.instruccion.registros.push_back(reg1);

            saltarEspacios();
            if (i >= entrada.size() || entrada[i] != ',') {
                r.error = "Falta la coma entre AL y BL.";
                return r;
            }
            r.tokens.push_back({TipoToken::COMA, ","});
            ++i;

            saltarEspacios();
            string reg2 = leerPalabra();
            if (reg2 != "BL") {
                r.error = reg2.empty() ? "Falta el segundo registro. Se esperaba BL."
                                       : "El segundo registro de ADD debe ser BL.";
                return r;
            }
            r.tokens.push_back({TipoToken::REGISTRO, reg2});
            r.instruccion.registros.push_back(reg2);

            if (!soloEspaciosHastaFin()) {
                r.error = "Sobran componentes despues de ADD AL, BL.";
                return r;
            }

            r.valida = true;
            return r;
        }

        if (op == "STO") {
            r.tokens.push_back({TipoToken::STO, op});
            r.instruccion.operacion = "STO";

            if (i >= entrada.size() || entrada[i] != ' ') {
                r.error = "Se esperaba uno o mas espacios despues de STO.";
                return r;
            }

            saltarEspacios();
            if (i >= entrada.size() || !isdigit(static_cast<unsigned char>(entrada[i]))) {
                r.error = "Falta una direccion decimal no negativa despues de STO.";
                return r;
            }

            string numero = leerNumero();
            r.tokens.push_back({TipoToken::NUMERO, numero});
            try {
                r.instruccion.direccion = stoi(numero);
                r.instruccion.tieneDireccion = true;
            } catch (...) {
                r.error = "La direccion numerica es demasiado grande.";
                return r;
            }

            if (!soloEspaciosHastaFin()) {
                r.error = "Sobran componentes despues de la direccion.";
                return r;
            }

            r.valida = true;
            return r;
        }

        if (op == "END") {
            r.tokens.push_back({TipoToken::END_, op});
            r.instruccion.operacion = "END";
            if (!soloEspaciosHastaFin()) {
                r.error = "END no admite operandos adicionales.";
                return r;
            }
            r.valida = true;
            return r;
        }

        r.error = "Mnemonico invalido: " + op + ".";
        return r;
    }
};

// ============================================================
// MAQUINA DE MOORE
// ============================================================
class MaquinaMoore {
private:
    enum class Estado {
        MOV_PREPARAR_DIRECCION,
        MOV_LEER_MEMORIA,
        MOV_CARGAR_REGISTRO,
        ADD_SUMAR,
        STO_PREPARAR_DIRECCION,
        STO_CARGAR_MBR,
        STO_ESCRIBIR_MEMORIA,
        END_HALT,
        FIN
    };

    Instruccion inst;
    Estado estado = Estado::FIN;

public:
    explicit MaquinaMoore(const Instruccion& instruccion) : inst(instruccion) {
        if (inst.operacion == "MOV") estado = Estado::MOV_PREPARAR_DIRECCION;
        else if (inst.operacion == "ADD") estado = Estado::ADD_SUMAR;
        else if (inst.operacion == "STO") estado = Estado::STO_PREPARAR_DIRECCION;
        else if (inst.operacion == "END") estado = Estado::END_HALT;
    }

    bool terminada() const { return estado == Estado::FIN; }

    string siguientePaso() {
        string salida;
        switch (estado) {
            case Estado::MOV_PREPARAR_DIRECCION:
                salida = "MAR <- " + to_string(inst.direccion);
                estado = Estado::MOV_LEER_MEMORIA;
                break;
            case Estado::MOV_LEER_MEMORIA:
                salida = "MBR <- M[MAR]";
                estado = Estado::MOV_CARGAR_REGISTRO;
                break;
            case Estado::MOV_CARGAR_REGISTRO:
                salida = inst.registros[0] + " <- MBR";
                estado = Estado::FIN;
                break;
            case Estado::ADD_SUMAR:
                salida = "ACC <- AL + BL";
                estado = Estado::FIN;
                break;
            case Estado::STO_PREPARAR_DIRECCION:
                salida = "MAR <- " + to_string(inst.direccion);
                estado = Estado::STO_CARGAR_MBR;
                break;
            case Estado::STO_CARGAR_MBR:
                salida = "MBR <- ACC";
                estado = Estado::STO_ESCRIBIR_MEMORIA;
                break;
            case Estado::STO_ESCRIBIR_MEMORIA:
                salida = "M[MAR] <- MBR";
                estado = Estado::FIN;
                break;
            case Estado::END_HALT:
                salida = "HALT <- 1";
                estado = Estado::FIN;
                break;
            case Estado::FIN:
                break;
        }
        return salida;
    }
};

// ============================================================
// FORMATEO DE SALIDA
// ============================================================
string aCRLF(const string& s) {
    string out;
    out.reserve(s.size() + 32);
    for (char c : s) {
        if (c == '\n') out += "\r\n";
        else out += c;
    }
    return out;
}

string textoTokens(const vector<Token>& tokens) {
    ostringstream out;
    for (const auto& t : tokens) {
        out << nombreToken(t.tipo) << "(\"" << t.lexema << "\")\n";
    }
    return out.str();
}

string textoComponentes(const Instruccion& inst) {
    ostringstream out;
    out << "Mnemonico: " << inst.operacion << "\n";
    if (inst.operacion == "MOV") {
        out << "Registro destino: " << inst.registros[0] << "\n";
        out << "Direccion de memoria: " << inst.direccion << "\n";
        out << "Direccionamiento: directo\n";
    } else if (inst.operacion == "ADD") {
        out << "Primer registro: " << inst.registros[0] << "\n";
        out << "Segundo registro: " << inst.registros[1] << "\n";
    } else if (inst.operacion == "STO") {
        out << "Direccion de memoria: " << inst.direccion << "\n";
        out << "Direccionamiento: directo\n";
    } else if (inst.operacion == "END") {
        out << "Sin operandos.\n";
    }
    return out.str();
}

string textoObjeto(const Instruccion& inst) {
    ostringstream out;
    out << "{\n";
    out << "  operacion: \"" << inst.operacion << "\",\n";
    out << "  registros: [";
    for (size_t i = 0; i < inst.registros.size(); ++i) {
        out << "\"" << inst.registros[i] << "\"";
        if (i + 1 < inst.registros.size()) out << ", ";
    }
    out << "],\n";
    out << "  direccion: ";
    if (inst.tieneDireccion) out << inst.direccion << "\n";
    else out << "null\n";
    out << "}\n";
    return out.str();
}

string textoMicrooperaciones(const Instruccion& inst) {
    ostringstream out;
    MaquinaMoore moore(inst);
    int paso = 1;
    while (!moore.terminada()) {
        string micro = moore.siguientePaso();
        if (!micro.empty()) out << paso++ << ". " << micro << "\n";
    }
    out << "Generacion terminada.\n";
    return out.str();
}

string textoResumen(const string& entrada, const ResultadoAFD& r) {
    ostringstream out;
    out << "ENTRADA\n" << entrada << "\n\n";
    out << "VALIDACION MEDIANTE AFD\n";
    if (r.valida) {
        out << "Instruccion valida.\n";
    } else {
        out << "Instruccion invalida: " << r.error << "\n";
        out << "No se construye el objeto instruccion.\n";
        out << "No se generan microoperaciones.\n";
    }
    return out.str();
}

string textoPruebasPDF() {
    vector<string> pruebas = {
        "MOV AL, 6",
        "MOV BL, 7",
        "ADD AL, BL",
        "STO 8",
        "END",
        "MOV AX, 6",
        "MOV AL 6",
        "ADD AL,",
        "STO",
        "END 8"
    };

    AnalizadorAFD afd;
    ostringstream out;
    out << "PRUEBAS SOLICITADAS EN EL PDF\n";
    out << "========================================\n\n";

    for (size_t idx = 0; idx < pruebas.size(); ++idx) {
        auto r = afd.analizar(pruebas[idx]);
        out << "PRUEBA " << (idx + 1) << ": " << pruebas[idx] << "\n";
        out << "----------------------------------------\n";
        out << textoResumen(pruebas[idx], r);
        if (r.valida) {
            out << "TOKENS\n" << textoTokens(r.tokens) << "\n";
            out << "COMPONENTES\n" << textoComponentes(r.instruccion) << "\n";
            out << "OBJETO INSTRUCCION\n" << textoObjeto(r.instruccion) << "\n";
            out << "MICROOPERACIONES\n" << textoMicrooperaciones(r.instruccion);
        }
        out << "\n";
    }
    return out.str();
}

// ============================================================
// GUI
// ============================================================
#define ID_INPUT           101
#define ID_BTN_ANALIZAR    102
#define ID_BTN_LIMPIAR     103
#define ID_BTN_EJEMPLO     104
#define ID_BTN_PRUEBAS     105
#define ID_STATUS          106
#define ID_RESUMEN         107
#define ID_TOKENS          108
#define ID_COMPONENTES     109
#define ID_OBJETO          110
#define ID_MICROOPS        111
#define ID_HEADER_TITLE    112
#define ID_HEADER_SUB      113
#define ID_LABEL_HINT      114

HWND hInput, hStatus, hResumen, hTokens, hComponentes, hObjeto, hMicroops;
HFONT hFontUI = nullptr;
HFONT hFontTitle = nullptr;
HFONT hFontMono = nullptr;
COLORREF gStatusBg = RGB(230, 236, 245);
COLORREF gStatusText = RGB(28, 52, 84);
string gStatusMessage = "Esperando instruccion...";

void setText(HWND h, const string& value) {
    string t = aCRLF(value);
    SetWindowTextA(h, t.c_str());
}

void clearOutput() {
    setText(hStatus, "");
    setText(hResumen, "");
    setText(hTokens, "");
    setText(hComponentes, "");
    setText(hObjeto, "");
    setText(hMicroops, "");
    gStatusMessage = "Esperando instruccion...";
    gStatusBg = RGB(230, 236, 245);
    gStatusText = RGB(28, 52, 84);
}

void setStatusVisual(bool valid, const string& message) {
    gStatusMessage = message;
    if (valid) {
        gStatusBg = RGB(221, 245, 228);
        gStatusText = RGB(27, 94, 32);
    } else {
        gStatusBg = RGB(252, 228, 236);
        gStatusText = RGB(173, 20, 87);
    }
    setText(hStatus, message);
    InvalidateRect(GetParent(hStatus), nullptr, TRUE);
}

void analizarEntrada() {
    char buffer[512] = {0};
    GetWindowTextA(hInput, buffer, sizeof(buffer));
    string entrada = buffer;

    if (entrada.empty()) {
        MessageBoxA(GetParent(hInput), "Primero escribe una instruccion.", "Aviso", MB_OK | MB_ICONINFORMATION);
        return;
    }

    AnalizadorAFD afd;
    ResultadoAFD r = afd.analizar(entrada);

    setText(hResumen, textoResumen(entrada, r));

    if (!r.valida) {
        setStatusVisual(false, "Resultado: INSTRUCCION INVALIDA");
        setText(hTokens, "No se reconocio una secuencia completa valida.");
        setText(hComponentes, "No disponibles.");
        setText(hObjeto, "No se construye el objeto instruccion.");
        setText(hMicroops, "No se generan microoperaciones.");
        return;
    }

    setStatusVisual(true, "Resultado: INSTRUCCION VALIDA");
    setText(hTokens, textoTokens(r.tokens));
    setText(hComponentes, textoComponentes(r.instruccion));
    setText(hObjeto, textoObjeto(r.instruccion));
    setText(hMicroops, textoMicrooperaciones(r.instruccion));
}

void cargarEjemplo() {
    SetWindowTextA(hInput, "MOV AL, 6");
    analizarEntrada();
}

void mostrarPruebas() {
    setStatusVisual(true, "Mostrando reporte completo de pruebas del PDF");
    setText(hResumen, "Se ejecuto el lote de pruebas solicitado por el PDF.\nRevisa el reporte detallado en el panel inferior.");
    setText(hTokens, "Incluido dentro del reporte general.");
    setText(hComponentes, "Incluido dentro del reporte general.");
    setText(hObjeto, "Incluido dentro del reporte general.");
    setText(hMicroops, textoPruebasPDF());
}

void applyFont(HWND hwnd, HFONT font) {
    SendMessage(hwnd, WM_SETFONT, (WPARAM)font, TRUE);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            hFontUI = CreateFontA(-18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
                                  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                  FF_DONTCARE, "Segoe UI");
            hFontTitle = CreateFontA(-28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET,
                                     OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                     FF_DONTCARE, "Segoe UI");
            hFontMono = CreateFontA(-18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
                                    OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                    FIXED_PITCH | FF_MODERN, "Consolas");

            HWND hTitle = CreateWindowA("STATIC", "ANALISIS LEXICO Y AUTOMATAS",
                          WS_VISIBLE | WS_CHILD, 24, 17, 650, 38, hwnd, (HMENU)ID_HEADER_TITLE, nullptr, nullptr);

            HWND hLbl1 = CreateWindowA("STATIC", "Instruccion:", WS_VISIBLE | WS_CHILD,
                          24, 94, 92, 22, hwnd, nullptr, nullptr, nullptr);
            hInput = CreateWindowExA(0, "EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
                          122, 90, 320, 30, hwnd, (HMENU)ID_INPUT, nullptr, nullptr);

            HWND b1 = CreateWindowA("BUTTON", "Analizar", WS_VISIBLE | WS_CHILD,
                          460, 89, 110, 32, hwnd, (HMENU)ID_BTN_ANALIZAR, nullptr, nullptr);
            HWND b2 = CreateWindowA("BUTTON", "Limpiar", WS_VISIBLE | WS_CHILD,
                          582, 89, 100, 32, hwnd, (HMENU)ID_BTN_LIMPIAR, nullptr, nullptr);
            HWND b3 = CreateWindowA("BUTTON", "Ejemplo", WS_VISIBLE | WS_CHILD,
                          694, 89, 100, 32, hwnd, (HMENU)ID_BTN_EJEMPLO, nullptr, nullptr);
            HWND b4 = CreateWindowA("BUTTON", "Pruebas PDF", WS_VISIBLE | WS_CHILD,
                          806, 89, 118, 32, hwnd, (HMENU)ID_BTN_PRUEBAS, nullptr, nullptr);

            CreateWindowA("STATIC",
                          "Formatos admitidos: MOV AL, [direccion] | MOV BL, [direccion] | ADD AL, BL | STO [direccion] | END",
                          WS_VISIBLE | WS_CHILD, 24, 130, 900, 22, hwnd, (HMENU)ID_LABEL_HINT, nullptr, nullptr);

            CreateWindowA("BUTTON", "Estado de validacion", WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
                          24, 158, 900, 58, hwnd, nullptr, nullptr, nullptr);
            hStatus = CreateWindowExA(0, "EDIT", "", WS_VISIBLE | WS_CHILD | ES_READONLY | ES_CENTER,
                          42, 182, 864, 24, hwnd, (HMENU)ID_STATUS, nullptr, nullptr);

            CreateWindowA("BUTTON", "Resumen", WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
                          24, 226, 440, 132, hwnd, nullptr, nullptr, nullptr);
            hResumen = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                          WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
                          38, 250, 412, 94, hwnd, (HMENU)ID_RESUMEN, nullptr, nullptr);

            CreateWindowA("BUTTON", "Tokens reconocidos", WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
                          484, 226, 440, 132, hwnd, nullptr, nullptr, nullptr);
            hTokens = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                          WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
                          498, 250, 412, 94, hwnd, (HMENU)ID_TOKENS, nullptr, nullptr);

            CreateWindowA("BUTTON", "Componentes identificados", WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
                          24, 368, 440, 132, hwnd, nullptr, nullptr, nullptr);
            hComponentes = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                          WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
                          38, 392, 412, 94, hwnd, (HMENU)ID_COMPONENTES, nullptr, nullptr);

            CreateWindowA("BUTTON", "Objeto instruccion", WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
                          484, 368, 440, 132, hwnd, nullptr, nullptr, nullptr);
            hObjeto = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                          WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
                          498, 392, 412, 94, hwnd, (HMENU)ID_OBJETO, nullptr, nullptr);

            CreateWindowA("BUTTON", "Microoperaciones / Reporte completo", WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
                          24, 510, 900, 176, hwnd, nullptr, nullptr, nullptr);
            hMicroops = CreateWindowExA(
    WS_EX_CLIENTEDGE,
    "EDIT",
    "",
    WS_VISIBLE | WS_CHILD |
    ES_MULTILINE |
    ES_AUTOVSCROLL |
    ES_READONLY |
    WS_VSCROLL,
    38, 574, 872, 110,
    hwnd,
    (HMENU)ID_MICROOPS,
    nullptr,
    nullptr
);

SendMessageA(hMicroops, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(8, 8));
SendMessageA(hMicroops, EM_FMTLINES, TRUE, 0);


            applyFont(hTitle, hFontTitle);
            applyFont(hLbl1, hFontUI);
            applyFont(hInput, hFontUI);
            applyFont(b1, hFontUI);
            applyFont(b2, hFontUI);
            applyFont(b3, hFontUI);
            applyFont(b4, hFontUI);
            applyFont(hStatus, hFontUI);
            applyFont(hResumen, hFontMono);
            applyFont(hTokens, hFontMono);
            applyFont(hComponentes, hFontMono);
            applyFont(hObjeto, hFontMono);
            applyFont(hMicroops, hFontMono);

            clearOutput();
            setText(hStatus, "Esperando instruccion...");
            SetWindowTextA(hInput, "MOV AL, 6");
            return 0;
        }

        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case ID_BTN_ANALIZAR:
                    analizarEntrada();
                    return 0;
                case ID_BTN_LIMPIAR:
                    SetWindowTextA(hInput, "");
                    clearOutput();
                    return 0;
                case ID_BTN_EJEMPLO:
                    cargarEjemplo();
                    return 0;
                case ID_BTN_PRUEBAS:
                    mostrarPruebas();
                    return 0;
            }
            break;
        }

        case WM_CTLCOLORSTATIC: {
    HDC hdc = (HDC)wParam;
    HWND ctrl = (HWND)lParam;
    int id = GetDlgCtrlID(ctrl);

    static HBRUSH hHeaderBrush = CreateSolidBrush(RGB(35, 64, 107));
    static HBRUSH hBgBrush = CreateSolidBrush(RGB(245, 247, 251));
    static HBRUSH hEditBrush = CreateSolidBrush(RGB(255, 255, 255));

    // Titulo
    if (id == ID_HEADER_TITLE) {
        SetTextColor(hdc, RGB(255, 255, 255));
        SetBkColor(hdc, RGB(35, 64, 107));
        SetBkMode(hdc, OPAQUE);
        return (INT_PTR)hHeaderBrush;
    }

    // Estado de validacion
    if (id == ID_STATUS) {
        SetTextColor(hdc, gStatusText);
        SetBkColor(hdc, gStatusBg);
        SetBkMode(hdc, OPAQUE);

        static HBRUSH hStatusBrush = nullptr;

        if (hStatusBrush)
            DeleteObject(hStatusBrush);

        hStatusBrush = CreateSolidBrush(gStatusBg);

        return (INT_PTR)hStatusBrush;
    }

    // CUADROS DE TEXTO DE SOLO LECTURA
    if (id == ID_RESUMEN ||
        id == ID_TOKENS ||
        id == ID_COMPONENTES ||
        id == ID_OBJETO ||
        id == ID_MICROOPS) {

        SetTextColor(hdc, RGB(25, 25, 25));

        // IMPORTANTE:
        // El fondo debe ser OPACO para que Windows
        // borre el texto anterior al hacer scroll.
        SetBkColor(hdc, RGB(255, 255, 255));
        SetBkMode(hdc, OPAQUE);

        return (INT_PTR)hEditBrush;
    }

    // Resto de controles
    SetTextColor(hdc, RGB(20, 20, 20));
    SetBkColor(hdc, RGB(245, 247, 251));
    SetBkMode(hdc, OPAQUE);

    return (INT_PTR)hBgBrush;
}

        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam;
            HWND ctrl = (HWND)lParam;
            int id = GetDlgCtrlID(ctrl);

            static HBRUSH hReadOnlyBrush = CreateSolidBrush(RGB(255, 255, 255));
            static HBRUSH hStatusBrush2 = nullptr;

            if (id == ID_STATUS) {
                if (hStatusBrush2) DeleteObject(hStatusBrush2);
                hStatusBrush2 = CreateSolidBrush(gStatusBg);
                SetTextColor(hdc, gStatusText);
                SetBkColor(hdc, gStatusBg);
                return (INT_PTR)hStatusBrush2;
            }

            SetTextColor(hdc, RGB(36, 36, 36));
            SetBkColor(hdc, RGB(255, 255, 255));
            return (INT_PTR)hReadOnlyBrush;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rc;
            GetClientRect(hwnd, &rc);

            HBRUSH bg = CreateSolidBrush(RGB(245, 247, 251));
            FillRect(hdc, &rc, bg);
            DeleteObject(bg);

            RECT header = {0, 0, rc.right, 70};
            HBRUSH headerBrush = CreateSolidBrush(RGB(35, 64, 107));
            FillRect(hdc, &header, headerBrush);
            DeleteObject(headerBrush);

            RECT accent = {0, 70, rc.right, 74};
            HBRUSH accentBrush = CreateSolidBrush(RGB(78, 115, 223));
            FillRect(hdc, &accent, accentBrush);
            DeleteObject(accentBrush);

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_DESTROY:
            if (hFontUI) DeleteObject(hFontUI);
            if (hFontTitle) DeleteObject(hFontTitle);
            if (hFontMono) DeleteObject(hFontMono);
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    const char CLASS_NAME[] = "AFD_MOORE_PRO_GUI";

    WNDCLASSA wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(
        0,
        CLASS_NAME,
        "ProyectoAFD",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, 970, 760,
        nullptr, nullptr, hInstance, nullptr
    );

    if (!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessageA(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return 0;
}
