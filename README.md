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

### Create the Homomorphic Encryption context
```text
var hc = seal.generateHomomorficContext();
```
or
```text
var hc = new seal.HomomorphicContext();
```

### Invoke various methods of a HomomorphicContext object
GET/SET:
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

### Notes
The SEAL's `PublicKey`, `SecretKey` and `Ciphertext` instances are serialized to/from the JS environment as Base64 encoded strings.