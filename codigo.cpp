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
   if (remove(rutaExterno.c_str()) == 0) {
       cout << ">> '" << rutaExterno << "' movido a la boveda y eliminado de afuera." << endl;
   } else {
       cout << ">> Error: El archivo se guardo pero no se pudo borrar de afuera." << endl;
   }
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
bool bovedaExiste(string nombre) {
   ifstream f(nombre.c_str());
   return f.good();
}


bool validarPassword(string ruta, string intento) {
   ifstream boveda(ruta, ios::binary);
   if (!boveda) return false;
  
   VaultHeader header;
   boveda.read(reinterpret_cast<char*>(&header), sizeof(VaultHeader));
   boveda.close();
  
   return (string(header.passwordHash) == intento);
}


int main() {
  int opcion;
  string seguridad;
  string bovedaNombre = "mi_boveda.bin";


  if (!bovedaExiste(bovedaNombre)) {
       cout << "No se encontro boveda. Vamos a crear una nueva." << endl;
       string p;
       cout << "Defina password para la nueva boveda: ";
       cin >> p;
       crearBoveda(bovedaNombre, p);
   } else {
       string intento;
       cout << "Boveda detectada. Ingrese la contraseña para entrar: ";
       cin >> intento;
      
       if (!validarPassword(bovedaNombre, intento)) {
           cout << "Contraseña incorrecta. Acceso denegado." << endl;
           return 0;
       }
       cout << "Acceso concedido." << endl;
   }




  do {
      cout << "\n--- MENU BOVEDA SEGURA ---" << endl;
      cout << "1. Insertar Archivo\n2. Listar Contenido\n3. Sacar un archivo\n4. Eliminar bóveda\n5. Salir\nSeleccione: ";
      cin >> opcion;
      if (opcion==1||opcion==2||opcion==3){
       string intento;
       cout << "Boveda detectada. Ingrese la contraseña para entrar: ";
       cin >> intento;
      
       if (!validarPassword(bovedaNombre, intento)) {
           cout << "Contraseña incorrecta. Acceso denegado." << endl;
           return 0;
       }
      }
      if (opcion == 1) {
          string arch;
          cout << "Nombre del archivo: "; cin >> arch;
          insertarArchivo(bovedaNombre, arch);
      }
      else if (opcion == 2) {
          listarContenido(bovedaNombre);
      }
      else if (opcion == 3) {
          string arch;
          cout << "Nombre del archivo a sacar: "; cin >> arch;
          extraerArchivo(bovedaNombre, arch);
      }
      else if (opcion == 4){
       cout<<"¿Estás seguro de que quiere borrar tu bóveda? (s/n)"<<endl;
       cin>>seguridad;
       if (seguridad=="s"){
           cout<<"Se perderán todos los archivos de su boveda.";
           cout<<"\n¿Está seguro de que desea continuar? (s/n)"<<endl;
           cin>>seguridad;
           if(seguridad=="s"){
               if (remove(bovedaNombre.c_str()) == 0) {
                   cout << "Se eliminó la boveda" << endl;
                   return 0;
                } else {
                   cout << ">> Error no se pudo boorrar la bóveda" << endl;
               }
           }
       }
      }
  } while (opcion != 5);




  return 0;
}

