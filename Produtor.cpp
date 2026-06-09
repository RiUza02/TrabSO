#include <iostream>
#include <cstring>

// Definição do nome da memória e do tamanho
const char* SHM_NAME = "MinhaMemoriaCompartilhada";
const size_t SHM_SIZE = 1024;
const char* MENSAGEM = "Ola! Dados vindos do Produtor cross-platform.";

#if defined(_WIN32) || defined(_WIN64)
    // --- CÓDIGO PARA WINDOWS ---
    #include <windows.h>
    
    int main() {
        std::cout << "[Produtor - Windows] Criando memoria compartilhada..." << std::endl;

        // 1. Criar o mapeamento de arquivo na memória paginada do Windows
        HANDLE hMapFile = CreateFileMappingA(
            INVALID_HANDLE_VALUE,    // Usa o arquivo de paginação do sistema
            NULL,                    // Segurança padrão
            PAGE_READWRITE,          // Permissão de leitura e escrita
            0,                       // Tamanho máximo (High-order DWORD)
            SHM_SIZE,                // Tamanho máximo (Low-order DWORD)
            SHM_NAME                 // Nome do objeto de memória
        );

        if (hMapFile == NULL) {
            std::cerr << "Erro ao criar mapeamento de arquivo (" << GetLastError() << ")." << std::endl;
            return 1;
        }

        // 2. Mapear a visão do arquivo no espaço de endereço do processo
        char* pBuf = (char*) MapViewOfFile(
            hMapFile,            // Handle do objeto de mapeamento
            FILE_MAP_ALL_ACCESS, // Permissão de escrita/leitura
            0,
            0,
            SHM_SIZE
        );

        if (pBuf == NULL) {
            std::cerr << "Erro ao mapear a visao do arquivo (" << GetLastError() << ")." << std::endl;
            CloseHandle(hMapFile);
            return 1;
        }

        // 3. Escrever os dados na memória
        std::strcpy(pBuf, MENSAGEM);
        std::cout << "[Produtor] Dados gravados: " << pBuf << std::endl;

        std::cout << "Pressione ENTER para liberar a memoria e encerrar..." << std::endl;
        std::cin.get();

        // 4. Limpeza de recursos
        UnmapViewOfFile(pBuf);
        CloseHandle(hMapFile);
        
        std::cout << "[Produtor] Memoria liberada com sucesso." << std::endl;
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

        // 1. Criar o objeto de memória compartilhada POSIX
        // O nome no Linux deve começar com uma barra '/'
        std::string linux_shm_name = "/" + std::string(SHM_NAME);
        int shm_fd = shm_open(linux_shm_name.c_str(), O_CREAT | O_RDWR, 0666);
        if (shm_fd == -1) {
            std::perror("Erro ao executar shm_open");
            return 1;
        }

        // 2. Definir o tamanho do segmento de memória
        if (ftruncate(shm_fd, SHM_SIZE) == -1) {
            std::perror("Erro ao executar ftruncate");
            close(shm_fd);
            return 1;
        }

        // 3. Mapear a memória compartilhada no espaço do processo
        char* pBuf = (char*) mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if (pBuf == MAP_FAILED) {
            std::perror("Erro ao executar mmap");
            close(shm_fd);
            return 1;
        }

        // 4. Escrever os dados na memória
        std::strcpy(pBuf, MENSAGEM);
        std::cout << "[Produtor] Dados gravados: " << pBuf << std::endl;

        std::cout << "Pressione ENTER para liberar a memoria e encerrar..." << std::endl;
        std::cin.get();

        // 5. Limpeza de recursos
        munmap(pBuf, SHM_SIZE);
        close(shm_fd);
        shm_unlink(linux_shm_name.c_str()); // Remove o objeto do sistema

        std::cout << "[Produtor] Memoria liberada com sucesso." << std::endl;
        return 0;
    }
#endif