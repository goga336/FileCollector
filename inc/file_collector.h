#include <cstdint>
#include <vector>
using namespace std;

class FileCollectotr{
    private:

    public:
    void CollectFile(uint32_t field, size_t fileSize);
    void OnNewChunk(uint32_t field, size_t pos, std::vector<uint8_t> &&chunk);
};