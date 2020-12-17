#include "net/transport.h"

int main(int argc, char *argv[])
{
    NT_TRY
    json j;
    for (const auto &apt : adaptor::all()) {
        j.push_back(apt.to_json());
    }
    spdlog::info(j.dump(3));
    NT_CATCH
}
