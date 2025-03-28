function build()
{
    cmake -B build
    cmake --build build --parallel
}

function run()
{
    ./bin/future
}