#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <string>
using namespace std;

struct VaultHeader {
   char magic[4];
   char passwordHash[64];
   int fileCount;
};
struct FileEntry {
   char nombre[32];
   long long tamano;
   long long offset;
};

void crearBoveda(string rutaBoveda, string password) {
   ofstream vault(rutaBoveda, ios::binary);
   if (!vault) {
       cout << "Error al crear la boveda." << endl;
       return;
   }
   VaultHeader header;
   memcpy(header.magic, "VLT1", 4);
   strncpy(header.passwordHash, password.c_str(), 63);
   header.fileCount = 0;
   vault.write(reinterpret_cast<char*>(&header), sizeof(VaultHeader));
   vault.close();
   cout << ">> Boveda '" << rutaBoveda << "' creada correctamente." << endl;
}
void insertarArchivo(string rutaBoveda, string rutaExterno) {
   ifstream archivoEntrada(rutaExterno, ios::binary | ios::ate);
   if (!archivoEntrada) {
       cout << "Error: No se encontro el archivo '" << rutaExterno << "'" << endl;
       return;
   }
   long long tamanoReal = archivoEntrada.tellg();
   archivoEntrada.seekg(0, ios::beg);
   fstream boveda(rutaBoveda, ios::binary | ios::in | ios::out);
   if (!boveda) return;
   VaultHeader header;
   boveda.read(reinterpret_cast<char*>(&header), sizeof(VaultHeader));
   boveda.seekp(0, ios::end);
   FileEntry nuevaEntrada;
   strncpy(nuevaEntrada.nombre, rutaExterno.c_str(), 31);
   nuevaEntrada.tamano = tamanoReal;
   nuevaEntrada.offset = (long long)boveda.tellp() + sizeof(FileEntry);
   boveda.write(reinterpret_cast<char*>(&nuevaEntrada), sizeof(FileEntry));
   vector<char> buffer(tamanoReal);
   archivoEntrada.read(buffer.data(), tamanoReal);
   boveda.write(buffer.data(), tamanoReal);
   header.fileCount++;
   boveda.seekp(0, ios::beg);
   boveda.write(reinterpret_cast<char*>(&header), sizeof(VaultHeader));
   boveda.close();
   archivoEntrada.close();
   cout << ">> '" << rutaExterno << "' guardado con exito." << endl;
}


void listarContenido(string rutaBoveda) {
   ifstream boveda(rutaBoveda, ios::binary);
   if (!boveda) {
       cout << "Error: No se pudo abrir la boveda." << endl;
       return;
   }
   VaultHeader header;
   boveda.read(reinterpret_cast<char*>(&header), sizeof(VaultHeader));
   cout << "\n=== CONTENIDO: " << header.fileCount << " ARCHIVOS ===" << endl;
   for (int i = 0; i < header.fileCount; i++) {
       FileEntry entrada;
       boveda.read(reinterpret_cast<char*>(&entrada), sizeof(FileEntry));
       cout << "[" << i + 1 << "] " << entrada.nombre << " | " << entrada.tamano << " bytes" << endl;
       boveda.seekg(entrada.tamano, ios::cur);
   }
   cout << "======================================\n" << endl;
   boveda.close();
}

void extraerArchivo(string rutaBoveda, string nombreABuscar) {
   ifstream boveda(rutaBoveda, ios::binary);
   if (!boveda) return;
   VaultHeader header;
   boveda.read(reinterpret_cast<char*>(&header), sizeof(VaultHeader));
   for (int i = 0; i < header.fileCount; i++) {
       FileEntry entrada;
       boveda.read(reinterpret_cast<char*>(&entrada), sizeof(FileEntry));
       if (string(entrada.nombre) == nombreABuscar) {
           boveda.seekg(entrada.offset, ios::beg);
           string nombreSalida = "extraido_" + string(entrada.nombre);
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


int main() {
   int opcion;
   string bovedaNombre = "mi_boveda.bin";
   do {
       cout << "\n--- MENU BOVEDA SEGURA ---" << endl;
       cout << "1. Crear Boveda\n2. Insertar Archivo\n3. Listar Contenido\n4. Sacar un archivo\n5. Salir\nSeleccione: ";
       cin >> opcion;
       if (opcion == 1) {
           string p; cout << "Defina password: "; cin >> p;
           crearBoveda(bovedaNombre, p);
       }
       else if (opcion == 2) {
           string arch; cout << "Nombre del archivo: "; cin >> arch;
           insertarArchivo(bovedaNombre, arch);
       }
       else if (opcion == 3) {
           listarContenido(bovedaNombre);
       }
       else if (opcion == 4) {
           string arch; cout << "Nombre del archivo a sacar: "; cin >> arch;
           extraerArchivo(bovedaNombre, arch);
       }
   } while (opcion != 5);
   return 0;
}

