var addon = require('bindings')('addon');


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

// It will execute in the current thread; fct won't return until it is finished
function runSync(val1, val2) {
  var start = Date.now();

  var val1 = 5;
  var val2 = -7;
  var result = addon.calculateSync(val1, val2); //<==HERE!!!

  printResult('Sync', val1, val2, result, Date.now() - start);
}


runSync(); //<==HERE!!!
