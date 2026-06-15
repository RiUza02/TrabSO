#include <iostream>
#include <cstring>
#include <string>

const char* SHM_NAME = "MinhaMemoriaCompartilhada";
const size_t SHM_SIZE = 1024;

#if defined(_WIN32) || defined(_WIN64)
    // --- CÓDIGO PARA WINDOWS ---
    #include <windows.h>
    
    int main() {
        std::cout << "[Produtor - Windows] Criando memoria compartilhada..." << std::endl;

        HANDLE hMapFile = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, SHM_SIZE, SHM_NAME);
        if (hMapFile == NULL) {
            std::cerr << "Erro ao criar mapeamento de arquivo (" << GetLastError() << ")." << std::endl;
            return 1;
        }

        char* pBuf = (char*) MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, SHM_SIZE);
        if (pBuf == NULL) {
            std::cerr << "Erro ao mapear a visao do arquivo (" << GetLastError() << ")." << std::endl;
            CloseHandle(hMapFile);
            return 1;
        }   

        // --- LOOP DE INTERAÇÃO COM O USUÁRIO ---
        std::string entrada;
        std::cout << "Memoria compartilhada pronta!" << std::endl;
        
        while (true) {
            std::cout << "Digite uma mensagem (ou 'sair' para encerrar): ";
            std::getline(std::cin, entrada);

            if (entrada == "sair") {
                break;
            }

            // Copia a mensagem digitada para a memória (com segurança para não estourar o tamanho)
            std::strncpy(pBuf, entrada.c_str(), SHM_SIZE - 1);
            pBuf[SHM_SIZE - 1] = '\0'; // Garante o caractere nulo no fim
            
            std::cout << "[Produtor] Enviado com sucesso!\n" << std::endl;
        }

        // Limpeza de recursos
        UnmapViewOfFile(pBuf);
        CloseHandle(hMapFile);
        
        std::cout << "[Produtor] Memoria liberada. Programa encerrado." << std::endl;
        return 0;
    }

#else
    // --- CÓDIGO PARA LINUX / POSIX ---
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <unistd.h>

    int main() {
        std::cout << "[Produtor - Linux] Criando memoria compartilhada..." << std::endl;

        std::string linux_shm_name = "/" + std::string(SHM_NAME);
        int shm_fd = shm_open(linux_shm_name.c_str(), O_CREAT | O_RDWR, 0666);
        if (shm_fd == -1) {
            std::perror("Erro ao executar shm_open");
            return 1;
        }

        if (ftruncate(shm_fd, SHM_SIZE) == -1) {
            std::perror("Erro ao executar ftruncate");
            close(shm_fd);
            return 1;
        }

        char* pBuf = (char*) mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if (pBuf == MAP_FAILED) {
            std::perror("Erro ao executar mmap");
            close(shm_fd);
            return 1;
        }

        // --- LOOP DE INTERAÇÃO COM O USUÁRIO ---
        std::string entrada;
        std::cout << "Memoria compartilhada pronta!" << std::endl;

        while (true) {
            std::cout << "Digite uma mensagem (ou 'sair' para encerrar): ";
            std::getline(std::cin, entrada);

            if (entrada == "sair") {
                break;
            }

            // Copia a mensagem digitada para a memória
            std::strncpy(pBuf, entrada.c_str(), SHM_SIZE - 1);
            pBuf[SHM_SIZE - 1] = '\0';
            
            std::cout << "[Produtor] Enviado com sucesso!\n" << std::endl;
        }

        // Limpeza de recursos
        munmap(pBuf, SHM_SIZE);
        close(shm_fd);
        shm_unlink(linux_shm_name.c_str()); 

        std::cout << "[Produtor] Memoria liberada. Programa encerrado." << std::endl;
        return 0;
    }
#endif