// $npm run demo:

var addon = require('bindings')('nodeseal');

//helper
function printResult(type, val1, val2, result, ms) {
    console.log();
    console.log('====================================');
    console.log('==== LOG MSG FROM JS:');
    console.log(type, 'method:');
    console.log('(-' + val1 + ' + ' + val2 + ') * ' + val2);
    console.log('Result = ' + result);
    console.log('\tTook ' + ms + 'ms');
    console.log();
}

//demo; invoke addon.calculateSync()
//It will execute in the current thread; fct won't return until it is finished
function runSync() {
    var val1 = 5;
    var val2 = -7;
    var expectedResult = (-val1 + val2) * val2;

    var start = Date.now();

    var result = addon.calculateSync(val1, val2); //<==HERE!!!

    printResult('Sync', val1, val2, result, Date.now() - start);

    //should be an assertEqual() here...
    console.log('Expected result = ' + expectedResult);
    if (result !== expectedResult)
        throw "Error: the 2 results differ!";
}

////////////////////////////////////////////////////////////////////////////////
// sync
try {
    runSync(); //<==HERE!!!
}
catch (err) {
    console.log(err);
}

////////////////////////////////////////////////////////////////////////////////
// with Promise
{
    function promiseCalc(val1, val2) {
        return new Promise(function (resolve, reject) {
            var result = addon.calculateSync(val1, val2); //<==HERE!!!
            resolve(result);
        });
    }


    let u = 8, v = -12;
    let expectedResult = (-u + v) * v;
    var start = Date.now();

    promiseCalc(u, v)
        .then(function (res) {
            printResult('Promise', u, v, res, Date.now() - start);

            //should be an assertEqual() here...
            console.log('Expected result = ' + expectedResult);
            if (res !== expectedResult)
                throw "Error: the 2 results differ!";
        });
}