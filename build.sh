set -e

if [ -d "./build" ]; then
    echo "Build directory exists. Not creating..."
else
    echo "Creating build/ directory"
    mkdir build
fi

cmake -B build -S . -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

if [ -L "./compile_commands.json" ]; then
  echo "Compile Commands JSON present, not symlinking"
else
  echo "Symlinking Compile Commands JSON"
  ln -s ./build/compile_commands.json ./
fi

cmake --build build -j 4
