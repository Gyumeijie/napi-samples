// Use the "bindings" package to locate the native bindings.
const binding = require('bindings')('binding');

// Call the function "startWork" which the native bindings library exposes.
// The function returns a promise which will be resolved at the complete of the
// work with a integer.
binding.startWork().then((result) => {
    console.log("[promise] received result: " + result)
});

async function startWork() {
    let result = await binding.startWork();
    console.log("[await] received result: " + result)
}
startWork();

setInterval(()=> console.log("js thread running..."), 1000);