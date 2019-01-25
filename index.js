/********************************************************************************************************************************
 * Copy-pasted from "my_seal_test.js"
 */

function mySealTest(seal) {

  //string pool:
  const RESULT_OK = "\t SUCCESS - same results";
  const RESULT_WRONG = "\t E R R O R  - the 2 results differ!!";
  const NEVER_THROWN_EXCEPTION = "THIS EXCEPTION SHOULD NEVER BE THROWN";

  const log = obj => console.log(String(obj));
  const logOK = (strPrefix, valRet, valExpected) => log(strPrefix + valRet + " " + (typeof valRet) + " " + ((valExpected === valRet) ? RESULT_OK : RESULT_WRONG));

  // INT 32: [-2147483648; 2147483647]
  const MIN_INT_32 = -2147483648;
  const MAX_INT_32 = 2147483647;

  let val1 = 5, val2 = -7;

  log("val1 = " + val1 + ", val2 = " + val2);

  try {

    let hc = seal.generateHomomorficContext();  //factory method

    let publicKey = hc.getPublicKey();
    let secretKey = hc.getSecretKey();

    let cipher1 = hc.encrypt(val1);
    let cipher2 = hc.encrypt(val2);

    //log(cipher1); //huuuuuuge Base 64 encoded string

    ////////////////////////////////////////////////////////////////////////////////
    //
    //TEST: decrypt()
    //

    //Test 1.1: SHOULD PASS
    {
      let v1 = hc.decrypt(cipher1);       //reverse operation
      logOK("decrypt - SAME CONTEXT:  ", v1, val1);
    }

    //Test 1.2: SHOULD FAIL - hcOther doesn't know the original PrivateKey
    try {
      let hcOther = new seal.HomomorphicContext();
      let v1 = hcOther.decrypt(cipher1);  //!!! SEAL WILL THROW A C++ EXCEPTION: "output out of range" !!!
      throw NEVER_THROWN_EXCEPTION; //never
    } catch (err) {
      log("decrypt - OTHER CONTEXT: \t\t Caught EXPECTED Exception: " + err);
    }

    //Test 1.3: SHOULD FAIL - hcOther knows the original PublicKey only, not the original PrivateKey
    try {
      let hcOther = new seal.HomomorphicContext();
      hcOther.setPublicKey(publicKey);
      let v1 = hcOther.decrypt(cipher1);  //!!! SEAL WILL THROW A C++ EXCEPTION: "output out of range" !!!
      throw NEVER_THROWN_EXCEPTION; //never
    } catch (err) {
      log("decrypt - OTHER CONTEXT: \t\t Caught EXPECTED Exception: " + err);
    }

    //Test 1.4: SHOULD PASS - hcOther knows the original PrivateKey (PublicKey isn't necessary for decription)
    {
      let hcOther = new seal.HomomorphicContext();
      hcOther.setSecretKey(secretKey);    // mandatory for decryption
      let v1 = hcOther.decrypt(cipher1);  // NOW IT'S OK
      logOK("decrypt - OTHER CONTEXT: ", v1, val1);
    }


    ////////////////////////////////////////////////////////////////////////////////
    //
    //TEST add()
    //

    //Test 2.1: SHOULD PASS
    {
      let cipherRes = hc.add(cipher1, cipher2);
      let vRet = hc.decrypt(cipherRes);
      let valExpected = val1 + val2;
      logOK("add     - SAME CONTEXT:  ", vRet, valExpected);
    }

    //Test 2.2: SHOULD PASS - SUM in other context, but with the same EncriptionParameters as for the original context!
    {
      let hcOther = new seal.HomomorphicContext();
      let cipherRes = hcOther.add(cipher1, cipher2);
      let vRet = hc.decrypt(cipherRes); //Decription in the original context!
      let valExpected = val1 + val2;
      logOK("add     - OTHER CONTEXT: ", vRet, valExpected);
    }

    //Test 2.3: SHOULD FAIL @decrypt - val2 encripted in other context; the 2 contexts use 2 different PublicKey(s)
    try {
      let hcOther = new seal.HomomorphicContext();
      let cipher2 = hcOther.encrypt(val2);
      let cipherRes = hcOther.add(cipher1, cipher2);  //or hc.add(cipher1, cipher2)
      let vRet = hc.decrypt(cipherRes);               //!!! SEAL WILL THROW A C++ EXCEPTION: "output out of range" !!!
      throw NEVER_THROWN_EXCEPTION; //never
    } catch (err) {
      log("add     - OTHER CONTEXT: \t\t Caught EXPECTED Exception: " + err);
    }

    //Test 2.4: SHOULD PASS
    {
      let hc2 = new seal.HomomorphicContext();
      hc2.setPublicKey(publicKey);                  //YES, both operands are encrypted with the same PublicKey!
      let cipher2 = hc2.encrypt(val2);

      let hc3 = seal.generateHomomorficContext();   //factory method
      let cipherRes = hc3.add(cipher1, cipher2);

      let vRet = hc.decrypt(cipherRes);             //YES, only the original context owns the PrivateKey!
      //let vRet = hc2.decrypt(cipherRes);          //<== SO THAT THIS LINE WILL THROW "output out of range" EXCEPTION!!!
      let valExpected = val1 + val2;
      logOK("add     - MANY CONTEXTS: ", vRet, valExpected);
    }

    //Test 2.5: SHOULD PASS
    {
      let cipherRes = hc.add(hc.encrypt(MIN_INT_32), hc.encrypt(MAX_INT_32));
      let vRet = hc.decrypt(cipherRes);
      let valExpected = MIN_INT_32 + MAX_INT_32;
      logOK("add     - SAME CONTEXT:  ", vRet, valExpected);
    }


    ////////////////////////////////////////////////////////////////////////////////
    //
    //TEST negate()
    //

    //Test 3.1: SHOULD PASS
    {
      let cipherRes = hc.negate(cipher2); //-(-7)
      let vRet = hc.decrypt(cipherRes);
      let valExpected = -val2;
      logOK("negate  - SAME CONTEXT:  ", vRet, valExpected);
    }

    //Test 3.2: SHOULD PASS
    {
      let cipherRes = hc.negate(hc.encrypt(MAX_INT_32));
      let vRet = hc.decrypt(cipherRes);
      let valExpected = -MAX_INT_32;
      logOK("negate  - SAME CONTEXT:  ", vRet, valExpected);
    }

    //Test 3.3: SHOULD FAIL @decrypt - because of IntegerEncoder::decode_int32()
    // Q: Do we plan to use FractionalEncoder instead of IntegerEncoder
    try {
      let cipherRes = hc.negate(hc.encrypt(MIN_INT_32));
      let vRet = hc.decrypt(cipherRes);               //!!! SEAL WILL THROW A C++ EXCEPTION: "cast failed" !!!
      throw NEVER_THROWN_EXCEPTION; //never
    } catch (err) {
      log("negate  - SAME CONTEXT:  \t\t Caught EXPECTED Exception: " + err);
    }


    ////////////////////////////////////////////////////////////////////////////////
    //
    //TEST sub()
    //

    //Test 4.1: SHOULD PASS
    {
      let cipherRes = hc.sub(cipher1, cipher2); //5 - (-7)
      let vRet = hc.decrypt(cipherRes);
      let valExpected = val1 - val2;
      logOK("sub     - SAME CONTEXT:  ", vRet, valExpected);
    }

    //Test 4.2: SHOULD FAIL @decrypt - because of IntegerEncoder::decode_int32()
    try {
      let cipherRes = hc.sub(hc.encrypt(MIN_INT_32), hc.encrypt(1)); // MIN_INT_32 - 1
      let vRet = hc.decrypt(cipherRes);               //!!! SEAL WILL THROW A C++ EXCEPTION: "cast failed" !!!
      throw NEVER_THROWN_EXCEPTION; //never
    } catch (err) {
      log("sub     - SAME CONTEXT:  \t\t Caught EXPECTED Exception: " + err);
    }


    ////////////////////////////////////////////////////////////////////////////////
    //
    //TEST multiply()
    //

    //Test 5.1: SHOULD PASS
    {
      let cipherRes = hc.multiply(cipher1, cipher2); //5 * (-7)
      let vRet = hc.decrypt(cipherRes);
      let valExpected = val1 * val2;
      logOK("multiply- SAME CONTEXT:  ", vRet, valExpected);
    }


    ////////////////////////////////////////////////////////////////////////////////
    //
    //TEST square()
    //

    //Test 6.1: SHOULD PASS
    {
      let cipherRes = hc.square(cipher2); //(-7)^2
      let vRet = hc.decrypt(cipherRes);
      let valExpected = Math.pow(val2, 2);
      logOK("square  - SAME CONTEXT:  ", vRet, valExpected);
    }


    ////////////////////////////////////////////////////////////////////////////////
    //
    //TEST getEncryptionParameters()
    //

    //SHOULD PASS
    {
      //let parms1 = hc.getEncryptionParameters();
      let hc1 = seal.generateHomomorficContext(); //HomomorphicContext(2048, 'coeff_modulus_128', 1<<8) for 'BFV' scheme
      let parms1 = hc1.getEncryptionParameters();
      let hc2 = new seal.HomomorphicContext(parms1);
      let parms2 = hc2.getEncryptionParameters();
      let sameEP = (parms1 === parms2);
      logOK("getEncryptionParameters - 2 CONTEXTS (default ctor): ", sameEP, true);
    }
    //SHOULD PASS
    {
      let hc1 = new seal.HomomorphicContext(8192, 'coeff_modulus_128', 40961); //'BFV' by default
      let parms1 = hc1.getEncryptionParameters();
      let hc2 = new seal.HomomorphicContext(parms1);
      let parms2 = hc2.getEncryptionParameters();
      let sameEP = (parms1 === parms2);
      logOK("getEncryptionParameters - 2 CONTEXTS    (init ctor): ", sameEP, true);
    }


    ////////////////////////////////////////////////////////////////////////////////
    //
    //TEST ALL
    //

    log("");
    //SHOULD PASS
    {
      let hc1 = new seal.HomomorphicContext(4096, 'coeff_modulus_192', Number(1 << 10));
      let parms1 = hc1.getEncryptionParameters();
      let pubKey1 = hc1.getPublicKey();
      let secKey1 = hc1.getSecretKey();
      let ciph1 = hc1.encrypt(val1);

      let hc2 = new seal.HomomorphicContext(parms1);
      let parms2 = hc2.getEncryptionParameters();
      hc2.setPublicKey(pubKey1);                    //if params1 <> params2: "PublicKey data is invalid" exception
      let ciph2 = hc2.encrypt(val2);

      let hc3 = new seal.HomomorphicContext(parms2); //same as `parms1`
      let c01 = hc3.negate(ciph1);
      let c02 = hc3.add(c01, ciph2);
      let c03 = hc3.multiply(c02, ciph2);
      let c04 = hc3.square(ciph1);
      let c05 = hc3.sub(c03, c04);

      let hc4 = new seal.HomomorphicContext(parms1);
      hc4.setSecretKey(secKey1);                    //if params1 <> params4: "SecretKey data is invalid" exception
      let vRet = hc4.decrypt(c05);

      let valExpected = ((-val1 + val2) * val2) - (val1 * val1); //59 = (-12 * -7) - 25
      logOK("ALL sealjs API - 4 CONTEXTS HAVING THE SAME ENCRIPTION PARAMETERS: ", vRet, valExpected);
    }
    log("");

  }
  catch (unexpectedError) {
    log("???  U N E X P E C T E D    E X C E P T I O N  ??? " + unexpectedError);
  }

} //end mySealTest



/********************************************************************************************************************************
 * Use of "seal.js" (JS glue code) and "seal.wasm" (WebAssembly) in NodeJS:
 */
const Module = require('./seal.js');

Module.onRuntimeInitialized = function () {

  let seal = this;
  mySealTest(seal);

} //end Module.onRuntimeInitialized


//The next explicit call is mandatory in case of standalone JavaScript (without .wasm file, when `WASM=0`):
// Module.onRuntimeInitialized();
