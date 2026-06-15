#include <iostream>
#include <cstring>
#include <string>

const char *SHM_NAME = "MinhaMemoriaCompartilhada";
const size_t SHM_SIZE = 1024;

#if defined(_WIN32) || defined(_WIN64)
// --- CÓDIGO PARA WINDOWS ---
#include <windows.h>

int main()
{
    std::cout << "[Consumidor - Windows] Conectando a memoria compartilhada..." << std::endl;

    HANDLE hMapFile = OpenFileMappingA(FILE_MAP_READ, FALSE, SHM_NAME);
    if (hMapFile == NULL)
    {
        std::cerr << "Erro ao abrir o mapeamento (Verifique se o Produtor esta rodando!). Erro: "
                  << GetLastError() << std::endl;
        return 1;
    }

    char *pBuf = (char *)MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, SHM_SIZE);
    if (pBuf == NULL)
    {
        std::cerr << "Erro ao mapear a visao do arquivo (" << GetLastError() << ")." << std::endl;
        CloseHandle(hMapFile);
        return 1;
    }

    std::cout << "[Consumidor] Lendo a memoria em tempo real (Pressione CTRL+C para sair)..." << std::endl;
    std::string ultima_mensagem = "";

    while (true)
    {
        if (std::string(pBuf) != ultima_mensagem)
        {
            std::cout << "--> Nova mensagem detectada: " << pBuf << std::endl;
            ultima_mensagem = pBuf;
        }
        Sleep(500); 
    }

    UnmapViewOfFile(pBuf);
    CloseHandle(hMapFile);
    return 0;
}

#else
// --- CÓDIGO PARA LINUX / MAC OS (POSIX) ---
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
    std::cout << "[Consumidor - Linux/macOS] Conectando a memoria compartilhada..." << std::endl;

    std::string linux_shm_name = "/" + std::string(SHM_NAME);

    int shm_fd = shm_open(linux_shm_name.c_str(), O_RDONLY, 00666);
    if (shm_fd == -1)
    {
        std::perror("Erro ao executar shm_open (O Produtor esta rodando?)");
        return 1;
    }

    char *pBuf = (char *)mmap(0, SHM_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (pBuf == MAP_FAILED)
    {
        std::perror("Erro ao executar mmap no Consumidor");
        close(shm_fd);
        return 1;
    }

    std::cout << "[Consumidor] Lendo a memoria em tempo real (Pressione CTRL+C para sair)..." << std::endl;
    std::string ultima_mensagem = "";

    while (true)
    {
        if (std::string(pBuf) != ultima_mensagem)
        {
            std::cout << "--> Nova mensagem detectada: " << pBuf << std::endl;
            ultima_mensagem = pBuf;
        }
        usleep(500000); 
    }

    munmap(pBuf, SHM_SIZE);
    close(shm_fd);
    return 0;
}
#endif