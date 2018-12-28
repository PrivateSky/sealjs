# sealjs
Node.js port of SEAL C++ package released by Microsoft and containing homomorphic encryption primitives

## Building and running

Compile and build:
```text
$ npm install
```

Then run a JS sample similar to `example_bfv_basics_i()` from [SEAL Examples](https://github.com/Microsoft/SEAL/tree/master/examples) :
```text
$ npm run demo
```

or see our API in action:
```text
$ npm start
```

## How to use

```text
var seal = require('bindings')('nodeseal');
```

### 1. Create the homomorphic encryption context
The next statements create 4 contexts having the same encryption parameters (the scheme is 'BFV' by default):
```text
var hc1 = seal.generateHomomorficContext();
var hc2 = new seal.HomomorphicContext();
var hc3 = new seal.HomomorphicContext(2048, 'coeff_modulus_128', 1<<8);
var hc4 = new seal.HomomorphicContext( hc3.getEncryptionParameters() );
```

The three parameters for `hc3` are:
- `poly_modulus_degree`: Number - one of the values: `1024`, `2048`, `4096`, `8192`, `16384`, or `32768`
- `coeff_modulus`: String - one of the values: `"coeff_modulus_128"`, `"coeff_modulus_192"`, or `"coeff_modulus_256"`
- `plain_modulus`: UINT64

### 2. Invoke various methods of a HomomorphicContext object
GET/SET:
- `getEncryptionParameters()` //no setter; use HomomorphicContext(parms) instead
- `getPublicKey()`
- `setPublicKey()`
- `getSecretKey()`
- `setSecretKey()`

INT32 <=> `Ciphertext` converters:
- `encrypt()`
- `decrypt()`

Homomorphic arithmetic:
- `negate()`
- `add()`
- `sub()`
- `multiply()`
- `square()`

## Notes
The SEAL's `EncryptionParameters`, `PublicKey`, `SecretKey` and `Ciphertext` objects are serialized to/from the JS environment as Base64 encoded strings.