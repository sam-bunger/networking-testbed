Module['locateFile'] = function (path) {
    if (path === 'index.wasm') {
        return Main['wasmFile'] == undefined ? 'index.wasm' : Main['wasmFile']
    } 
    throw new Error('Cannot locate file ' + path)
}
