// Use the "bindings" package to locate the native bindings.
const binding = require('bindings')('binding');

console.log('This should be three:', binding.add(1, 2))

binding.add(1);