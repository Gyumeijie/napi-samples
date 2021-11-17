// Use the "bindings" package to locate the native bindings.
const binding = require('bindings')('binding');

console.log(binding.hello()); // 'world'