var seal = require('bindings')('nodeseal');

//string pool:
const STR_RESULT_OK = "\t SUCCESS - same results";
const STR_RESULT_WRONG = "\t ERROR - the 2 results differ!!";

const log = obj => console.log(String(obj));


/*******************************************************************************
 * Simplified API - 'light' version
 * 
 * You may use several contexts during a workflow of some SEAL operations; just
 * pay attention for:
 *   - the contexts you need to synchronize for one or more of:
 *        EncryptionParameters | PublicKey | PrivateKey
 *   - the owner of the PrivateKey (it is the MASTER context)
 * 
 * How to use:
 * (1) Create the HE:
 *           var hc = seal.generateHomomorficContext();
 *       or
 *           var hc = new seal.HomomorphicContext();
 * (2) Use various methods of a HomomorphicContext instance:
 *   - getPublicKey() - get the PublicKey (serialized as String)
 *                      var publicKey = hc.getPublicKey();
 *   - setPublicKey() - set the PublicKey (serialized as String)
 *                      hc.setPublicKey(publicKey);
 *   - getSecretKey() - get the SecretKey (serialized as String)
 *                      var secretKey = hc.getSecretKey();
 *   - setSecretKey() - set the SecretKey (serialized as String)
 *                      hc.setSecretKey(secretKey);
 * 
 *   - encrypt()      - encrypt an INT32 value to a Ciphertext (serialized as String)
 *                      var cipher = hc.encrypt(123);
 *   - decrypt()      - decrypt a Ciphertext (serialized as String) to an INT32 value
 *                      var n = hc.decrypt(cipher);
 * 
 *   - negate()       - HE negate a Ciphertext (serialized as String)
 *                      var cipher = hc.encrypt(123);
 * 
 *******************************************************************************/

// INT 32: [-2147483648; 2147483647]
const MIN_INT_32 = -2147483648;
const MAX_INT_32 =  2147483647;

let val1 = 5, val2 = -7;

log("val1 = " + val1 + ", val2 = " + val2);

try {
  
  let hc = seal.generateHomomorficContext();	//factory method

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
    log("decrypt - SAME CONTEXT:  " + v1 + " " + (typeof v1) + " " + ((val1 === v1) ? STR_RESULT_OK : STR_RESULT_WRONG));
  }

  //Test 1.2: SHOULD FAIL - hcOther doesn't know the original PrivateKey
  try {
    let hcOther = new seal.HomomorphicContext();
    let v1 = hcOther.decrypt(cipher1);  //!!! SEAL WILL THROW A C++ EXCEPTION: "output out of range" !!!
  } catch (err) {
    log("decrypt - OTHER CONTEXT: Caught EXPECTED Exception: " + err);
  }

  //Test 1.3: SHOULD FAIL - hcOther knows the original PublicKey only, not the original PrivateKey
  try {
    let hcOther = new seal.HomomorphicContext();
    hcOther.setPublicKey(publicKey);
    let v1 = hcOther.decrypt(cipher1);  //!!! SEAL WILL THROW A C++ EXCEPTION: "output out of range" !!!
  } catch (err) {
    log("decrypt - OTHER CONTEXT: Caught EXPECTED Exception: " + err);
  }

  //Test 1.4: SHOULD PASS - hcOther knows the original PrivateKey (PublicKey isn't necessary for decription)
  {
    let hcOther = new seal.HomomorphicContext();
    hcOther.setSecretKey(secretKey);    // mandatory for decryption
    let v1 = hcOther.decrypt(cipher1);  // NOW IT'S OK
    log("decrypt - OTHER CONTEXT: " + v1 + " " + (typeof v1) + " " + ((val1 === v1) ? STR_RESULT_OK : STR_RESULT_WRONG));
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
    log("add     - SAME CONTEXT:  " + vRet + " " + (typeof vRet) + " " + ((valExpected === vRet) ? STR_RESULT_OK : STR_RESULT_WRONG));
  }

  //Test 2.2: SHOULD PASS - SUM in other context, but with the same EncriptionParameters as for the original context!
  {
    let hcOther = new seal.HomomorphicContext();
    let cipherRes = hcOther.add(cipher1, cipher2);

    let vRet = hc.decrypt(cipherRes); //Decription in the original context!
    let valExpected = val1 + val2;
    log("add     - OTHER CONTEXT: " + vRet + " " + (typeof vRet) + " " + ((valExpected === vRet) ? STR_RESULT_OK : STR_RESULT_WRONG));
  }

  //Test 2.3: SHOULD FAIL @decrypt - val2 encripted in other context; the 2 contexts use 2 different PublicKey(s)
  try {
    let hcOther = new seal.HomomorphicContext();
    let cipher2 = hcOther.encrypt(val2);
    let cipherRes = hcOther.add(cipher1, cipher2);  //or hc.add(cipher1, cipher2)

    let vRet = hc.decrypt(cipherRes);               //!!! SEAL WILL THROW A C++ EXCEPTION: "output out of range" !!!
  } catch (err) {
    log("add     - OTHER CONTEXT: Caught EXPECTED Exception: " + err);
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
    log("add     - MANY CONTEXTS: " + vRet + " " + (typeof vRet) + " " + ((valExpected === vRet) ? STR_RESULT_OK : STR_RESULT_WRONG));
  }

  //Test 2.5: SHOULD PASS
  {
    let cipherRes = hc.add(hc.encrypt(MIN_INT_32), hc.encrypt(MAX_INT_32));
    let vRet = hc.decrypt(cipherRes);
    let valExpected = MIN_INT_32 + MAX_INT_32;
    log("add     - SAME CONTEXT:  " + vRet + " " + (typeof vRet) + " " + ((valExpected === vRet) ? STR_RESULT_OK : STR_RESULT_WRONG));
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
    log("negate  - SAME CONTEXT:  " + vRet + " " + (typeof vRet) + " " + ((valExpected === vRet) ? STR_RESULT_OK : STR_RESULT_WRONG));
  }

  //Test 3.2: SHOULD PASS
  {
    let cipherRes = hc.negate(hc.encrypt(MAX_INT_32));
    let vRet = hc.decrypt(cipherRes);
    let valExpected = -MAX_INT_32;
    log("negate  - SAME CONTEXT:  " + vRet + " " + (typeof vRet) + " " + ((valExpected === vRet) ? STR_RESULT_OK : STR_RESULT_WRONG));
  }

  //Test 3.3: SHOULD FAIL @decrypt - because of IntegerEncoder::decode_int32()
  // Q: Do we plan to use FractionalEncoder instead of IntegerEncoder
  try {
    let cipherRes = hc.negate(hc.encrypt(MIN_INT_32));
    let vRet = hc.decrypt(cipherRes);               //!!! SEAL WILL THROW A C++ EXCEPTION: "cast failed" !!!
  } catch (err) {
    log("negate  - SAME CONTEXT:  Caught EXPECTED Exception: " + err);
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
    log("sub     - SAME CONTEXT:  " + vRet + " " + (typeof vRet) + " " + ((valExpected === vRet) ? STR_RESULT_OK : STR_RESULT_WRONG));
  }

  //Test 4.2: SHOULD FAIL @decrypt - because of IntegerEncoder::decode_int32()
  try {
    let cipherRes = hc.sub(hc.encrypt(MIN_INT_32), hc.encrypt(1)); // MIN_INT_32 - 1
    let vRet = hc.decrypt(cipherRes);               //!!! SEAL WILL THROW A C++ EXCEPTION: "cast failed" !!!
  } catch (err) {
    log("sub     - SAME CONTEXT:  Caught EXPECTED Exception: " + err);
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
    log("multiply- SAME CONTEXT:  " + vRet + " " + (typeof vRet) + " " + ((valExpected === vRet) ? STR_RESULT_OK : STR_RESULT_WRONG));
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
    log("square  - SAME CONTEXT:  " + vRet + " " + (typeof vRet) + " " + ((valExpected === vRet) ? STR_RESULT_OK : STR_RESULT_WRONG));
  }

}
catch (unexpectedError) {
  log("???  U N E X P E C T E D    E X C E P T I O N  ??? " + unexpectedError);
}

//module.exports = addon; //?
