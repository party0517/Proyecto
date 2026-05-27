#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <string>
#include <conio.h>
#include <stdlib.h>
#include <cstdlib>
using namespace std;

struct VaultHeader
{
    char magic[4];
    char password[64];
    int fileCount;
};

struct FileEntry
{
    char nombre[32];
    long long tamano;
    long long offset;
};

bool desencriptarBoveda(string contrasena)
{
    system("openssl enc -base64 -d -in boveda_segura -out boveda_decodificada");
    string comando = "openssl enc -aes-256-cbc -d -pbkdf2 -in boveda_decodificada -out mi_boveda.bin -pass pass:" + contrasena;
    int resultado = system(comando.c_str());
    system("del boveda_decodificada 2>nul");
    
    if (resultado != 0)
    {
        system("del mi_boveda.bin 2>nul");
        return false;
    }
    system("del boveda_segura 2>nul");
    return true;
}

void encriptarBoveda(string contrasena)
{
    string comando = "openssl enc -aes-256-cbc -pbkdf2 -in mi_boveda.bin -out boveda_encriptada -pass pass:" + contrasena;
    system(comando.c_str());
    system("del mi_boveda.bin");
    system("openssl enc -base64 -in boveda_encriptada -out boveda_segura");
    system("del boveda_encriptada");
}
string contrasena()
{
    char pass[32];
    int i = 0;
    char a;
    for (;;)
    {
        a = getch(); 
        if (((a >= 'a' && a <= 'z') || (a >= 'A' && a <= 'Z') || (a >= '0' && a <= '9')) && i < 31)
        {
            pass[i] = a;
            ++i;
            cout << "*";
        }
        else if (a == '\b' && i >= 1) 
        {
            cout << "\b \b"; 
            --i;
        }
        else if (a == '\r') 
        {
            pass[i] = '\0'; 
            break;
        }
    }
    return string(pass);
}

void crearBoveda(string rutaBoveda, string password)
{
    ofstream vault(rutaBoveda, ios::binary);
    if (!vault)
    {
        cout << "Error al crear la boveda." << endl;
        return;
    }
    VaultHeader header;
    memcpy(header.magic, "VLT1", 4);
    strncpy(header.password, password.c_str(), 63);
    header.fileCount = 0;
    vault.write(reinterpret_cast<char *>(&header), sizeof(VaultHeader));
    vault.close();
    cout << ">> Boveda '" << rutaBoveda << "' creada correctamente." << endl;
}

void insertarArchivo(string rutaBoveda, string rutaExterno)
{
    ifstream archivoEntrada(rutaExterno, ios::binary | ios::ate);
    if (!archivoEntrada)
    {
        cout << "Error: No se encontro el archivo '" << rutaExterno << "'" << endl;
        return;
    }
    long long tamanoReal = archivoEntrada.tellg();
    archivoEntrada.seekg(0, ios::beg);
    fstream boveda(rutaBoveda, ios::binary | ios::in | ios::out);
    if (!boveda)
        return;
    VaultHeader header;
    boveda.read(reinterpret_cast<char *>(&header), sizeof(VaultHeader));
    boveda.seekp(0, ios::end);
    FileEntry nuevaEntrada;
    strncpy(nuevaEntrada.nombre, rutaExterno.c_str(), 31);
    nuevaEntrada.tamano = tamanoReal;
    nuevaEntrada.offset = (long long)boveda.tellp() + sizeof(FileEntry);
    boveda.write(reinterpret_cast<char *>(&nuevaEntrada), sizeof(FileEntry));
    vector<char> buffer(tamanoReal);
    archivoEntrada.read(buffer.data(), tamanoReal);
    boveda.write(buffer.data(), tamanoReal);
    header.fileCount++;
    boveda.seekp(0, ios::beg);
    boveda.write(reinterpret_cast<char *>(&header), sizeof(VaultHeader));
    boveda.close();
    archivoEntrada.close();
    if (remove(rutaExterno.c_str()) == 0)
    {
        cout << ">> '" << rutaExterno << "' movido a la boveda y eliminado de afuera." << endl;
    }
    else
    {
        cout << ">> Error: El archivo se guardo pero no se pudo borrar de afuera." << endl;
    }
}

void listarContenido(string rutaBoveda)
{
    ifstream boveda(rutaBoveda, ios::binary);
    if (!boveda)
    {
        cout << "Error: No se pudo abrir la boveda." << endl;
        return;
    }
    int conteo = 0;
    VaultHeader header;
    boveda.read(reinterpret_cast<char *>(&header), sizeof(VaultHeader));
    cout << "\n=== CONTENIDO: " << header.fileCount << " ARCHIVOS ===" << endl;
    for (int i = 0; i < header.fileCount; i++)
    {
        FileEntry entrada;
        boveda.read(reinterpret_cast<char *>(&entrada), sizeof(FileEntry));
        cout << "[" << i + 1 << "] " << entrada.nombre << " | " << entrada.tamano << " bytes" << endl;
        conteo += entrada.tamano;
        boveda.seekg(entrada.tamano, ios::cur);
    }
    cout <<"\nTamaño total de la bóveda: "<<conteo<< " bytes"<<endl;
    cout << "======================================\n"
         << endl;
    boveda.close();
}

void extraerArchivo(string rutaBoveda, string nombreABuscar)
{
    string opc;
    string nombreSalida;
    ifstream boveda(rutaBoveda, ios::binary);
    if (!boveda)
        return;
    VaultHeader header;
    boveda.read(reinterpret_cast<char *>(&header), sizeof(VaultHeader));
    for (int i = 0; i < header.fileCount; i++)
    {
        FileEntry entrada;
        boveda.read(reinterpret_cast<char *>(&entrada), sizeof(FileEntry));
        if (string(entrada.nombre) == nombreABuscar)
        {
            boveda.seekg(entrada.offset, ios::beg);
            cout<<"\n¿Quiéres extraer el archivo temporalmente? (s/n)";
            cin >>opc;
            if (opc=="s"){
                nombreSalida = "temp_" + string(entrada.nombre);
            }else{
                nombreSalida = string(entrada.nombre);
            }
            ofstream archivoSalida(nombreSalida, ios::binary);
            vector<char> buffer(entrada.tamano);
            boveda.read(buffer.data(), entrada.tamano);
            archivoSalida.write(buffer.data(), entrada.tamano);
            archivoSalida.close();
            cout << ">> Archivo extraido como: " << nombreSalida << endl;
            return;
        }
        boveda.seekg(entrada.tamano, ios::cur);
    }
    cout << "Error: Archivo no encontrado." << endl;
}

bool bovedaExiste(string nombre)
{
    ifstream f(nombre.c_str());
    return f.good();
}

bool validarPassword(string ruta, string intento)
{
    ifstream boveda(ruta, ios::binary);
    if (!boveda)
        return false;

    VaultHeader header;
    boveda.read(reinterpret_cast<char *>(&header), sizeof(VaultHeader));
    boveda.close();

    return (string(header.password) == intento);
}

bool archivoDuplicado(string rutaBoveda, string nombreBuscar)
{
    ifstream boveda(rutaBoveda, ios::binary);
    if (!boveda)
        return false;

    VaultHeader header;
    boveda.read(reinterpret_cast<char *>(&header), sizeof(VaultHeader));

    for (int i = 0; i < header.fileCount; i++)
    {
        FileEntry entrada;
        boveda.read(reinterpret_cast<char *>(&entrada), sizeof(FileEntry));
        if (string(entrada.nombre) == nombreBuscar)
        {
            boveda.close();
            return true;
        }
        boveda.seekg(entrada.tamano, ios::cur);
    }
    boveda.close();
    return false;
}



int main()
{
    setlocale(LC_ALL, "");
    int opcion;
    string seguridad;
    string bovedaNombre = "mi_boveda.bin";
    string contrasena_encriptar;
    bool existe = false;

    if (!bovedaExiste("boveda_segura"))
    {
        cout << "No se encontro boveda. Vamos a crear una nueva." << endl;
        string p;
        cout << "INgrese la contraseña para la nueva boveda: ";

        p = contrasena();
        crearBoveda(bovedaNombre, p);
    }
    else
    {
        existe = true;
        cout << "Boveda detectada. Ingrese la contraseña para desencriptarla: ";
        contrasena_encriptar = contrasena();
        desencriptarBoveda(contrasena_encriptar);
        if (!bovedaExiste(bovedaNombre))
        {
            cout << "\nLa contraseña de OpenSSL es incorrecta. No se pudo descifrar el archivo." << endl;
            return 0;
        }
        string intento;
        cout << "\nAhora ingrese la contraseña para ingresar a la boveda: ";
        intento = contrasena();

        if (!validarPassword(bovedaNombre, intento))
        {
            cout << "Contraseña incorrecta. Acceso denegado." << endl;
            encriptarBoveda(contrasena_encriptar);
            return 0;
        }
        cout << "\nAcceso concedido." << endl;
    }
    do
    {
        cout << "\n--- MENU BOVEDA SEGURA ---" << endl;
        cout << "1. Insertar Archivo\n2. Listar Contenido\n3. Sacar un archivo\n4. Eliminar bóveda\n5. Salir\nSeleccione: ";
        cin >> opcion;
        if (opcion == 1)
        {
            string arch;
            cout << "Nombre del archivo: ";
            cin >> arch;
            if (archivoDuplicado(bovedaNombre, arch))
            {
                cout << "Ya existe un archivo con ese nombre en la bóveda. Cámbiale el nombre afuera primero." << endl;
            }
            else
            {
                insertarArchivo(bovedaNombre, arch);
            }
        }
        else if (opcion == 2)
        {
            listarContenido(bovedaNombre);
        }
        else if (opcion == 3)
        {
            string arch;
            cout << "Nombre del archivo a sacar: ";
            cin >> arch;
            extraerArchivo(bovedaNombre, arch);
        }
        else if (opcion == 4)
        {
            cout << "¿Estás seguro de que quiere borrar tu bóveda? (s/n)" << endl;
            cin >> seguridad;
            if (seguridad == "s")
            {
                cout << "Se perderán todos los archivos de su boveda.";
                cout << "\n¿Está seguro de que desea continuar? (s/n)" << endl;
                cin >> seguridad;
                if (seguridad == "s")
                {
                    if (remove(bovedaNombre.c_str()) == 0)
                    {
                        cout << "Se eliminó la boveda" << endl;
                        return 0;
                    }
                    else
                    {
                        cout << ">> Error no se pudo boorrar la bóveda" << endl;
                    }
                }
            }
        }
    } while (opcion != 5);
    system("del temp_* 2>nul");
    if (existe)
    {
        encriptarBoveda(contrasena_encriptar);
    }
    else
    {
        cout << "Muy bien antes de irte ingrese la contraseña para encriptar su boveda: ";
        contrasena_encriptar = contrasena();
        encriptarBoveda(contrasena_encriptar);
        cout << "Muy bien sus archivos están protegidos.";
    }
    return 0;
}
