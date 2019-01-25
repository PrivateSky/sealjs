
em++ --verbose "./seal/batchencoder.cpp" "./seal/biguint.cpp" "./seal/ciphertext.cpp" "./seal/ckks.cpp" ^
 "./seal/context.cpp" "./seal/decryptor.cpp" "./seal/encoder.cpp" "./seal/encryptionparams.cpp" ^
 "./seal/encryptor.cpp" "./seal/evaluator.cpp" "./seal/galoiskeys.cpp" "./seal/keygenerator.cpp" ^
 "./seal/memorymanager.cpp" "./seal/plaintext.cpp" "./seal/randomgen.cpp" "./seal/relinkeys.cpp" ^
 "./seal/smallmodulus.cpp" "./seal/util/aes.cpp" "./seal/util/baseconverter.cpp" "./seal/util/clipnormal.cpp" ^
 "./seal/util/globals.cpp" "./seal/util/hash.cpp" "./seal/util/mempool.cpp" "./seal/util/numth.cpp" ^
 "./seal/util/polyarith.cpp" "./seal/util/polyarithmod.cpp" "./seal/util/polyarithsmallmod.cpp" "./seal/util/smallntt.cpp" ^
 "./seal/util/uintarith.cpp" "./seal/util/uintarithmod.cpp" "./seal/util/uintarithsmallmod.cpp" "./seal/util/uintcore.cpp" ^
 "./src/base64.cpp" ^
 --bind "./src/homomorphic_context.cpp" -O0 -D _DISABLE_EXTENDED_ALIGNED_STORAGE -I./ -std=c++17 -m64 ^
 -o "./seal.js" ^
 -s WASM=1 -s NO_EXIT_RUNTIME=1 -s ALLOW_MEMORY_GROWTH=1 -s DISABLE_EXCEPTION_CATCHING=0 -s SINGLE_FILE=1
